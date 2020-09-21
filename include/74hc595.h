#ifndef __74HC595_H__
#define __74HC595_H__

/**
 * Driver for the 74HC595
 * shift register, up to 32 bits
 */
 
 #define MAX_IC_COUNT 4
 
struct _74hc595_pins_t
{
    uint8_t	pin_ser;
    uint8_t	pin_oe;
    uint8_t	pin_rclk;
    uint8_t	pin_srclk;  
};

struct _74hc595_t
{   
    uint8_t ic_count; 
    struct _74hc595_pins_t pinconfig;
};

// On error, ic_count is set to 0!
struct _74hc595_t hc595_module_create
    (struct _74hc595_pins_t* pincfg, uint8_t ic_count);

void shift_out
    (struct _74hc595_t* s, uint32_t value, size_t bits);

void toggle_oe_pin(struct _74hc595_t* s, uint8_t value);

#endif
