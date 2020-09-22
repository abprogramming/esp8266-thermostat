#include <string.h>
#include "common.h"
#include "7segment.h"

/////////////////////////////////////////////////////
// Character set
   
const uint8_t digits[] = {
// 0     1     2     3     4     
  0x81, 0xCF, 0x92, 0x86, 0xCC,
// 5     6     7     8     9
  0xA4, 0xA0, 0x8F, 0x80, 0x84
};

const uint8_t letters[] = {
// A     b     c     d     E     F     G     H     i
  0x88, 0xE0, 0xB1, 0xC2, 0xB0, 0xB8, 0xA1, 0xE8, 0xEF,
// J    (K)     L   (M)    n     o     P     q     r 
  0xC3, 0xFF, 0xF1, 0xFF, 0xEA, 0xE2, 0xB0, 0x8C, 0xFA,
// S     t     U    (V)   (W)   (X)    y     Z
  0xA4, 0xF0, 0xC1, 0xFF, 0xFF, 0xFF, 0xC4, 0x92
};

// For decimal point (and on the thermostat board, 
// rectangular LED for minus sign)
// On my hardware setup, the DP pin is controlled by
// the MSB of the 74HC595, since it's output, the
// QA pin (15) is separated from the rest (pins 1-7)
const uint8_t DP  = 0x80;

/////////////////////////////////////////////////////

static struct _74hc595_t iface;

bool _7segment_module_create
    (struct _74hc595_pins_t* pincfg, uint8_t char_cnt)
{
    iface = hc595_module_create(pincfg, char_cnt);
    if (iface.ic_count == 0)
    {
        dprintf("Error creating shift register interface!\n")
        return false;
    }
    
    return true;
}

bool _7segment_print(const char* str)
{
    size_t sz = strlen(str);
    
    if (sz == 0)
    {
        return false;
    }
    
    if (iface.ic_count == 0)
    {
        dprintf("Error: shift register interface uninitalized!\n")
        return false;
    }
    
    size_t cnt = 0;
    uint8_t shift = 24;
    uint32_t out = 0x0;
    
    for (size_t i = 0; i < sz; i++)
    {
        char c = str[i];

	// Ignore spaces
        if (c == ' ')
        {
            continue;
        }
        
        uint8_t val = 0xFF;

        if (c >= 'a' && c <= 'z')
        {
            dprintf("lc");
            val = letters[(size_t) (c - 'a')];
        }
        else if (c >= 'A' && c <= 'Z')
        {
            val = letters[(size_t) (c - 'A')];
        }
        else if (c >= '0' && c <= '9')
        {
            val = digits[(size_t) (c - '0')];
        }
        
        if (val == 0xFF)
        {
            dprintf("Warning: character '%c' is not supported,"
                    " display will stay blank!\n", c);
        }
        
        // Examine if there's at least one more
        // character in the input string and see
        // if it's a decimal point...
        if (i + 1 < sz)
        {
            if (str[i + 1] == '.')
            {
                // Because of negative logic, the MSB
                // is always on and it should be turned 
                // off to display the decimal point
                val -= DP;
                i++;
            }
        } 
        
        out += val << shift;
        
        dprintf("write %c %u shift %u out %u\n", c, val, shift, out);
        shift -= 8;
        cnt++;
        
        if (cnt > iface.ic_count)
        {
            dprintf("Overflow warning: displaying only %u characters"
                    " are supported!\n", iface.ic_count);
        }
    }
    
    shift_out(&iface, out, 32);
    return true;
}

/////////////////////////////////////////////////////

// Controlling the display on/off states by 74HC595's OE pin.
void _7segment_set_state(display_state_t state)
{
    // Negative logic!
    switch (state)
    {
        case DISPLAY_OFF:
            toggle_oe_pin(&iface, 1);
            break;
        case DISPLAY_ON:
            toggle_oe_pin(&iface, 0);
            break;
    }
}
