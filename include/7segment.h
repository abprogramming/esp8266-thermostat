#ifndef __7SEGMENT_H__
#define __7SEGMENT_H__

#include "stdbool.h"
#include "74hc595.h"

/** 
 * API for controlling seven segment
 * displays with 74HC595 shift registers.
 */

typedef enum
{
    DISPLAY_OFF,
    DISPLAY_ON
} display_state_t;

bool _7segment_module_create
    (struct _74hc595_pins_t* pincfg, uint8_t char_cnt);

bool _7segment_print(const char* str);
    
void _7segment_set_state(display_state_t state);

#endif
