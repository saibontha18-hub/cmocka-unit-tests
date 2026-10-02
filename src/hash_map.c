#include "hash_map.h"

#include <string.h>

uint32_t hm_fnv1a(const char *key)
{
    uint32_t h = 2166136261u; /* FNV offset basis */
    while (*key) {
        h ^= (uint8_t) *key++;
        h *= 16777619u;       /* FNV prime */
    }
    return h;
}

void hm_init(hash_map_t *m, hm_entry_t *entries, size_t capacity)
{
    size_t i;

    m->entries = entries;
    m->capacity = capacity;
    m->count = 0;
    for (i = 0; i < capacity; i++)
        entries[i].state = HM_EMPTY;
}

static int key_ok(const char *key)
{
    return key != NULL && strlen(key) <= HM_KEY_MAX;
}

/*
 * Probe for `key`. On return:
 *   - *slot points at the entry holding the key, or at the slot where a new
 *     key should be inserted (first tombstone if any, else first empty slot).
 *   - returns 1 if the key was found, 0 otherwise.
 * Never loops more than `capacity` steps, so it always terminates.
 */
static int probe(const hash_map_t *m, const char *key, size_t *slot)
{
    size_t i, idx, first_deleted;
    int have_deleted = 0;

    if (m->capacity == 0)
        return 0;

    idx = hm_fnv1a(key) % (uint32_t) m->capacity;
    first_deleted = idx;

    for (i = 0; i < m->capacity; i++) {
        size_t s = (idx + i) % m->capacity;
        hm_entry_t *e = &m->entries[s];

        if (e->state == HM_EMPTY) {
            *slot = have_deleted ? first_deleted : s;
            return 0;
        }
        if (e->state == HM_DELETED) {
            if (!have_deleted) {
                first_deleted = s;
                have_deleted = 1;
            }
            continue;
        }
        if (strcmp(e->key, key) == 0) {
            *slot = s;
            return 1;
        }
    }

    /* Every slot is occupied or deleted: table is logically full. */
    *slot = first_deleted;
    return 0;
}

int hm_put(hash_map_t *m, const char *key, int32_t value)
{
    size_t slot;

    if (!key_ok(key))
        return -1;

    if (probe(m, key, &slot)) {
        m->entries[slot].value = value; /* overwrite */
        return 0;
    }
    if (m->count == m->capacity)
        return -1; /* full: no room for a new key */

    strncpy(m->entries[slot].key, key, HM_KEY_MAX);
    m->entries[slot].key[HM_KEY_MAX] = '\0';
    m->entries[slot].value = value;
    m->entries[slot].state = HM_OCCUPIED;
    m->count++;
    return 0;
}

int hm_get(const hash_map_t *m, const char *key, int32_t *value)
{
    size_t slot;

    if (!key_ok(key))
        return -1;

    if (probe(m, key, &slot)) {
        if (value)
            *value = m->entries[slot].value;
        return 0;
    }
    return -1;
}

int hm_delete(hash_map_t *m, const char *key)
{
    size_t slot;

    if (!key_ok(key))
        return -1;

    if (probe(m, key, &slot)) {
        m->entries[slot].state = HM_DELETED; /* tombstone */
        m->count--;
        return 0;
    }
    return -1;
}

size_t hm_count(const hash_map_t *m)
{
    return m->count;
}
