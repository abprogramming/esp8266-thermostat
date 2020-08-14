#ifndef __MAIN_H__
#define __MAIN_H__

// Task handles
static TaskHandle_t main_task_h    = NULL;
static TaskHandle_t temp_task_h    = NULL;
static TaskHandle_t display_task_h = NULL;
static TaskHandle_t input_task_h   = NULL;

void user_init(void);
void main_task(void *pvParameters);

#endif
