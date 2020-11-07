#include "common.h"
#include <stdio.h>
#include <string.h>
#include <ip_addr.h>
#include <tcp.h>

#include "client.h"

static struct tcp_pcb* pcb = NULL;
static bool   is_tcp_ready = true; 

static float roundf(float v)
{
    float f = (int) (v * 10);
    return (float) f / 10;
}

static err_t cbConnect(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    uint32_t temp = (uint32_t) arg;
    float t = roundf(((float) temp) / 100);
    char *format = "GET /outtemp?val=%0.1f HTTP/1.1\r\nHost: 10.1.1.1\r\n\r\n";
    char payload[strlen(format)];
    snprintf(payload, strlen(format), format, t);

    err_t e = tcp_write(pcb, payload, strlen(payload), TCP_WRITE_FLAG_COPY);
    if (e)
    {
        is_tcp_ready = false;
        return 1;
    }

    e = tcp_output(pcb);
    if (e)
    {
        is_tcp_ready = false;
        return 1;
    }

    dprintf("TCP sent: %s\n", payload);

    tcp_close(pcb);
    return 0;
}

void client_init(void)
{
    // Initalize WiFi client (station mode)
    struct sdk_station_config config =
    {
        .ssid     = AP_SSID,
        .password = AP_PSK,
    };
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();
}
      
void client_connect(uint32_t temp)
{
    // Wait some time to be sure, that
    // the we are connected to the server AP
    DELAY(5000);
    
    ip_addr_t ip;
    IP4_ADDR(&ip, 10, 1, 1, 1);
    
    pcb = tcp_new();
    tcp_arg(pcb, (void*) temp);
    
    err_t e = tcp_bind(pcb, NULL, 128);
    if (e)
    {
        is_tcp_ready = false;
        return;
    }
    
    e = tcp_connect(pcb, &ip, 80, cbConnect);
    if (e)
    {
        is_tcp_ready = false;
        return;
    }
}

bool tcp_ready(void)
{
    return is_tcp_ready;
}
