#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>

/*
 * ring_buffer.h - Fixed-capacity byte ring (circular) buffer, C99.
 *
 * Single-producer / single-consumer friendly; no dynamic allocation.
 * The caller provides the storage array. All functions return 0 on
 * success, -1 on failure (full on put, empty on get).
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

int rb_put(ring_buffer_t *rb, uint8_t byte);       /* 0 ok, -1 full */
int rb_get(ring_buffer_t *rb, uint8_t *byte);     /* 0 ok, -1 empty */

size_t rb_count(const ring_buffer_t *rb);
size_t rb_capacity(const ring_buffer_t *rb);
int rb_is_empty(const ring_buffer_t *rb);
int rb_is_full(const ring_buffer_t *rb);

#endif /* RING_BUFFER_H */
