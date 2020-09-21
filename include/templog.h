#ifndef __TEMPLOG_H__
#define __TEMPLOG_H__

/**
 * Store temperature history
 */

void log_task(void *pvParameters);
void get_log(uint32_t **log, size_t *bits);

#endif
