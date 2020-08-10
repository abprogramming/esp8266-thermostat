#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "esp/uart.h"
#include "esp8266.h"

#include "common.h"
#include "buttons.h"
#include "display.h"

static TimerHandle_t display_timer = NULL;

void display_timer_callback(TimerHandle_t pxTimer)
{
}

void set_temperature_task(void *pvParameters)
{
     display_timer = TimerCreate("display_timer",
	DISPLAY_DELAY/portTICK_PERIOD_MS, pdFALSE, 0, &display_timer_callback);
}

void button_interrupt(uint8_t gpio_num)
{
    taskENTER_CRITICAL();
    display_on();
    DELAY(10000);
    display_off();
    taskEXIT_CRITICAL();
}

void input_control_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t)pvParameters;

    // Attach an interrupt handler the GPIO pin of the tactile button
    gpio_set_interrupt(PIN_TACTILEBUTTON,
		    GPIO_INTTYPE_LEVEL_HIGH, &button_interrupt);

    xTaskCreate(&set_temperature_task, "set_temperature_task",
	    256, (void*)main_task_h, 2, NULL);

		

    float temps[2];
    uint32_t notify_val;

    for (;;)
    {
	xTaskNotify(main_task_h, notify_val,
		(eNotifyAction)eSetValueWithOverwrite);
    }
}
  
