#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include "ds18b20/ds18b20.h"

#include "common.h"
#include "temp.h"


/**
 * Detect DS18B20 sensors, get
 * temperature readings and transmit 
 * it to main task.
 * 
 * I've used a TO92 packaged one for
 * as a room temperature sensor and 
 * a waterproof version for outside.
 * Although they could be connected on a 
 * single GPIO pin, I am using separate ones
 * to avoid hardcoded serial numbers &c.
 */
 
#define MAX_SENSORS 2

float get_temperature_reading(uint8_t pin)
{
    int r = 0;
    float temp;
    int sensor_cnt = 0;
    ds18b20_addr_t addrs[MAX_SENSORS];

    sensor_cnt = ds18b20_scan_devices(pin, addrs, MAX_SENSORS);
    if (sensor_cnt != 1)
    {
        printf("ERROR: temperature sensor detection error on pin %u (found:%d)!\n",
            pin, sensor_cnt);
        return (float) TEMP_ERR;
    }

    r = ds18b20_measure(pin, *addrs, 1);
    DELAY(1000);
    if (!r)
        return (float) TEMP_ERR;
    temp = ds18b20_read_temperature(pin, *addrs);
    return temp;
}

// This function scales temperatures to convert
// then to integers and then packs the two
// separate readings to a single 32-bit integer
// to be sent as an RTOS task notification value.
// The higher 16 bits will hold the inside temp.

uint32_t pack_floats(float *t)
{
    uint32_t neg = 0;
    uint32_t t0 = FLT2UINT32(t[0]);
   	// Outside temp. can be negative
    if (t[1] < 0)
    {
        t[1] = fabs(t[1]);
        neg = TEMP_NEG;
    }
    uint32_t t1 = FLT2UINT32(t[1]) + neg;
    t0 <<= 0x10;
    return (t0 + t1);
}

void read_temp_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;

    float temps[2];
    uint32_t notify_val;

    for (;;)
    {
        temps[0] = 23.5865;	//get_temperature_reading(PIN_DS18B20_ROOM);
        temps[1] = -34.1223;	//get_temperature_reading(PIN_DS18B20_OUTS);
        DELAY(2000);
        notify_val = pack_floats(&temps);
        xTaskNotify(main_task_h, notify_val,
            (eNotifyAction) eSetValueWithOverwrite);
        /*	
        	temps[0] = 12.9865;	//get_temperature_reading(PIN_DS18B20_ROOM);
        	temps[1] = TEMP_ERR;	//get_temperature_reading(PIN_DS18B20_OUTS);
        	DELAY(2000);
        	notify_val = pack_floats(&temps);
        	xTaskNotify(main_task_h, notify_val,
        		(eNotifyAction)eSetValueWithOverwrite);

        	temps[0] = 25.0;	//get_temperature_reading(PIN_DS18B20_ROOM);
        	temps[1] = -12.45;	//get_temperature_reading(PIN_DS18B20_OUTS);
        	DELAY(2000);
        	notify_val = pack_floats(&temps);
        	xTaskNotify(main_task_h, notify_val,
        		(eNotifyAction)eSetValueWithOverwrite);
        		*/
    }
}
