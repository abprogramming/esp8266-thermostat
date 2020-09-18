#include "common.h"
#include "relay.h"

static void relay_toggle(struct relay_module_t *r)
{
    // Negative logic!
    switch (r->state)
    {
        case RELAY_OFF:
            gpio_write(r->pin, 1);
            break;
        case RELAY_ON:
            gpio_write(r->pin, 0);
            break;
    }
}

void test_relay(struct relay_module_t *r)
{
    size_t c = 3;
    while (c--)
    {
    r->state = RELAY_ON;
    relay_toggle(r);
    DELAY(500);
    r->state = RELAY_OFF;
    relay_toggle(r);
    DELAY(500);
    }
}

void relay_init(struct relay_module_t *r, uint8_t pin)
{
    r->pin = pin;
    r->hysteresis = (uint16_t) FLT2UINT32(HYST_INITIAL);
    
    gpio_enable(r->pin, GPIO_OUTPUT);
    test_relay(r);
    
    r->state = RELAY_OFF;
    relay_toggle(r);
}

relay_update_ret_t update_relay_state
   (struct relay_module_t *r, uint16_t act, uint16_t tgt)
{
    dprintf("rs act %u tgt %u h %u\n", act, tgt, r->hysteresis);
    
    relay_update_ret_t ret = STATE_UNCHANGED;
    
    if (r->state == RELAY_OFF && 
        act <= tgt - r->hysteresis)
    {
        r->state = RELAY_ON;
        ret = STATE_CHANGED;
    }
    
    if (r->state == RELAY_ON && 
        act >= tgt + r->hysteresis)
    {
        r->state = RELAY_OFF;
        ret = STATE_CHANGED;
    }
    
    relay_toggle(r);
    return ret;
}
