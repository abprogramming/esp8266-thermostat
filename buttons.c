#include "common.h"
#include "timers.h"
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
static uint8_t set_mode = 0;

// Epsilon value to suppress noise, below this value
// we consider the value is unchanged
static const uint16_t adc_epsilon = 1;

uint32_t adc_temp_to_uint32(uint16_t adc)
{
    uint32_t out = 0;
    float temp;
    //t16 to float 
    out = FLT2UINT32(temp);
    out <<= 16;
    return out;
}

uint32_t get_notification_from_adc(uint16_t adc, uint16_t magic)
{
}

void set_temperature_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;
    
    // After starting the program and the task is created
    // we read and store the actual value of the potentiometer
    // to initialize last_adc_value and then suspend this task.
    // It will be woken up button_interrupt. 
    
    uint16_t adc = 0;
    uint32_t notify_val = 0;
    
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
            set_mode = 1;
            // Keep display alive
            xTimerReset(display_timer, 0);
            notify_val   = adc_temp_to_uint32(adc);
            // Append magic to notification value
            notify_val <<= 16;
            notify_val  += MAGIC_SET_TEMP;
            xTaskNotify(main_task_h, notify_val,
                (eNotifyAction) eSetValueWithOverwrite);   
        }
        
        DELAY(100);
    }

}

void display_timer_cb(TimerHandle_t pxTimer)
{
    taskENTER_CRITICAL();
    printf("timer end\n");
    fflush(stdout);
    // Suspend temperature knob control
    vTaskSuspend(set_temp_task_h);
    set_display_state(DISPLAY_OFF);
    taskEXIT_CRITICAL();
}

void button_interrupt(uint8_t gpio_num)
{
    (void)gpio_num;
    
    printf("GPIO ir\n");
    fflush(stdout);
    
    taskENTER_CRITICAL();
    if (set_mode == 0)
    {
        set_display_state(DISPLAY_ON);
        // Start temperature knob control
        vTaskResume(set_temp_task_h);
    }
    else
    {
        // Send the final knob value 
        // to the main for display and 
        // setting the new temperature value
        set_mode = 0;
        
    }
    taskEXIT_CRITICAL();
    
    xTimerStart(display_timer, 10);
}

void input_control_init(void *main_task_h)
{
    gpio_enable(PIN_TACTILEBUTTON, GPIO_INPUT);
    
    // Attach an interrupt handler the GPIO pin of the tactile button
    gpio_set_interrupt(PIN_TACTILEBUTTON,
        GPIO_INTTYPE_EDGE_ANY, &button_interrupt);
            
    // Timer for the display, the callback function turns the
    // displaty to OFF state, if no user iteraction occurs in a time
    display_timer = xTimerCreate("display_timer",
        DISPLAY_DELAY / portTICK_PERIOD_MS,
        pdFALSE, 0, &display_timer_cb);

    // Task for handling the temperature knob
    xTaskCreate(&set_temperature_task, "set_temperature_task",
        256, main_task_h, PRIO_DEFAULT, &set_temp_task_h);
}
  
