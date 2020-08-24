#include "common.h"
#include "timers.h"
#include "display.h"
#include "input.h"


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
static TaskHandle_t  main_task_h = NULL;
static TaskHandle_t  set_temp_task_h = NULL;
static uint16_t last_adc_value = UINT16_MAX;
static bool set_mode = false;


/////////////////////////////////////////////////////
// Temperature knob control and ADC voltage
// conversion to temperature values

static float celsius_per_adc_step = 
    (TEMP_MAX - TEMP_MIN) / (ADC_MAX - ADC_MIN);
    
// Epsilon value to suppress noise, below this value
// we consider the value is unchanged
static const uint16_t adc_epsilon = 10;

uint32_t adc_temp_to_uint32(uint16_t adc)
{
    float t = (ADC_MAX - adc) * celsius_per_adc_step;
    t += TEMP_MIN;
    return FLT2UINT32(t);
}

void set_temperature_task(void *pvParameters)
{    
    uint16_t adc = 0;
    uint32_t notify_val = 0;
    
    for (;;)
    {
        adc = sdk_system_adc_read();
        dprintf("adc %u\n", adc);
        
        // After starting this task is created we read and store
        // the actual value of the potentiometer to initialize
        // last_adc_value and then suspend this task.
        // It will be woken up button_interrupt. 
        if (last_adc_value == UINT16_MAX)
        {
            last_adc_value = adc;
            vTaskSuspend(NULL);
        }
        
        if (abs(adc - last_adc_value) > adc_epsilon)
        {
            set_mode = true;
            
            // Keep display alive
            xTimerReset(display_timer, 0);
            
            notify_val   = adc_temp_to_uint32(adc);
            notify_val <<= 16;
            notify_val  += MAGIC_SET_TEMP;
            
            // Send temperature value to 
            // main task for display
            xTaskNotify(main_task_h, notify_val,
                (eNotifyAction) eSetValueWithOverwrite);   
                
            last_adc_value = adc;
        }
        DELAY(100);
    }
}


/////////////////////////////////////////////////////
// Temperature knob control and ADC voltage
// conversion to temperature values

void button_interrupt(uint8_t gpio_num)
{
    (void)gpio_num;
 
    // Disable this ISR until the display
    // is turned off
    gpio_set_interrupt(PIN_TACTILEBUTTON,
        GPIO_INTTYPE_NONE, &button_interrupt);
        
    // Turn on display
    set_display_state(DISPLAY_ON);
    
    // Start temperature knob control
    xTaskResumeFromISR(set_temp_task_h);
    
    // Start countdown to turn off display
    // if there's no user interaction
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(xTimerStartFromISR(display_timer,
       &xHigherPriorityTaskWoken) != pdPASS)
    {
	// In case of failure, don't let the display on
	set_display_state(DISPLAY_OFF);
    }
 }
 
void display_timer_cb(TimerHandle_t pxTimer)
{   
    // Suspend temperature knob control
    vTaskSuspend(set_temp_task_h);
    
    // If a new temperature was set,
    // send it for the main task
    if (set_mode)
    {
        uint32_t notify_val = 0;
        notify_val   = adc_temp_to_uint32(last_adc_value);
        notify_val <<= 16;
        notify_val  += MAGIC_ACC_TEMP;
        set_mode = false;
        xTaskNotify(main_task_h, notify_val,
                (eNotifyAction) eSetValueWithOverwrite);   
     }       
    
    // Turn off display
    set_display_state(DISPLAY_OFF);
       
    // Re-enable interrupt to handle the
    // button press event
    gpio_set_interrupt(PIN_TACTILEBUTTON,
        GPIO_INTTYPE_EDGE_POS, &button_interrupt);
}

void input_control_task(void *pvParameters)
{
    main_task_h = (TaskHandle_t) pvParameters;
    
    gpio_enable(PIN_TACTILEBUTTON, GPIO_INPUT);
    
    // Attach an interrupt handler the GPIO pin of the tactile button
    gpio_set_interrupt(PIN_TACTILEBUTTON,
        GPIO_INTTYPE_EDGE_POS, &button_interrupt);
            
    // Timer for the display, the callback function turns the
    // displaty to OFF state, if no user iteraction occurs in a time
    display_timer = xTimerCreate("display_timer",
        DISPLAY_DELAY / portTICK_PERIOD_MS,
        pdFALSE, 0, &display_timer_cb);

    // Task for handling the temperature knob
    xTaskCreate(&set_temperature_task, "set_temperature_task",
        256, main_task_h, PRIO_DEFAULT, &set_temp_task_h);
        
    for (;;)
    {
    }
}
  
