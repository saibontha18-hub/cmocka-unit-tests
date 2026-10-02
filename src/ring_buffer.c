#include "ring_buffer.h"

void rb_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity)
{
    rb->buf = storage;
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

int rb_put(ring_buffer_t *rb, uint8_t byte)
{
    if (rb->count == rb->capacity)
        return -1; /* full */

    rb->buf[rb->tail] = byte;
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count++;
    return 0;
}

int rb_get(ring_buffer_t *rb, uint8_t *byte)
{
    if (rb->count == 0)
        return -1; /* empty */

    *byte = rb->buf[rb->head];
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count--;
    return 0;
}

size_t rb_count(const ring_buffer_t *rb)
{
    return rb->count;
}

size_t rb_capacity(const ring_buffer_t *rb)
{
    return rb->capacity;
}

int rb_is_empty(const ring_buffer_t *rb)
{
    return rb->count == 0;
}

int rb_is_full(const ring_buffer_t *rb)
{
    return rb->count == rb->capacity;
}
