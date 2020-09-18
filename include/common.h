#ifndef __COMMON_H__
#define __COMMON_H__

/////////////////////////////////////////////////////

#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"

/////////////////////////////////////////////////////
// GPIO pin numbers

// DS18B20 temperature sensors
// (1-wire protocol)
#define PIN_DS18B20_ROOM    2 
#define PIN_DS18B20_OUTS    0

// 75HC595 Shift register for 
// seven-segment display control
#define PIN_74HC595_SER    16
#define PIN_74HC595_OE     14
#define PIN_74HC595_RCLK   12
#define PIN_74HC595_SRCLK  13

// GPIO pins for buttons
#define PIN_TACTILEBUTTON   4

// GPIO pin for relay control
#define PIN_RELAY           5



/////////////////////////////////////////////////////
// Temperature limits for user input
#define TEMP_SET_MIN (float) 18.0
#define TEMP_SET_MAX (float) 28.0

// Valid temperature limits
#define TEMP_MIN_VALID (float) -30.0
#define TEMP_MAX_VALID (float)  40.0

// Set temperature after boot
#define TEMP_INITIAL (float) 21.5

// Hysteresis value after boot
#define HYST_INITIAL (float) 0.5

// The value adc_read returns, when the
// temperature knob set to max resistance
#define ADC_MIN (uint16_t) 813

// It's always 1024 (see SDK docs) but it
// looks prettier this way
#define ADC_MAX (uint16_t) 1024



/////////////////////////////////////////////////////
// Miscellaneous

// Priorities for RTOS tasks
#define PRIO_DEFAULT 2

// Define a convient macro for delays,
// looks better when used frequently in the code
#define DELAY(x) vTaskDelay(x/portTICK_PERIOD_MS)

//Define time to automatically switch off display
#define DISPLAY_DELAY 5 /* sec */ *  1000



/////////////////////////////////////////////////////
// Macros for defining and handling 32-bit
// RTOS notification values used to transfer
// temperature values between tasks

// Constant to indicate failure
#define TEMP_ERR UINT16_MAX

// Constant added to the 16-bit
// unsigned temperature values to
// indicate if it's negative
#define TEMP_NEG 0x8000

// Magic number for the lower 16 bit
// of the task notification value
// to indicate that the value is
// from the temperature knob
// (in this mode, no outside
// temperature is displayed)
#define MAGIC_SET_TEMP 0x5E77

// Magic number for the lower 16 bit
// of the task notification value
// to notify the main task about the
// user's acceptance of the new
// temperature setting in set mode
#define MAGIC_ACC_TEMP 0xACCE

// Scaling and rounding floating point values
// to 32-bit unsigned integers.
// Hundredths precision is more than enough
// for household usage.
#define FLT2UINT32(x) (uint32_t) (x * 100)

// Macros for unpacking 16-bit unsigned ints
// from the packed 32-bit notification values
#define GETUPPER16(x) (uint16_t) (x >> 0x10)
#define GETLOWER16(x) (uint16_t) (x  & 0x0000ffff)



/////////////////////////////////////////////////////
// Debugging

// UART baud rate
#define BAUDRATE 9600

#define dprintf(fmt, ...) \
    printf ("%10u %s | " fmt, \
    (uint32_t) xTaskGetTickCount * portTICK_PERIOD_MS, \
    pcTaskGetName(NULL), ##__VA_ARGS__);
    
    
#endif
