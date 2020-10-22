#include "common.h"
#include <string.h>
#include <stdio.h>
#include <httpd/httpd.h>
#include <dhcpserver.h>

#include "templog.h"
#include "clock.h"
#include "server.h"
#include "cgi-ssi.h"


static TaskHandle_t main_task_h = NULL;
struct log_buffer_t templog;

static float TEMP_TGT_V  = TEMP_INITIAL;

static float TEMP_ROOM_V = 655;
static float TEMP_OUTS_V = 655;

static float LAST_ROOM_TEMP = 0;
static float LAST_OUTS_TEMP = 0;

// This indicates the actual value upon
// which the decision to change the relay
// state was made by the main task
static float LAST_RELAY_TEMP = 0;

static uint8_t RSTATE = 0;
static uint8_t RFORCEON = 0;


/////////////////////////////////////////////////////
// Manage temperature log
// (With additional information, like temperature
//  set events and relay state changes)

static uint32_t last_temp_time = 0;
static const uint32_t temp_log_frequency = 10 * 60;

enum logtype {
    LOG_TEMP,
    LOG_RELAY,
    LOG_SETTEMP
};

static void write_log(enum logtype t)
{
    char v[28];
    memset((void*) v, 0, 28);

    // Not every temp. reading should be stored...
    uint32_t ut = 0;
    if (t == LOG_TEMP)
    {
        ut = get_uptime();
        if (last_temp_time != 0 &&
            last_temp_time + temp_log_frequency > ut)
        {
            return;
        }
        if (TEMP_ROOM_V == LAST_ROOM_TEMP &&
            TEMP_OUTS_V == LAST_OUTS_TEMP)
        {
            return;
        }
    }

    switch (t)
    {
        case LOG_TEMP:
            snprintf(v, 28, "R:%0.1f O:%0.1f", TEMP_ROOM_V, TEMP_OUTS_V);
            last_temp_time = ut;
            LAST_ROOM_TEMP = TEMP_ROOM_V;
            LAST_OUTS_TEMP = TEMP_OUTS_V;
            break;

        case LOG_RELAY:
            snprintf(v, 28, "RELAY %s (%0.1f)", RSTATE ? "On" : "Off", LAST_RELAY_TEMP);
            break;

        case LOG_SETTEMP:
            snprintf(v, 28, "SET %0.1f", TEMP_TGT_V);
            break;

        default:
            return;
    }

    struct log_entry_t e;
    e.ts = get_time();
    strncpy(e.v, v, 28);
    log_buffer_push(&templog, e);
}


/////////////////////////////////////////////////////
// Functions for sending FreeRTOS notifications
// to the main task about user decisions

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


/////////////////////////////////////////////////////
// Rules for filling the dynamic parts (server side-includes)
// of the hosted web pages.

int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
{
     char buf[72];

    // This is for the log entries
    if (iIndex >= LOG_MIN)
    {
        struct log_entry_t e = log_buffer_getnext(&templog);
        ts_to_str(e.ts, buf);
        snprintf(pcInsert, iInsertLen, "%s %s", buf, e.v);
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
            ts_to_str(get_time(), buf);
            snprintf(pcInsert, iInsertLen, "%s", buf);
            break;

        case SERVER_UPTIME:
            snprintf(pcInsert, iInsertLen, "%u", get_uptime());
            break;

        default:
            snprintf(pcInsert, iInsertLen, "N/A");
            break;
    }

    // Tell the server how many characters to insert
    return (strlen(pcInsert));
}


/////////////////////////////////////////////////////
// Handler functions for the GET HTTP requests

// Set new target temperature
const char *settemp_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "val") == 0)
        {
            TEMP_TGT_V = atof(pcValue[i]);
            notify_settemp();
            write_log(LOG_SETTEMP);
        }
    }
    return "/index.ssi";
}

// Toggle force relay on function
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

// Request for the log page
const char *logpage_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    log_buffer_reset(&templog);
    return "/log.ssi";
}

// Set system time (software real time clock)
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

// Get outside temperature from another
// device running the client firmware
const char *outtemp_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++)
    {
        if (strcmp(pcParam[i], "val") == 0)
        {
            TEMP_OUTS_V = atof(pcValue[i]);
        }
    }
    return "/index.ssi";
}


static float roundf(float v)
{
    float f = (int) (v * 10);
    return (float) f / 10;
}


// Register the main HTTP Daemon task and CGI request addresses
static void httpd_task(void *pvParameters)
{
    tCGI pCGIs[] =
    {
        { "/settemp",  (tCGIHandler) settemp_cgi_handler  },
        { "/setrelay", (tCGIHandler) setrelay_cgi_handler },
        { "/log",      (tCGIHandler) logpage_cgi_handler  },
        { "/settime",  (tCGIHandler) settime_cgi_handler  },
        { "/outtemp",  (tCGIHandler) outtemp_cgi_handler  },
    };

    // register handlers and start the server
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
        sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    httpd_init();

    uint32_t recv_temp;

    // Main loop: wait for notification from the main task
    // and fill the local copies of the received values
    // to be available for SSI handlers
    for (;;)
    {
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);

        if (GETLOWER16(recv_temp) == MAGIC_RELAY_OFF)
        {
            RSTATE = 0;
            LAST_RELAY_TEMP = ((float) GETUPPER16(recv_temp)) / 100;
            write_log(LOG_RELAY);
        }
        else if (GETLOWER16(recv_temp) == MAGIC_RELAY_ON)
        {
            RSTATE = 1;
            LAST_RELAY_TEMP = ((float) GETUPPER16(recv_temp)) / 100;
            write_log(LOG_RELAY);
        }
        else
        {
            TEMP_ROOM_V = roundf(((float) GETLOWER16(recv_temp)) / 100);
            // Deprecated
            //TEMP_OUTS_V = roundf(((float) GETLOWER16(recv_temp)) / 100);
            write_log(LOG_TEMP);
        }
    }
}


/////////////////////////////////////////////////////
// Server initialization:
// set up WiFi SoftAP and DHCP server,
// then start the HTTP Daemon task

TaskHandle_t server_init(TaskHandle_t _main_task_h)
{
    start_clock();
    templog = log_buffer_init(100);

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

    return task_h;
}
