#include "common.h"
#include "main.h"
#include "temp.h"
#include "relay.h"
#if HAS_DISPLAY
# include "display.h"
# include "input.h"
#endif
#include "server.h"
#include "client.h"
#include "config.h"


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

// Task handles
static TaskHandle_t main_task_h    = NULL;
static TaskHandle_t temp_task_h    = NULL;

#if !IS_CLIENT
static TaskHandle_t httpd_task_h   = NULL;
#endif

#if HAS_DISPLAY
static TaskHandle_t display_task_h = NULL;
static TaskHandle_t input_task_h   = NULL;
#endif

// Entry point: create tasks
void user_init(void)
{
    uart_set_baud(0, BAUDRATE);

    BaseType_t res;
    
    // Main flow control
    res = xTaskCreate(&main_task, "main_task",
            1024, NULL, PRIO_DEFAULT, &main_task_h);
    check_task_creation_result(res, "main");


    // Each task receives the handle for the main task

    // Read temperature data from sensors
    res = xTaskCreate(&read_temp_task, "read_temp",
        256, (void*) main_task_h, PRIO_DEFAULT, &temp_task_h);
    check_task_creation_result(res, "temperature sensor");

#if HAS_DISPLAY
    // Handle the 7-segment displays
    res = xTaskCreate(&display_control_task, "display_control",
        256, (void*) main_task_h, PRIO_DEFAULT, &display_task_h);
    check_task_creation_result(res, "display control");

    // Handle user input, button, switch and potentiometer
    res = xTaskCreate(&input_control_task, "input_task",
        256, (void*) main_task_h, PRIO_DEFAULT, &input_task_h);
    check_task_creation_result(res, "input control");
#endif

#if IS_CLIENT
    // Initalize device as WiFi station and
    // try to connect to a thermostat server
    client_init();
#else
    // Initialize SoftAP and HTTP server
    // the function returns a handle to the task created
    httpd_task_h = server_init(main_task_h);
#endif

}

void main_task(void *pvParameters)
{
    // Variables storing actual and target temperature
    uint32_t recv_temp;
    
#if !IS_CLIENT
    // Target temperature from user
    uint16_t set_temp = (uint16_t) FLT2UINT32(TEMP_INITIAL);

    // Initialize feedback LED for
    // forced off mode
    bool force_on = false;
    gpio_enable(PIN_LED, GPIO_OUTPUT);
    gpio_write (PIN_LED,  1);  

    // Initialize relay control object
    struct relay_module_t relay = relay_module_create(PIN_RELAY);
    
#endif

    // Wait some time to be sure everything is ready
    DELAY(5000);

    for (;;)
    {
        // This is true, if the received task notification value
        // indicates a "normal" temperature from the sensor(s), 
        // not some special magic/control value
        bool normal_temp = true;

        // Get temperature readings and user input values
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);

        //dprintf("recv val=0x%x\n", recv_temp);
        
#if !IS_CLIENT
        if (GETLOWER16(recv_temp) == MAGIC_SET_TEMP)
        {
            //uint16_t t = GETUPPER16(recv_temp);
            normal_temp = false;
        }

        // Store new temperature in case of user inputs
        if (GETLOWER16(recv_temp) == MAGIC_ACC_TEMP)
        {
            set_temp = GETUPPER16(recv_temp);
            normal_temp = false;
        }

        // Handle forceon input from server module
        if (recv_temp == MAGIC_FORCERELAY_ON ||
            recv_temp == MAGIC_FORCERELAY_OFF)
        {
            force_on =
                (recv_temp == MAGIC_FORCERELAY_ON ? true : false);
            normal_temp = false;
        }
        
         if (GETLOWER16(recv_temp) == MAGIC_HYSTERESIS)
        {
            relay.hysteresis = GETUPPER16(recv_temp) * 10;
            normal_temp = false;
        }
#endif

#if HAS_DISPLAY
        // Refresh display
        xTaskNotify(display_task_h, recv_temp,
            (eNotifyAction) eSetValueWithOverwrite);
#endif

        // If the received notification value is
        // a temperature reading...
        if (normal_temp)
        {
            
#if IS_CLIENT
            //...if it's a client application,
            // send it to the server
            if (tcp_ready())
            {
                client_connect(recv_temp);
                DELAY(10 * 60 * 1000);
            }
            else
            {
                dprintf("FATAL ERROR: failed to connect server, "
                        "restarting system...\n");
                restart_device();
            }
#else

            // ...if a server apllication,
            // send temperatures to server module
            xTaskNotify(httpd_task_h, recv_temp,
                (eNotifyAction) eSetValueWithOverwrite);

            // And decide if we need to turn on or off the relay
            relay_update_ret_t ret;
            if (force_on)
            {
                // Force mode: never turn on the relay
                // and feedback this mode by turning the LED on
                ret = update_relay_state(&relay, 2000, 1000);
                gpio_write(PIN_LED, 0);

            }
            else
            {
                ret = update_relay_state
                    (&relay, recv_temp, set_temp);
                gpio_write(PIN_LED, 1);
            }

            // If the relay state has changed:
            // notify the server task and send the
            // temperature which triggered the change
            if (ret == STATE_CHANGED)
            {
                uint32_t notify_val = 0;
                notify_val   = recv_temp;
                if (force_on)
                    notify_val = MAGIC_FORCERELAY_ON;
                notify_val <<= 16;
                uint32_t magic =
                    (relay.state == RELAY_OFF ?
                                    MAGIC_RELAY_OFF :
                                    MAGIC_RELAY_ON);
                notify_val  += magic;
                xTaskNotify(httpd_task_h, notify_val,
                     (eNotifyAction) eSetValueWithOverwrite);
            }
#endif
        }
    }
}


// Restart device
void restart_device(void)
{
    fflush(stdout);
    uart_flush_txfifo(0);
    uart_flush_txfifo(1);
    DELAY(10);
    sdk_system_restart();
}

// If any of these critical calls above fails, we should
// not continue and try to restart the MCU ASAP
void check_task_creation_result(BaseType_t r, char *name)
{
    if (r != pdPASS)
    {
        dprintf("FATAL ERROR: failed to create %s task, "
                "restarting system...\n", name);
        restart_device();
    }
}
