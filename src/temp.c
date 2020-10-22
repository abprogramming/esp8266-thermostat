#include "common.h"
#include "math.h"
#include "ds18b20/ds18b20.h"
#include "temp.h"


/**
 * Detect DS18B20 sensors, get
 * temperature readings and transmit
 * it to main task.
 */

static void validate_temperature
    (float *t, float min, float max)
{
    if (*t > max ||
        *t < min)
    {
        *t = TEMP_ERR;
    }
}

static float get_temperature_reading(uint8_t pin)
{
    int r = 0;
    float temp;
    int sensor_cnt = 0;
    const int max_sensor_cnt = 2;
    ds18b20_addr_t addrs[max_sensor_cnt];

    sensor_cnt = ds18b20_scan_devices(pin, addrs, max_sensor_cnt);
    if (sensor_cnt != 1)
    {
        //dprintf("ERROR: temperature sensor detection error on pin %u (found:%d)!\n",
        //    pin, sensor_cnt);
        return (float) TEMP_ERR;
    }

    r = ds18b20_measure(pin, *addrs, 1);
    DELAY(1000);
    if (!r)
        return (float) TEMP_ERR;
    temp = ds18b20_read_temperature(pin, *addrs);
    //dprintf("%f C from GPIO %d\n", temp, pin);

    return temp;
}

void read_temp_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;

    for (;;)
    {
        float temp = get_temperature_reading(PIN_DS18B20);

        // Ensure that the readings are between the pre-defined constraints.
        // If the outside temperature has any errors we can proceed.
        // If the room temperature has errors the reading is not sent to main task

#if IS_CLIENT
        validate_temperature(&temp, OUTS_TEMP_MIN_VALID, OUTS_TEMP_MAX_VALID);
        if (temp == TEMP_ERR)
        {
            continue;
        }
#else
        validate_temperature(&temp, ROOM_TEMP_MIN_VALID, ROOM_TEMP_MAX_VALID);
#endif // IS_CLIENT

        uint32_t notify_val = FLT2UINT32(temp);

        // This should NEVER happen, but we better check...
        if (GETLOWER16(notify_val) != MAGIC_SET_TEMP &&
            GETLOWER16(notify_val) != MAGIC_ACC_TEMP)
        {
            xTaskNotify(main_task_h, notify_val,
            (eNotifyAction) eSetValueWithOverwrite);
        }
    }
}
