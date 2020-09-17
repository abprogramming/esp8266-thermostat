#include "common.h"
#include "74hc595.h"


void shift_init(void)
{
    gpio_enable(PIN_74HC595_SER,   GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_OE,    GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_RCLK,  GPIO_OUTPUT);
    gpio_enable(PIN_74HC595_SRCLK, GPIO_OUTPUT);
    
    // Clear latches
    shift_out(0xFFFFFFFF, 32);
}

void shift_out(uint32_t value, size_t bits)
{
    gpio_write(PIN_74HC595_RCLK,  0);
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
