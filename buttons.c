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

uint32_t adc_temp_to_uint32(uint16_t adc)
{
	uint32_t out = 0;
	
	return out;
}

void set_temperature_task(void *pvParameters)
{
	TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;
	
	// After starting the program and the task is created
	// we read and store the actual value of the potentiometer
	// to initialize last_adc_value and then suspend this task.
	// It will be woken up button_interrupt. 
	
	uint16_t adc = 0;
	
	for (;;)
	{
		adc = sdk_system_adc_read();
		
		if (adc == UINT16_MAX)
		{
			last_adc_value = adc;
			vTaskSuspend(set_temp_task_h);
		}
		
		if (abs(adc - last_adc_value) > adc_epsilon)
		{
			
		}
		
		DELAY(100);
	}

}

void display_timer_cb(TimerHandle_t pxTimer)
{
	taskENTER_CRITICAL();
	printf("timer end\n");
	fflush(stdout);
	set_display_state(DISPLAY_OFF);
	// Suspend temperature knob control
	//vTaskSuspend(set_temp_task_h);
    taskEXIT_CRITICAL();
}

void button_interrupt(uint8_t gpio_num)
{
	(void)gpio_num;
	printf("GPIO ir\n");
	fflush(stdout);
    taskENTER_CRITICAL();
	set_display_state(DISPLAY_ON);
    taskEXIT_CRITICAL();
    
    xTimerStart(display_timer, 10);
}

void input_control_init(void *main_task_h)
{
    // Attach an interrupt handler the GPIO pin of the tactile button
    gpio_set_interrupt(PIN_TACTILEBUTTON,
		    GPIO_INTTYPE_EDGE_ANY, &button_interrupt);
		    printf("it ok\n");
		    
	// Timer for the display, the callback function turns the
	// displaty to OFF state, if no user iteraction occurs in a time
	//display_timer = xTimerCreate("display_timer", DISPLAY_DELAY / portTICK_PERIOD_MS,
    //    pdFALSE, 0, &display_timer_cb);

	// Task for handling the temperature knob
    //xTaskCreate(&set_temperature_task, "set_temperature_task",
	//    256, main_task_h, 2, &set_temp_task_h);

}
  
