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
// Temperature limits, which can be set
#define TEMP_MIN (float) 18.0
#define TEMP_MAX (float) 26.0

// Set temperature after boot
#define TEMP_INITIAL (float) 21.5

// Hysteresis value after boot
#define HYST_INITIAL (float) 0.5


/////////////////////////////////////////////////////
// GPIO pin numbers

// DS18B20 temperature sensors
// (1-wire protocol)
#define PIN_DS18B20_ROOM    0 
#define PIN_DS18B20_OUTS    2

// 75HC595 Shift register for 
// seven-segment display control
#define PIN_74HC595_SER    12
#define PIN_74HC595_OE     14
#define PIN_74HC595_RCLK    4
#define PIN_74HC595_SRCLK   5

// GPIO pins for buttons
#define PIN_TACTILEBUTTON  13

// GPIO pin for relay control
#define PIN_RELAY          16


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
// Miscellaneous settings and
// definitions for common use

// Priorities for RTOS taska
#define PRIO_DEFAULT 2

// UART baud rate for debugging
#define BAUDRATE 9600

// Define a convient macro for delays,
// looks better when used frequently in the code
#define DELAY(x) vTaskDelay(x/portTICK_PERIOD_MS)

//Define time to automatically switch off display
#define DISPLAY_DELAY 5 /* sec */ *  1000

#endif
