#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "display.h"

/**
 * Driving four seven-segment displays to show
 * in- and outdoor temperatures with four
 * 75HC595 shift registers.
 */

// 7-segment digits byte values
//                     0     1     2     3     4	    
const uint8_t[10] = { 0x01, 0xCF, 0x92, 0x86, 0xCC,
                      0xA4, 0xA0, 0x8F, 0x00, 0x84 };
//                     5     6     7     8     9

static void shift_out(uint32_t value)
{
    gpio_write(SH, 0);
    gpio_write(ST, 0);
    
    for (uint8_t i = 0; i < 32; i++)
    {
        gpio_write(PIN_OUT_74HC595_DS, (value >> i) & 0x1);

	//clock
        gpio_write(PIN_OUT_74HC595_SRCLK, 1);
        DELAY(1);
        gpio_write(PIN_OUT_74HC595_SRCLK, 0);
    }
    
    //latch
    gpio_write(PIN_OUT_74HC595_RCLK, 1);
    DELAY(1);
    gpio_write(PIN_OUT_74HC595_RCLK, 0);
}

static void gpio_init(void)
{
    gpio_enable(PIN_OUT_74HC595_DS,    GPIO_OUTPUT);
    gpio_enable(PIN_OUT_74HC595_RCLK,  GPIO_OUTPUT);
    gpio_enable(PIN_OUT_74HC595_SRCLK, GPIO_OUTPUT);
}

///////////////////////////////////////////////////////////////

uint32_t temp_values_to_bytes()
{
}

void display_control_task(void *pvParameters)
{
    gpio_init();

    while(1)
    {
        gpio_write(gpio, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_write(gpio, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
