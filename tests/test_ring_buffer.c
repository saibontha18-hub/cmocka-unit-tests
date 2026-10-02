/* Unit tests for ring_buffer using CMocka. */

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "ring_buffer.h"

#define CAP 8

static uint8_t storage[CAP];
static ring_buffer_t rb;

static int setup(void **state)
{
    (void) state;
    rb_init(&rb, storage, CAP);
    return 0;
}

static void test_init_state(void **state)
{
    (void) state;
    assert_true(rb_is_empty(&rb));
    assert_false(rb_is_full(&rb));
    assert_int_equal(rb_count(&rb), 0);
    assert_int_equal(rb_capacity(&rb), CAP);
}

static void test_put_get_fifo(void **state)
{
    uint8_t out;
    (void) state;

    assert_int_equal(rb_put(&rb, 0xAA), 0);
    assert_int_equal(rb_put(&rb, 0xBB), 0);
    assert_int_equal(rb_put(&rb, 0xCC), 0);
    assert_int_equal(rb_count(&rb), 3);

    assert_int_equal(rb_get(&rb, &out), 0);
    assert_int_equal(out, 0xAA);
    assert_int_equal(rb_get(&rb, &out), 0);
    assert_int_equal(out, 0xBB);
    assert_int_equal(rb_get(&rb, &out), 0);
    assert_int_equal(out, 0xCC);

    assert_true(rb_is_empty(&rb));
}

static void test_overflow_rejected(void **state)
{
    uint8_t out;
    size_t i;
    (void) state;

    for (i = 0; i < CAP; i++)
        assert_int_equal(rb_put(&rb, (uint8_t) i), 0);

    assert_true(rb_is_full(&rb));
    assert_int_equal(rb_put(&rb, 0xFF), -1); /* must not overwrite */
    assert_int_equal(rb_count(&rb), CAP);

    /* contents intact after rejected put */
    for (i = 0; i < CAP; i++) {
        assert_int_equal(rb_get(&rb, &out), 0);
        assert_int_equal(out, (uint8_t) i);
    }
}

static void test_underflow_rejected(void **state)
{
    uint8_t out = 0x5A;
    (void) state;

    assert_int_equal(rb_get(&rb, &out), -1);
    assert_int_equal(out, 0x5A); /* untouched on failure */
}

static void test_wrap_around(void **state)
{
    uint8_t out;
    size_t i;
    (void) state;

    /* Fill, drain half, then fill again to force index wrap. */
    for (i = 0; i < CAP; i++)
        assert_int_equal(rb_put(&rb, (uint8_t) (10 + i)), 0);
    for (i = 0; i < CAP / 2; i++) {
        assert_int_equal(rb_get(&rb, &out), 0);
        assert_int_equal(out, (uint8_t) (10 + i));
    }
    for (i = 0; i < CAP / 2; i++)
        assert_int_equal(rb_put(&rb, (uint8_t) (20 + i)), 0);

    assert_true(rb_is_full(&rb));

    for (i = 0; i < CAP / 2; i++) {
        assert_int_equal(rb_get(&rb, &out), 0);
        assert_int_equal(out, (uint8_t) (10 + CAP / 2 + i));
    }
    for (i = 0; i < CAP / 2; i++) {
        assert_int_equal(rb_get(&rb, &out), 0);
        assert_int_equal(out, (uint8_t) (20 + i));
    }
    assert_true(rb_is_empty(&rb));
}

static void test_fill_drain_cycle(void **state)
{
    uint8_t out;
    int round;
    size_t i;
    (void) state;

    /* Several full fill/drain cycles: order must stay FIFO every time. */
    for (round = 0; round < 5; round++) {
        for (i = 0; i < CAP; i++)
            assert_int_equal(rb_put(&rb, (uint8_t) (round * CAP + i)), 0);
        for (i = 0; i < CAP; i++) {
            assert_int_equal(rb_get(&rb, &out), 0);
            assert_int_equal(out, (uint8_t) (round * CAP + i));
        }
    }
    assert_true(rb_is_empty(&rb));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_init_state, setup),
        cmocka_unit_test_setup(test_put_get_fifo, setup),
        cmocka_unit_test_setup(test_overflow_rejected, setup),
        cmocka_unit_test_setup(test_underflow_rejected, setup),
        cmocka_unit_test_setup(test_wrap_around, setup),
        cmocka_unit_test_setup(test_fill_drain_cycle, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
