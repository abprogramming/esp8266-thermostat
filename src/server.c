#include "common.h"
#include <string.h>
#include <stdio.h>
#include <httpd/httpd.h>
#include <dhcpserver.h>
#include <semphr.h>

#include "templog.h"
#include "server.h"


#define AP_SSID "Thermostat-AP"
#define AP_PSK  "trustno1"

static TaskHandle_t main_task_h = NULL;

static float TEMP_TGT_V  = TEMP_INITIAL;
static float TEMP_ROOM_V = 655;
static float TEMP_OUTS_V = 655;
static uint8_t RSTATE = 0;
static uint8_t RFORCEON = 0;

struct log_buffer_t templog;

/////////////////////////////////////////////////////
// Minimal implementation of a eeal-time clock

static uint32_t TIMESTAMP;
SemaphoreHandle_t sem_ts;

static uint32_t get_time()
{
    uint32_t out = 0;
    if (sem_ts != NULL)
    {
        if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
        {
            out = TIMESTAMP;
            xSemaphoreGive(sem_ts);
        }
    }
    return out;
}

static void set_time(uint32_t ts)
{
    if (sem_ts != NULL)
    {
        if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
        {
            TIMESTAMP = ts;
            xSemaphoreGive(sem_ts);
        }
    }
}

static void clock_task(void *pvParameters)
{
    sem_ts = xSemaphoreCreateMutex();
    for (;;)
    {
        if (sem_ts != NULL)
        {
            if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
            {
                TIMESTAMP++;
                printf("TS %u\n", TIMESTAMP);
                xSemaphoreGive(sem_ts);
            }
        }
        DELAY(1000);
    }
}


/////////////////////////////////////////////////////
// Manage log

static void log_settemp()
{
    char v[12];
    memset((void*) v, 0, 12);
    snprintf(v, 12, "SET %0.1f", TEMP_TGT_V);
    struct log_entry_t e;
    e.ts = get_time();
    strncpy(e.v, v, 12);
    log_buffer_push(&templog, e);
}

static void log_relay()
{
    char v[12];
    memset((void*) v, 0, 12);
    snprintf(v, 12, "RELAY %s", RSTATE ? "On" : "Off");
    struct log_entry_t e;
    e.ts = get_time();
    strncpy(e.v, v, 12);
    log_buffer_push(&templog, e);
}

/////////////////////////////////////////////////////
// CGI/SSI Handlers

enum {
    TEMP_TGT,
    TEMP_ROOM,
    TEMP_OUTS,
    RELAY_STATE,
    RELAY_FORCEON,
    LOG_TSTAMP
};

static void notify_settemp()
{
    uint32_t notify_val = 0;
    notify_val   = FLT2UINT32(TEMP_TGT_V);
    notify_val <<= 16;
    notify_val  += MAGIC_ACC_TEMP;
    xTaskNotify(main_task_h, notify_val,
            (eNotifyAction) eSetValueWithOverwrite);   
}

static void notify_forceon()
{
    uint32_t notify_val =
        (RFORCEON ? MAGIC_FORCERELAY_ON : MAGIC_FORCERELAY_OFF);
    xTaskNotify(main_task_h, notify_val,
            (eNotifyAction) eSetValueWithOverwrite);   
}

int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
{
    // This is for the log entries
    if (iIndex > LOG_TSTAMP)
    {
        struct log_entry_t e = log_buffer_getnext(&templog);
        snprintf(pcInsert, iInsertLen, "%u | %s", e.ts, e.v);
        return (strlen(pcInsert));
    }
    
    switch (iIndex)
    {
        case TEMP_TGT:
            snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_TGT_V);
            break;
        case TEMP_ROOM:
            if (TEMP_ROOM_V >= 100)
            {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_ROOM_V);
            }
            break;
        case TEMP_OUTS:
            if (TEMP_OUTS_V >= 100)
            {
                snprintf(pcInsert, iInsertLen, "N/A");
            }
            else
            {
                snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_OUTS_V);
            }
            break;
        case RELAY_STATE:
            snprintf(pcInsert, iInsertLen, "%s", RSTATE ? "On" : "Off");
            break;
        case RELAY_FORCEON:
            snprintf(pcInsert, iInsertLen, "%u", RFORCEON);
            break;
        case LOG_TSTAMP:
            snprintf(pcInsert, iInsertLen, "%u", get_time());
            break;
        default:
            snprintf(pcInsert, iInsertLen, "N/A");
            break;
    }

    // Tell the server how many characters to insert
    return (strlen(pcInsert));
}

const char *settemp_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "val") == 0)
        {
            TEMP_TGT_V = atof(pcValue[i]);
            notify_settemp();
            log_settemp();
        }
    }
    return "/index.ssi";
}

const char *setrelay_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "on") == 0)
        {
            RFORCEON = 1;
        }
        else if (strcmp(pcParam[i], "off") == 0)
        {
            RFORCEON = 0;
        }
    }
    notify_forceon();
    return "/index.ssi";
}

const char *logpage_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    log_buffer_reset(&templog);
    return "/log.ssi";
}

const char *settime_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "val") == 0)
        {
            uint32_t v;
            sscanf(pcValue[i], "%u", &v);
            set_time(v);
        }
    }
    return "/index.ssi";
}

void httpd_task(void *pvParameters)
{
    tCGI pCGIs[] =
    {
        { "/settemp",  (tCGIHandler) settemp_cgi_handler  },
        { "/setrelay", (tCGIHandler) setrelay_cgi_handler },
        { "/log",      (tCGIHandler) logpage_cgi_handler  },
        { "/settime",  (tCGIHandler) settime_cgi_handler  },
    };

    // Each element in the array corresponds to
    // a member of the unnamed enum above
    const char *pcConfigSSITags[] =
    {
        "tgttemp",
        "roomtemp", 
        "outstemp",
        "rstate",
        "forceon",
        "tstamp",
        "log1",
        "log2",
        "log3",
        "log4",
        "log5",
        "log6",
        "log7",
        "log8",
        "log9",
        "log10",
    };

    // register handlers and start the server
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
        sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    httpd_init();

    uint32_t recv_temp;
    for (;;)
    {
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
            
        if (recv_temp == MAGIC_RELAY_OFF)
        {
            RSTATE = 0;
            log_relay();
        }
        else if (recv_temp == MAGIC_RELAY_ON)
        {
            RSTATE = 1;
            log_relay();
        }
        else 
        {
            TEMP_ROOM_V = ((float) GETUPPER16(recv_temp)) / 100;
            TEMP_OUTS_V = ((float) GETLOWER16(recv_temp)) / 100;
        }
    }
}


/////////////////////////////////////////////////////
// Server initialization

TaskHandle_t server_init(TaskHandle_t _main_task_h)
{
    set_time(0);
    templog = log_buffer_init(8 * 60);
    
    main_task_h = _main_task_h;
    sdk_wifi_set_opmode(SOFTAP_MODE);
    
    struct ip_info ap_ip;
    IP4_ADDR(&ap_ip.ip, 10, 1, 1, 1);
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    IP4_ADDR(&ap_ip.netmask, 255, 0, 0, 0);
    sdk_wifi_set_ip_info(1, &ap_ip);
        
    struct sdk_softap_config ap_config =
    {
        .ssid            = AP_SSID,
        .ssid_hidden     = 0,
        .channel         = 3,
        .ssid_len        = strlen(AP_SSID),
        .authmode        =  AUTH_WPA_WPA2_PSK,
        .password        = AP_PSK,
        .max_connection  = 3,
        .beacon_interval = 100,
    };
    sdk_wifi_softap_set_config(&ap_config);
    
    ip_addr_t first_client_ip;
    IP4_ADDR(&first_client_ip, 10, 1, 1, 2);
    dhcpserver_start(&first_client_ip, 4);
    
    TaskHandle_t task_h = NULL;
    xTaskCreate(&httpd_task, "HTTP Daemon", 1024, NULL, PRIO_DEFAULT, &task_h);
    xTaskCreate(&clock_task, "Clock", 1024, NULL, PRIO_HIGH, NULL);
    return task_h;
}
