#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

#include "common.h"
#include "display.h"
#include "74hc595.h"

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

// 7-segment digits byte values
//                            0     1     2     3     4	    
const uint8_t digits[10] = { 0x81, 0xCF, 0x92, 0x86, 0xCC,
                             0xA4, 0xA0, 0x8F, 0x80, 0x84 };
//                            5     6     7     8     9
// Mask to use if we want to drive Q(A) pin high
const uint8_t QA = 0x80;

// Digit to display in case of sensor failure
const uint8_t ERR_DISPLAY = 0xFE;


/**
 *This part is responsible for converting
 *two temperature readings to a 32-bit value
 *for the daisy-chained shift registers
 *(considering that the MSB should be on U1 and LSB
 *should be on U4, to be in the designated order).
 */

static inline
uint32_t round_to_multiple(uint32_t n, uint32_t multiple)
{
    return ((n + multiple / 2) / multiple) *multiple;
}

static uint32_t value_to_byte(uint32_t n)
{
    uint8_t v;
    uint16_t out;
    uint8_t digit1 = 0x0;
    uint8_t digit2 = 0x0;
    uint8_t neg = 0x0;
    uint8_t dot = 0x0;

   	// Handle error value
    if (n == TEMP_ERR)
    {
        out = (ERR_DISPLAY << 8) + ERR_DISPLAY;
        return out;
    }
    

   	// Handle if number is negative
    if (n & TEMP_NEG)
    {
        n -= TEMP_NEG;
        printf("temp neg %u\n", n);
        neg = QA;
    }
    

   	// The last digit is truncated
   	// by rounding so omit it
    n /= 10;


   	// The next digit determines if
   	// the decimal point should be turned on
    v = n % 10;
    if (v)
        dot = QA;

    printf("d1: %d\n", v);


   	// The next two digits are the original
   	// integer part, which is displayed as
   	// numbers on seven-segments

    n /= 10;
    v = n % 10;
    digit1 = digits[v] - neg;
    printf("d2: %d\n", v);

    n /= 10;
    v = n % 10;
    digit2 = digits[v] - dot;
    printf("d3: %d\n", v);
    

    out = (digit1 << 8) + digit2;
    printf("%u %u out: %u\n", digit1, digit2, out);
    return (uint32_t) out;
}

static uint32_t temp_values_to_bytes(uint32_t u)
{
    uint32_t out;
    uint32_t room = round_to_multiple(GETUPPER16(u), 50);
    uint32_t outs = round_to_multiple(GETLOWER16(u), 50);
    printf("rounded: %u %u\n", room, outs);
    out = (value_to_byte(room) << 16) + value_to_byte(outs);
    printf("out2: %u\n", out);
    return out;
}


/**
 * Display init, test sequence and controlling 
 * the display of digits and on/off states.
 * The test sequence consists of displaying
 * '8007' (BOOT) for 3 seconds, then turn off
 * the display.
 */

void set_display_state(display_state_t state)
{
        switch (state)
        {
            case DISPLAY_OFF:
                gpio_write(PIN_74HC595_OE, 0);
                break;
            case DISPLAY_ON:
                gpio_write(PIN_74HC595_OE, 1);
                break;
        }
}

static void test_display(void)
{
    uint32_t out = digits[8];
    out << 8;
    out += digits[0];
    out << 8;
    out += digits[0];
    out << 8;
    out += digits[7];
    shift_out(out, sizeof(out));
    set_display_state(DISPLAY_ON);
    DELAY(3000);
    set_display_state(DISPLAY_OFF);
}

void display_control_task(void *pvParameters)
{
    uint32_t recv_temp;
    uint32_t out;

    shift_init();
    test_display();

    for (;;)
    {
        xTaskNotifyWait((uint32_t) 0x0, (uint32_t) UINT32_MAX,
            (uint32_t*) &recv_temp, (TickType_t) portMAX_DELAY);
        out = temp_values_to_bytes(recv_temp);
        shift_out(out, sizeof(out));
        DELAY(1000);
    }
}
