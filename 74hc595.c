#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "common.h"
#include "74hc595.h"

/**
 * Minimalistic driver for the 74HC595
 * shift register, up to 32 bits
 */
 
void shift_init(void)
{
    gpio_enable(PIN_74HC595_SER,   GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_OE,    GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_RCLK,  GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_SRCLK, GPIO_OUTPUT);
}

void shift_out(uint32_t value, size_t bits)
{
    gpio_write(PIN_74HC595_RCLK, 0);
    gpio_write(PIN_74HC595_SRCLK, 0);

    for (size_t i = 0; i < bits; i++)
    {
        gpio_write(PIN_74HC595_SER, (value >> i) & 0x1);

       	// Tick clock
        gpio_write(PIN_74HC595_SRCLK, 1);
        DELAY(1);
        gpio_write(PIN_74HC595_SRCLK, 0);
    }

   	// Latch
    gpio_write(PIN_74HC595_RCLK, 1);
    DELAY(1);
    gpio_write(PIN_74HC595_RCLK, 0);
}
