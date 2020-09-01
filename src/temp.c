#include "common.h"
#include "math.h"
#include "ds18b20/ds18b20.h"
#include "temp.h"


/**
 * Detect DS18B20 sensors, get
 * temperature readings and transmit 
 * it to main task.
 * 
 * I've used a TO92 packaged one as
 * room temperature sensor and 
 * a waterproof version for the outside.
 * Although they could be connected on a 
 * single GPIO pin, I am using separate ones
 * to avoid hardcoded serial numbers.
 * 
 * However I've kept specific parts of the
 * code which would allow to have two (or more)
 * of them on oone GPIO pin.
 */

static void validate_temperature(float *t)
{
    if (*t > TEMP_MAX_VALID ||
        *t < TEMP_MIN_VALID)
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
        dprintf("ERROR: temperature sensor detection error on pin %u (found:%d)!\n",
            pin, sensor_cnt);
        return (float) TEMP_ERR;
    }

    r = ds18b20_measure(pin, *addrs, 1);
    DELAY(1000);
    if (!r)
        return (float) TEMP_ERR;
    temp = ds18b20_read_temperature(pin, *addrs);
    dprintf("%f C from GPIO %d\n", temp, pin);

    // Ensure that the reading is between the pre-defined constraints
    validate_temperature(&temp);
    return temp;
}

// This function scales temperatures to convert
// then to integers and then packs the two
// separate readings to a single 32-bit integer
// to be sent as an RTOS task notification value.
// The higher 16 bits will hold the inside temp.

inline static uint32_t pack_floats(float t[])
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
    t0 <<= 16;
    return (t0 + t1);
}

void read_temp_task(void *pvParameters)
{
    TaskHandle_t main_task_h = (TaskHandle_t) pvParameters;

    for (;;)
    {
        float temps[2];
        temps[0] = get_temperature_reading(PIN_DS18B20_ROOM);
        temps[1] = get_temperature_reading(PIN_DS18B20_OUTS);
        uint32_t notify_val = pack_floats(temps);
        
        // This should NEVER happen, but we better check...
        if (GETLOWER16(notify_val) != MAGIC_SET_TEMP &&
            GETLOWER16(notify_val) != MAGIC_ACC_TEMP)
        {
            xTaskNotify(main_task_h, notify_val,
            (eNotifyAction) eSetValueWithOverwrite);
        }
    }
}
