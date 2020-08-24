#include "common.h"
#include "main.h"
#include "temp.h"
#include "relay.h"
#include "display.h"
#include "input.h"


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
        256, NULL, PRIO_DEFAULT, &main_task_h);


    // Each task receives the handle for the main task

    // Read temperature data from sensors
    xTaskCreate(&read_temp_task, "read_temp",
        256, (void*) main_task_h, PRIO_DEFAULT, &temp_task_h);

    // Handle the 7-segment displays
    xTaskCreate(&display_control_task, "display_control",
        256, (void*) main_task_h, PRIO_DEFAULT, &display_task_h);
    // Handle user input, button, switch and potentiometer
    xTaskCreate(&input_control_task, "display_control",
        256, (void*) main_task_h, PRIO_DEFAULT, &input_task_h);
    
    // Init relay module
    relay_init();
    set_relay_state(RELAY_OFF);
}

/////////////////////////////////////////////////////

void update_relay_state
(
    relay_state_t *st,
    uint16_t actual,
    uint16_t target,
    uint16_t hyst
)
{
    dprintf("rs act %u tgt %u h %u\n", actual, target, hyst);
    if (*st == RELAY_OFF && 
        actual <= target - hyst)
    {
        *st = RELAY_ON;
    }
    
    if (*st == RELAY_ON && 
        actual >= target + hyst)
    {
        *st = RELAY_OFF;
    }
}

/////////////////////////////////////////////////////

void main_task(void *pvParameters)
{
    uint32_t recv_temp;
    uint16_t set_temp = (uint16_t) FLT2UINT32(TEMP_INITIAL);
    uint16_t hyst = (uint16_t) FLT2UINT32(HYST_INITIAL);
    relay_state_t relay_state = RELAY_OFF;

    test_relay();
    
    // Wait some time to be sure everything is ready
    DELAY(3000);
    
    for (;;)
    {
        // Get temperature readings and user input values
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
        dprintf("recv val=0x%x room=%u outside=%u\n", recv_temp,
            GETUPPER16(recv_temp), GETLOWER16(recv_temp));
        
        if (GETLOWER16(recv_temp) == MAGIC_SET_TEMP)
        {
            uint16_t t = GETUPPER16(recv_temp);
            dprintf("temperature from adc: %u\n", t);
        }

        
        // Store new temperature in case of user inputs
        if (GETLOWER16(recv_temp) == MAGIC_ACC_TEMP)
        {
            set_temp = GETUPPER16(recv_temp);
            dprintf("new temperature set: %u\n", set_temp);
        }

        // Refresh display
        xTaskNotify(display_task_h, recv_temp,
            (eNotifyAction) eSetValueWithOverwrite);
            
        // Decide if we need to turn on or off the relay
        if (GETLOWER16(recv_temp) != MAGIC_SET_TEMP &&
            GETLOWER16(recv_temp) != MAGIC_ACC_TEMP)
        {
            update_relay_state(&relay_state,
                GETUPPER16(recv_temp), set_temp, hyst);
            set_relay_state(relay_state);
        }
    }
}
