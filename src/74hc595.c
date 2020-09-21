#include "common.h"
#include "74hc595.h"

struct _74hc595_t hc595_module_create
    (struct _74hc595_pins_t* pincfg, uint8_t ic_count)
{
    struct _74hc595_t s;
    
    if (ic_count > MAX_IC_COUNT)
    {
        dprintf("Could not create 74HC595 module with %u ICs."
                "Maximum supported: %u\n", ic_count, MAX_IC_COUNT);
        s.ic_count = 0;
        return s;
    }

    s.ic_count = ic_count;
    s.pinconfig = *pincfg;
    gpio_enable(pincfg->pin_ser,   GPIO_OUTPUT);
    gpio_enable(pincfg->pin_oe,    GPIO_OUTPUT);
    gpio_enable(pincfg->pin_rclk,  GPIO_OUTPUT);
    gpio_enable(pincfg->pin_srclk, GPIO_OUTPUT);
    
    // Clear latches
    shift_out(&s, 0xFFFFFFFF, 32);
    
    return s;
}

void shift_out(struct _74hc595_t* s, uint32_t value, size_t bits)
{
    if (bits % 8 != 0)
    {
        dprintf("The defined bits value must be a multiple of 8!"
                " (value=%u bits=%u)\n", value, bits);
        return;
    }

    if (s->ic_count < (bits / 8))
    {
        dprintf("Could not write more bytes than the configured"
                "number of 74HC595 ICs (max. %u, want: %u)\n",
                s->ic_count, bits / 8);
        return;
    }
    
    gpio_write(s->pinconfig.pin_rclk,  0);
    gpio_write(s->pinconfig.pin_srclk, 0);

    for (size_t i = 0; i < bits; i++)
    {
        gpio_write(s->pinconfig.pin_ser, (value >> i) & 0x1);
        // Tick clock
        gpio_write(s->pinconfig.pin_srclk, 1);
        DELAY(1);
        gpio_write(s->pinconfig.pin_srclk, 0);
    }

    // Latch
    gpio_write(s->pinconfig.pin_rclk, 1);
    DELAY(1);
    gpio_write(s->pinconfig.pin_rclk, 0);
}

void toggle_oe_pin(struct _74hc595_t* s, uint8_t value)
{
    gpio_write(s->pinconfig.pin_oe, value);
}
