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

static void test_display(void)
{
    _7segment_set_state(DISPLAY_ON);
    _7segment_print("test");
    DELAY(3000);
}

static inline
uint16_t round_to_multiple(uint16_t n, uint16_t multiple)
{
    return ((n + multiple / 2) / multiple) *multiple;
}

static inline
char get_dp(uint16_t temp)
{
    return (temp % 100 == 0 ? ' ' : '.'); 
}

static void display_set_temp(const char* str, uint32_t recv_temp)
{
    char s[8];
    uint16_t user_temp = round_to_multiple(GETUPPER16(recv_temp), 50);
    snprintf(s, 8, "%s%u%c",
        str, user_temp / 100,
        get_dp(user_temp));
    _7segment_print(s);
}

static void display_normal_temp(uint32_t recv_temp)
{
    char s[8];
    char neg = ' ';
    uint16_t room = round_to_multiple(GETUPPER16(recv_temp), 50);
    uint16_t outs = round_to_multiple(GETLOWER16(recv_temp), 50);
    if (outs & TEMP_NEG)
    {
    outs -= TEMP_NEG;
    neg = '.';
    }
    uint16_t _3rd = outs / 1000;
    snprintf(s, 8, "%u%c %u%c %u%c",
        // 1st two digits with optional DP
        room / 100, get_dp(room),
        // 3rd digit with optional minus sign
        _3rd, neg,
        // 4th digit with optinal DP
        (outs -(_3rd * 1000)) / 100,
        get_dp(outs));
    _7segment_print(s);
}

void display_control_task(void *pvParameters)
{
    uint32_t recv_temp;
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
        return;
    test_display();


    for (;;)
    {
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
            
            
        if (GETLOWER16(recv_temp) == MAGIC_SET_TEMP)
        {
        // Display user defined ('set') temperature
        display_set_temp("SE", recv_temp);
            set_mode = true;
        }
        
        else if (GETLOWER16(recv_temp) == MAGIC_ACC_TEMP)
        {
            // If the temperature is accepted, display it for
            // 3 seconds and then return to normal mode
            display_set_temp("AC", recv_temp);
            DELAY(3000);
            set_mode = false;
            first = true;
            continue;
        }
       

        if (!set_mode) // Block normal temperature display in set mode
        {
        display_normal_temp(recv_temp);
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
