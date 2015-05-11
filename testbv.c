#include <stdio.h>
#include "bitvector.h"

int main() {
    bitvector t;
    uint8_t b;
    uint8_t b1, b2;
    BV_init(&t, 0);
    BV_push_bit(t, 0);
    b1 = BV_pop_bit(t);
    b = 'h';
    BV_push_bytes(t, &b, 1, 8);
    //BV_pop_bytes(t, &b2, 8);
    //printf("<0, %d>\n", b);
    //printf("<%d, %d>\n", b1, b2);
    //BV_push_bit(t, 0);
    b = 'e';
    BV_push_bytes(t, &b, 1, 8);
    //printf("<0, %d>\n", b);
    b1 = BV_pop_bit(t);
    BV_pop_bytes(t, &b2, 8);
    printf("<%d, %d>\n", b1, b2);
    b1 = BV_pop_bit(t);
    BV_pop_bytes(t, &b2, 8);
    printf("<%d, %d>\n", b1, b2);
    return 0;
}
