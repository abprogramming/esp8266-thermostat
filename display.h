#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/** API for controlling the display state (on/off)
 *(which basically boils down to pulling the
 * OE pin of 75HC595 to high/low level)
 */

typedef enum
{
    DISPLAY_OFF,
    DISPLAY_ON
} display_state_t;

void set_display_state(display_state_t state);

void display_control_task(void *pvParameters);

#endif
