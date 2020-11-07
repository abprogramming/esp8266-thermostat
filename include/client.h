#ifndef __TEMPCLIENT_H__
#define __TEMPCLIENT_H__

#include "stdbool.h"

// Estabilish WiFi connection to Thermostat-AP
void client_init(void);

// Initiate TCP connection to thermostat server (10.1.1.1)
void client_connect(uint32_t temp);

// Returns true, if connection was succesful
bool tcp_ready(void);

// Send the current measured temperature as  TCP packet
bool tcp_send_packet(uint32_t temp);

#endif
