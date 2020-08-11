#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "common.h"
#include "main.h"
#include "temp.h"
//#include "relay.h"
#include "display.h"
#include "buttons.h"


/** 
 * This defines the entry point for the application
 * and the main task (loop). There's one separate
 * task for each input/output device or sensor.
 * Tasks are communicating thru RTOS task notifications,
 * but only with the main task.
 * 
 * The purpose of the main task is to collect and store
 * the actual temperature data and the desired room
 * temperature value, send it for the displays and
 * the relay control (which is responsible to decide
 * when to turn on or off the heater).
 */

void user_init(void)
{
    uart_set_baud(0, BAUDRATE);

   	// Main flow control
    xTaskCreate(&main_task, "main_task",
        256, NULL, 2, &main_task_h);

   	// Each task receives the handle for the main task

   	// Read temperature data from sensors
    xTaskCreate(&read_temp_task, "read_temp",
        256, (void*) main_task_h, 2, &temp_task_h);

   	// Control the relay 
   	//xTaskCreate(&relay_control_task,  "relay_control",    256, NULL, 2, &relay_task_h);

   	// Handle the 7-segment displays
    xTaskCreate(&display_control_task, "display_control",
        256, (void*) main_task_h, 2, &display_task_h);

   	// Handle user input, button, switch and potentiometer
   	xTaskCreate(&input_control_task, "input_control",
   	    256, (void*) main_task_h, 2, &input_task_h);
}

void main_task(void *pvParameters)
{
    uint32_t recv_temp;
    uint32_t set_temp;

   	// Wait some time to be
   	// sure everything is ready
    DELAY(3000);

    for (;;)
    {
       	// Get temperature readings
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
        printf("recv val=%u room=%u outside=%u\n", recv_temp, GETUPPER16(recv_temp), GETLOWER16(recv_temp));

       	// Refresh display
        xTaskNotify(display_task_h, recv_temp,
            (eNotifyAction) eSetValueWithOverwrite);
    }
}
