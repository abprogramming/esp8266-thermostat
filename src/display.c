#include "common.h"
#include "display.h"
#include "7segment.h"

/**
 * The display consists of four seven-segments
 * and an additional, rectangle-shaped led for
 * the negative sign for outside temperatures.
 * 
 * The precision of the display is 0.5 Celsius.
 * The received values are rounded to this value.
 * The half degree-part of the value is implemented
 * and displayed using the decimal point (DP) of 
 * the second and the fourth seven-segment display.
 * It is connected to the Q(A) pin of U2 and U4 ICs,
 * while the negative sign is connected to the Q(A)
 * pin of U3. The Q(A) pin of U1 remains unconnected.
 */

static inline
uint16_t round_to_multiple(uint16_t n, uint16_t multiple)
{
    return ((n + multiple / 2) / multiple) *multiple;
}

static void test_display(void)
{
    _7segment_set_state(DISPLAY_ON);
    _7segment_print("test");
    DELAY(3000);
}

void display_control_task(void *pvParameters)
{
    uint32_t recv_temp;
    char str[8];
    bool set_mode = false;
    bool first = true;
    
    // Configure pins for shift register
    // and initialize seven-segment display driver
    struct _74hc595_pins_t pcfg;
    pcfg.pin_ser   = PIN_74HC595_SER;
    pcfg.pin_oe    = PIN_74HC595_OE;
    pcfg.pin_rclk  = PIN_74HC595_RCLK;
    pcfg.pin_srclk = PIN_74HC595_SRCLK; 
    if (!(_7segment_module_create(&pcfg, 4)))
    {
        return;
    }
    test_display();
    
    for (;;)
    {
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
            

            
        if (GETLOWER16(recv_temp) == MAGIC_SET_TEMP)
        {
            set_mode = true;
            uint16_t user_temp = round_to_multiple(GETUPPER16(recv_temp), 50);
            char dp = (user_temp % 100 == 0 ? ' ' : '.'); 
            snprintf(str, 8, "SE%u%c", user_temp / 100, dp);
            _7segment_print(str);
        }
        
        else if (GETLOWER16(recv_temp) == MAGIC_ACC_TEMP)
        {
            // If the temperature is accepted, display it for
            // 3 seconds and then return to normal mode
            uint16_t user_temp = round_to_multiple(GETUPPER16(recv_temp), 50);
            char dp = (user_temp % 100 == 0 ? ' ' : '.'); 
            snprintf(str, 8, "AC%u%c", user_temp / 100, dp);
            _7segment_print(str);
            DELAY(3000);
            set_mode = false;
            first = true;
            continue;
        }
        
        // Block normal temperature display in set mode
        if (!set_mode)
        {
            uint16_t room = round_to_multiple(GETUPPER16(recv_temp), 50);
            uint16_t outs = round_to_multiple(GETLOWER16(recv_temp), 50);
            snprintf(str, 8, "%u%u", room / 100, outs / 100);
            _7segment_print(str);
        }
        
        if (first)
        {
            first = false;
            DELAY(3000);
            _7segment_set_state(DISPLAY_OFF);
        }
        
        DELAY(10);
    }
}
