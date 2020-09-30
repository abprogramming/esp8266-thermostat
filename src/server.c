#include <string.h>
#include <stdio.h>
#include <httpd/httpd.h>
#include <dhcpserver.h>
#include "common.h"
#include "server.h"

#define AP_SSID "Thermostat-AP"
#define AP_PSK "esp-open-rtos"

enum {
    TEMP_TGT,
    TEMP_ROOM,
    TEMP_OUTS,
    RELAY_STATE,
    RELAY_FORCEON
};

static TaskHandle_t main_task_h = NULL;

static float TEMP_TGT_V  = 21.5;
static float TEMP_ROOM_V = 21.5;
static float TEMP_OUTS_V = 21.5;
static uint8_t RSTATE = 0;
static uint8_t RFORCEON = 0;

int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
{
    switch (iIndex)
    {
        case TEMP_TGT:
            snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_TGT_V);
            break;
        case TEMP_ROOM:
            snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_ROOM_V);
            break;
        case TEMP_OUTS:
            snprintf(pcInsert, iInsertLen, "%0.1f", TEMP_OUTS_V);
            break;
        case RELAY_STATE:
            snprintf(pcInsert, iInsertLen, "%s", RSTATE ? "On" : "Off");
            break;
        case RELAY_FORCEON:
            snprintf(pcInsert, iInsertLen, "%u", RFORCEON);
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
    return "/index.ssi";
}

void httpd_task(void *pvParameters)
{
    tCGI pCGIs[] =
    {
        {"/settemp",  (tCGIHandler) settemp_cgi_handler},
        {"/setrelay", (tCGIHandler) setrelay_cgi_handler},
    };

    // Each element in the array corresponds to
    // a member of the unnamed enum above
    const char *pcConfigSSITags[] =
    {
        "tgttemp",
        "roomtemp", 
        "outstemp",
        "rstate",
        "forceon"
    };

    // register handlers and start the server
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0]));
    http_set_ssi_handler((tSSIHandler) ssi_handler, pcConfigSSITags,
        sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    printf("sizeof %zd\n", sizeof (pcConfigSSITags) / sizeof (pcConfigSSITags[0]));
    httpd_init();

    for (;;);
}

TaskHandle_t server_init(TaskHandle_t _main_task_h)
{
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
    xTaskCreate(&httpd_task, "HTTP Daemon", 1024, NULL, 2, &task_h);
    return task_h;
}
