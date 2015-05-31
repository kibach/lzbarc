#include <mutest.h>
#include "bitvector.h"

void mu_test_simple() {
    bitvector t;
    uint16_t b;
    uint8_t b1;
    BV_init(&t, NULL, NULL, 0);
    BV_push_bit(t, 0);
    BV_push_bit(t, 1);
    BV_push_bit(t, 1);
    BV_push_bit(t, 0);
    b = 1032;
    BV_push_bytes(t, (uint8_t *) &b, 2, 12);
    BV_pop_bytes(t, &b1, 8);
    mu_ensure(b1 == 86);
    BV_pop_bytes(t, &b1, 8);
    mu_ensure(b1 == 40);
    BV_push_bit(t, 1);
    BV_fill(t);
    BV_pop_bytes(t, &b1, 8);
    mu_ensure(b1 == 128);
}
