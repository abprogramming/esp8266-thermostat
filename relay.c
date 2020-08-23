#include "common.h"
#include "relay.h"


void relay_init(void)
{
    gpio_enable(PIN_RELAY, GPIO_OUTPUT);
}

void set_relay_state(relay_state_t state)
{
    // Negative logic!
    switch (state)
    {
        case RELAY_OFF:
            gpio_write(PIN_RELAY, 1);
            break;
        case RELAY_ON:
            gpio_write(PIN_RELAY, 0);
            break;
    }
}

void test_relay(void)
{
    set_relay_state(RELAY_ON);
    DELAY(500);
    set_relay_state(RELAY_OFF);
    DELAY(500);
    set_relay_state(RELAY_ON);
    DELAY(500);
    set_relay_state(RELAY_OFF);
    DELAY(500);
    set_relay_state(RELAY_ON);
    DELAY(500);
    set_relay_state(RELAY_OFF);
    DELAY(500);
}
