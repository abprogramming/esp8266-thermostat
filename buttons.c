#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include "timers.h"

#include "common.h"
#include "buttons.h"
#include "display.h"

/**
 * This module controls user inputs,
 * namely a tactile button and a potentiometer.
 * 
 * When the button is pressed, the display turns
 * on and shows the temperature.
 * If the uses takes no further action, the display
 * is turned off in a few seconds.
 * 
 * The button also activates the temperature knob task,
 * a 10k pontentiometer, which is being readed
 * with ADC. If the user turns the knob, the device
 * enters set temperature mode and the display stays on
 * until the new temperature is set, so there's no new
 * value coming from the potentiometer.
 */


static TimerHandle_t display_timer = NULL;
static TaskHandle_t set_temp_task_h = NULL;
static uint16_t last_adc_value = UINT16_MAX;

// Epsilon value to suppress noise, below this value
// we consider the value is unchanged
static const adc_epsilon = 1;

void set_temperature_task(void *pvParameters)
{
	TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;
	
	// After starting the program and the task is created
	// we read and store the actual value of the potentiometer
	// to initialize last_adc_value, to know if the user
	// wants to enter to set mode
	
	

}

void display_timer_cb(TimerHandle_t pxTimer)
{
	taskENTER_CRITICAL();
	set_display_state(DISPLAY_OFF);
	// Suspend temperature knob control
	vTaskSuspend(set_temp_task_h);
    taskEXIT_CRITICAL();
}

void button_interrupt(uint8_t gpio_num)
{
	(void)gpio_num;
	
    taskENTER_CRITICAL();
	set_display_state(DISPLAY_ON);
    taskEXIT_CRITICAL();
    
    xTimerStart(display_timer, 10);
}

void input_control_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;

    // Attach an interrupt handler the GPIO pin of the tactile button
    gpio_set_interrupt(PIN_TACTILEBUTTON,
		    GPIO_INTTYPE_LEVEL_HIGH, &button_interrupt);
		    
	// Timer for the display, the callback function turns the
	// displaty to OFF state, if no user iteraction occurs in a time
	display_timer = xTimerCreate("display_timer", DISPLAY_DELAY / portTICK_PERIOD_MS,
        pdFALSE, 0, &display_timer_cb);

	// Task for handling the temperature knob
    xTaskCreate(&set_temperature_task, "set_temperature_task",
	    256, (void*) main_task_h, 2, &set_temp_task_h);

    float temps[2];
    uint32_t notify_val;

    for (;;)
    {
	xTaskNotify(main_task_h, notify_val,
		(eNotifyAction)eSetValueWithOverwrite);
    }
}
  
