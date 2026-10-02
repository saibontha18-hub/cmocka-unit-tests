#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>
#include <stdint.h>

/*
 * hash_map.h - Fixed-capacity string-keyed hash map, C99.
 *
 * Open addressing with linear probing and tombstones. No dynamic
 * allocation: the caller provides the entry storage array, just like
 * ring_buffer. Keys are hashed with FNV-1a (32-bit).
 *
 * All functions return 0 on success, -1 on failure (table full on
 * insert of a new key, missing key on lookup/delete, over-long key).
 * Inserting an existing key overwrites its value.
 */

#define HM_KEY_MAX 23  /* longest key, not counting the NUL terminator */

typedef enum
{
    HM_EMPTY,     /* never used */
    HM_OCCUPIED,  /* holds a live key/value pair */
    HM_DELETED    /* tombstone: keep probing past it */
} hm_state_t;

typedef struct
{
    hm_state_t state;
    char key[HM_KEY_MAX + 1];
    int32_t value;
} hm_entry_t;

typedef struct
{
    hm_entry_t *entries;
    size_t capacity;
    size_t count;   /* live entries */
} hash_map_t;

void hm_init(hash_map_t *m, hm_entry_t *entries, size_t capacity);

int hm_put(hash_map_t *m, const char *key, int32_t value);
int hm_get(const hash_map_t *m, const char *key, int32_t *value);
int hm_delete(hash_map_t *m, const char *key);

size_t hm_count(const hash_map_t *m);

/* Exposed for unit tests (collision crafting). */
uint32_t hm_fnv1a(const char *key);

#endif /* HASH_MAP_H */
