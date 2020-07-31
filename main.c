#include "FreeRTOS.h"
#include "task.h"
#include "esp/uart.h"

#include "temp.h"
#include "relay.h"
#include "display.h"
#include "buttons.h"

void user_init(void) {
    uart_set_baud(0, 115200);

    //Read temperature data from DS18B20's
    xTaskCreate(&read_temp, "read_temp", 256, NULL, 2, NULL);
    
    //Control the relay 
    xTaskCreate(&read_temp, "read_temp", 256, NULL, 2, NULL);

    //Handle the 7-segment displays
    xTaskCreate(&display_control, "print_temperature", 256, NULL, 2, NULL);

    //Handle buttons, switches and potentiometers
    xTaskCreate(&buttons_control, "print_temperature", 256, NULL, 2, NULL);

}

