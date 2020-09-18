#ifndef __RELAY_H__
#define __RELAY_H__

/** 
 * API for controlling the relay
 * for turning the heater on/off. 
 */
 
typedef enum
{
    RELAY_OFF,
    RELAY_ON
} relay_state_t;

typedef enum
{
    STATE_UNCHANGED,
    STATE_CHANGED
} relay_update_ret_t;

struct relay_module_t
{
    uint8_t	pin;
    relay_state_t state;
    uint16_t hysteresis;
};

void relay_init(struct relay_module_t *r, uint8_t pin);

// Test the relay module: turn on/off three times
void test_relay(struct relay_module_t *r);

// Try to turn relay on/off according to the desired and
// the actual temperature. The function returns a relay_update_ret_t
// type value which indicates whether the relay state was changed or not.
relay_update_ret_t update_relay_state
   (struct relay_module_t *r, uint16_t act, uint16_t tgt);

#endif
