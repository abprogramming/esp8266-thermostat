#include "common.h"
#include "clock.h"
#include <time.h>
#include <semphr.h>

static uint32_t UPTIME;
static uint32_t TIMESTAMP;
static SemaphoreHandle_t sem_ts;
static bool clock_started = false;

void ts_to_str(uint32_t ts, char* buf)
{
    time_t unixtime = ts;
    struct tm stm = *gmtime(&unixtime);
    strftime(buf, 9, "%H:%M", &stm);
}

uint32_t get_time(void)
{
    if (!clock_started)
    {
        return 0;
    }

    uint32_t out = 0;
    if (sem_ts != NULL)
    {
        if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
        {
            out = TIMESTAMP;
            xSemaphoreGive(sem_ts);
        }
    }
    return out;
}

uint32_t get_uptime(void)
{
    if (!clock_started)
    {
        return 0;
    }

    uint32_t out = 0;
    if (sem_ts != NULL)
    {
        if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
        {
            out = UPTIME;
            xSemaphoreGive(sem_ts);
        }
    }
    return out;
}

void set_time(uint32_t ts)
{
    if (!clock_started)
    {
        return;
    }

    if (sem_ts != NULL)
    {
        if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
        {
            if (ts > TIMESTAMP)
            {
                TIMESTAMP = ts;
            }
            xSemaphoreGive(sem_ts);
        }
    }
}

static void clock_task(void *pvParameters)
{
    sem_ts = xSemaphoreCreateMutex();
    for (;;)
    {
        if (sem_ts != NULL)
        {
            if (xSemaphoreTake(sem_ts, (TickType_t) 0 ))
            {
                UPTIME++;
                TIMESTAMP++;
                xSemaphoreGive(sem_ts);
            }
        }
        DELAY(1000);
    }
}

BaseType_t start_clock()
{
    UPTIME = 0;
    TIMESTAMP = 0;
    BaseType_t r = xTaskCreate
        (&clock_task, "Clock", 256, NULL, PRIO_HIGH, NULL);

    if (r == pdPASS)
    {
        clock_started = true;
    }
    return r;
}
