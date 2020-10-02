#ifndef __CLOCK_H__
#define __CLOCK_H__

/**
 * Minimal software implementation of a real-time clock
 */
 
// Returns a UNIX timestamp, must be set with
// set_time(), until that it equals uptime
uint32_t get_time();

uint32_t get_uptime();

void set_time(uint32_t ts);

// Starts a high-priority clock task 
BaseType_t start_clock(void);

#endif
