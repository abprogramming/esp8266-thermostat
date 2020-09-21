#include "common.h"
#include "templog.h"

struct log_entry_t
{
    uint32_t temps;
    uint16_t set_temp;
    bool     relay_state; 
};

struct log_buffer_t
{
    size_t entries;
    struct log_entry_t *first;
    struct log_entry_t *current;
};

void log_task(void *pvParameters)
{ 
    struct log_buffer_t buf;
    buf.entries = 8 * 60;
    buf.first = calloc(buf.entries,
                       sizeof(struct log_entry_t));
    buf.current = buf.first;
    
    
    for (;;)
    {
        
        DELAY(60 * 1000);
    }
}
