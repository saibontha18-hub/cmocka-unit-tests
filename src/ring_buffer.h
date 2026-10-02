#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>

/*
 * Fixed-capacity byte ring buffer. Caller provides the storage array,
 * so there's no malloc anywhere. Returns 0 on success, -1 on failure.
 */

typedef struct
{
    uint8_t *buf;
    size_t capacity;
    size_t head;   /* index of next byte to read */
    size_t tail;   /* index of next byte to write */
    size_t count;  /* bytes currently stored */
} ring_buffer_t;

void rb_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity);

int rb_put(ring_buffer_t *rb, uint8_t byte);
int rb_get(ring_buffer_t *rb, uint8_t *byte);

size_t rb_count(const ring_buffer_t *rb);
size_t rb_capacity(const ring_buffer_t *rb);
int rb_is_empty(const ring_buffer_t *rb);
int rb_is_full(const ring_buffer_t *rb);

#endif /* RING_BUFFER_H */
