#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define PIN_OUT_74HC595_DS 13
#define PIN_OUT_74HC595_RCLK 15
#define PIN_OUT_74HC595_SRCLK 14

#define DELAY(x) vTaskDelay(x/portTICK_PERIOD_MS)

void display_control_task(void *pvParameters);

#endif
