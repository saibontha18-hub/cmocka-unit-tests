/* Unit tests for hash_map using CMocka. */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include "hash_map.h"

#define CAP 8

static hm_entry_t entries[CAP];
static hash_map_t hm;

static int setup(void **state)
{
    (void) state;
    hm_init(&hm, entries, CAP);
    return 0;
}

static void test_init_state(void **state)
{
    (void) state;
    assert_int_equal(hm_count(&hm), 0);
}

static void test_insert_lookup(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_put(&hm, "alpha", 1), 0);
    assert_int_equal(hm_put(&hm, "beta", 2), 0);
    assert_int_equal(hm_count(&hm), 2);

    assert_int_equal(hm_get(&hm, "alpha", &v), 0);
    assert_int_equal(v, 1);
    assert_int_equal(hm_get(&hm, "beta", &v), 0);
    assert_int_equal(v, 2);
}

static void test_overwrite_keeps_count(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_put(&hm, "alpha", 1), 0);
    assert_int_equal(hm_put(&hm, "alpha", 42), 0);
    assert_int_equal(hm_count(&hm), 1);

    assert_int_equal(hm_get(&hm, "alpha", &v), 0);
    assert_int_equal(v, 42);
}

static void test_missing_key(void **state)
{
    int32_t v = 0x5A5A5A5A;
    (void) state;

    assert_int_equal(hm_put(&hm, "alpha", 1), 0);
    assert_int_equal(hm_get(&hm, "gamma", &v), -1);
    assert_int_equal(v, 0x5A5A5A5A);
}

static void test_delete(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_put(&hm, "alpha", 1), 0);
    assert_int_equal(hm_put(&hm, "beta", 2), 0);

    assert_int_equal(hm_delete(&hm, "alpha"), 0);
    assert_int_equal(hm_count(&hm), 1);
    assert_int_equal(hm_get(&hm, "alpha", &v), -1);

    /* deleting twice fails, and so does deleting a key that was never there */
    assert_int_equal(hm_delete(&hm, "alpha"), -1);
    assert_int_equal(hm_delete(&hm, "gamma"), -1);

    assert_int_equal(hm_get(&hm, "beta", &v), 0);
    assert_int_equal(v, 2);
}

static void test_delete_then_reinsert(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_put(&hm, "alpha", 1), 0);
    assert_int_equal(hm_delete(&hm, "alpha"), 0);
    assert_int_equal(hm_put(&hm, "alpha", 7), 0);
    assert_int_equal(hm_count(&hm), 1);
    assert_int_equal(hm_get(&hm, "alpha", &v), 0);
    assert_int_equal(v, 7);
}

static void test_table_full(void **state)
{
    char key[8];
    int32_t v = -1;
    size_t i;
    (void) state;

    for (i = 0; i < CAP; i++) {
        snprintf(key, sizeof(key), "k%zu", i);
        assert_int_equal(hm_put(&hm, key, (int32_t) i), 0);
    }
    assert_int_equal(hm_count(&hm), CAP);

    /* new key rejected when full */
    assert_int_equal(hm_put(&hm, "overflow", 99), -1);
    assert_int_equal(hm_count(&hm), CAP);

    /* overwriting an existing key is still fine when full */
    assert_int_equal(hm_put(&hm, "k3", 33), 0);
    assert_int_equal(hm_get(&hm, "k3", &v), 0);
    assert_int_equal(v, 33);
    assert_int_equal(hm_count(&hm), CAP);
}

/*
 * "aa", "ai" and "aq" all land in the same slot with capacity 8 (I checked
 * the hashes by hand), so inserting all three forces linear probing.
 */
static void test_collisions(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_fnv1a("aa") % CAP, hm_fnv1a("ai") % CAP);
    assert_int_equal(hm_fnv1a("aa") % CAP, hm_fnv1a("aq") % CAP);

    assert_int_equal(hm_put(&hm, "aa", 1), 0);
    assert_int_equal(hm_put(&hm, "ai", 2), 0);
    assert_int_equal(hm_put(&hm, "aq", 3), 0);

    assert_int_equal(hm_get(&hm, "aa", &v), 0);
    assert_int_equal(v, 1);
    assert_int_equal(hm_get(&hm, "ai", &v), 0);
    assert_int_equal(v, 2);
    assert_int_equal(hm_get(&hm, "aq", &v), 0);
    assert_int_equal(v, 3);
}

static void test_collision_delete_probing(void **state)
{
    int32_t v = -1;
    (void) state;

    assert_int_equal(hm_put(&hm, "aa", 1), 0);
    assert_int_equal(hm_put(&hm, "ai", 2), 0);

    /* deleting the first of a probe chain must not hide the second */
    assert_int_equal(hm_delete(&hm, "aa"), 0);
    assert_int_equal(hm_get(&hm, "ai", &v), 0);
    assert_int_equal(v, 2);

    /* a new colliding key reuses the tombstone slot */
    assert_int_equal(hm_put(&hm, "aq", 3), 0);
    assert_int_equal(hm_get(&hm, "aq", &v), 0);
    assert_int_equal(v, 3);
    assert_int_equal(hm_get(&hm, "ai", &v), 0);
    assert_int_equal(v, 2);
    assert_int_equal(hm_count(&hm), 2);
}

static void test_key_limits(void **state)
{
    char long_key[HM_KEY_MAX + 2];
    int32_t v = -1;
    (void) state;

    /* exactly HM_KEY_MAX chars is fine */
    memset(long_key, 'x', HM_KEY_MAX);
    long_key[HM_KEY_MAX] = '\0';
    assert_int_equal(hm_put(&hm, long_key, 9), 0);
    assert_int_equal(hm_get(&hm, long_key, &v), 0);
    assert_int_equal(v, 9);

    /* one char too many is rejected */
    long_key[HM_KEY_MAX] = 'x';
    long_key[HM_KEY_MAX + 1] = '\0';
    assert_int_equal(hm_put(&hm, long_key, 9), -1);
    assert_int_equal(hm_get(&hm, long_key, &v), -1);

    /* empty key works */
    assert_int_equal(hm_put(&hm, "", 5), 0);
    assert_int_equal(hm_get(&hm, "", &v), 0);
    assert_int_equal(v, 5);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_init_state, setup),
        cmocka_unit_test_setup(test_insert_lookup, setup),
        cmocka_unit_test_setup(test_overwrite_keeps_count, setup),
        cmocka_unit_test_setup(test_missing_key, setup),
        cmocka_unit_test_setup(test_delete, setup),
        cmocka_unit_test_setup(test_delete_then_reinsert, setup),
        cmocka_unit_test_setup(test_table_full, setup),
        cmocka_unit_test_setup(test_collisions, setup),
        cmocka_unit_test_setup(test_collision_delete_probing, setup),
        cmocka_unit_test_setup(test_key_limits, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
