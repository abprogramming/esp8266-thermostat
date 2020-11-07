#ifndef __CLOCK_H__
#define __CLOCK_H__

/**
 * Minimal software implementation of a real-time clock
 */
 
// Returns a UNIX timestamp, must be set with
// set_time(), until that it equals uptime
uint32_t get_time(void);

uint32_t get_uptime(void);

void set_time(uint32_t ts);

// Convert timestamp to human-readable form
void ts_to_str(uint32_t ts, char* buf);

// Starts a high-priority clock task 
BaseType_t start_clock(void);

#endif
