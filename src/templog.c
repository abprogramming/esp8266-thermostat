#include "common.h"
#include "templog.h"


struct log_buffer_t log_buffer_init(size_t sz)
{ 
    struct log_buffer_t buf;
    buf.entries = sz;
    buf.first = calloc(buf.entries, sizeof(struct log_entry_t));
    buf.ptr_wr = buf.first;
    buf.ptr_rd = buf.first;
    return buf;
}

void log_buffer_push(struct log_buffer_t *buf, struct log_entry_t val)
{
    buf->ptr_wr++;
    if (buf->ptr_wr >= log_buffer_end(buf))
        buf->ptr_wr = buf->first;
    *buf->ptr_wr = val;
}

struct log_entry_t log_buffer_getnext(struct log_buffer_t *buf)
{
    buf->ptr_rd++;
    if (buf->ptr_rd >= log_buffer_end(buf))
        buf->ptr_rd = buf->first;
    return (*buf->ptr_rd);
}

void log_buffer_reset(struct log_buffer_t *buf)
{
    buf->ptr_rd = buf->first;
}

struct log_entry_t *log_buffer_end(struct log_buffer_t *buf)
{
    return buf->first + (sizeof(struct log_entry_t) * (buf->entries));
}


