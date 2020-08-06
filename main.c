#include "FreeRTOS.h"
#include "task.h"
#include "esp/uart.h"

#include "temp.h"
//#include "relay.h"
#include "display.h"
//#include "buttons.h"

static TaskHandle_t main_task_h    = NULL;
static TaskHandle_t temp_task_h    = NULL;
static TaskHandle_t relay_task_h   = NULL;
static TaskHandle_t display_task_h = NULL;
static TaskHandle_t input_task_h   = NULL;

void user_init(void) {

    uart_set_baud(0, 9600);

    //Main flow control
    xTaskCreate(&main_task,           "main_task",        256, NULL, 2, &main_task_h);

    //Read temperature data from DS18B20's
    xTaskCreate(&read_temp_task,      "read_temp",        256, NULL, 2, &temp_task_h);
    
    //Control the relay 
    xTaskCreate(&relay_control_task,  "relay_control",    256, NULL, 2, &relay_task_h);

    //Handle the 7-segment displays
    xTaskCreate(&display_control_task, "display_control", 256, NULL, 2, &display_task_h);

    //Handle user input, button, switch and potentiometer
    xTaskCreate(&input_control_task,   "input_control",   256, NULL, 2, &input_task_h);

}

