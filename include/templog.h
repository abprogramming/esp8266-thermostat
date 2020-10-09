#ifndef __TEMPLOG_H__
#define __TEMPLOG_H__

/**
 * Buffer implementation for storing
 * temperature and relay history
 */

struct log_entry_t
{
    uint32_t ts;
    char v[28];
};

struct log_buffer_t
{
    size_t entries;
    struct log_entry_t *first;
    struct log_entry_t *ptr_wr;
    struct log_entry_t *ptr_rd;
};

struct log_buffer_t log_buffer_init(size_t sz);

// Increment write pointer and write new value
void log_buffer_push(struct log_buffer_t *buf, struct log_entry_t val);

// Read the next value. The reading is performed backwards!
struct log_entry_t log_buffer_getnext(struct log_buffer_t *buf);

// Set read pointer to the last written element
void log_buffer_reset(struct log_buffer_t *buf);

// Points to the memory space right after the last element
struct log_entry_t *log_buffer_end(struct log_buffer_t *buf);

#endif
