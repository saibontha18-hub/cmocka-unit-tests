#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stddef.h>
#include <stdint.h>

/*
 * Fixed-capacity string-keyed hash map. Open addressing, linear probing,
 * tombstones on delete; FNV-1a for hashing. Caller provides the entry
 * storage, so no malloc. Inserting an existing key overwrites it.
 */

#define HM_KEY_MAX 23  /* longest key, not counting the NUL terminator */

typedef enum
{
    HM_EMPTY,
    HM_OCCUPIED,
    HM_DELETED    /* tombstone: probes must skip past these */
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
