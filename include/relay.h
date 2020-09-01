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

void relay_init(void);
void set_relay_state(relay_state_t state);

// Test the relay module: turn on/off three times
void test_relay(void);

#endif
