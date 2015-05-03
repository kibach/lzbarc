#ifndef BITVECTOR_H_INCLUDED
#define BITVECTOR_H_INCLUDED
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

struct bitarray {
    uint8_t * bits;
    uint32_t bitsize;
    uint32_t fifo_start;
    uint32_t allocated_bytes;
    FILE * flush_to;
};

typedef struct bitarray * bitvector;

enum {
    BV_SUCCESS = 100,
    BV_RANGE_ERROR = 101,
    BV_INVALID_BIT_VALUE_ERROR = 102,
    BV_BUFFER_ERROR = 103,
    BV_MEMORY_ERROR = 104,
    BV_FLUSH_ERROR = 105
};

enum {
    BITS_PER_BYTE = 8,
    BYTES_PER_FLUSHBLOCK = 1024
};

extern uint8_t BV_init(bitvector *op, FILE * flush);
extern uint8_t BV_get_by_index(bitvector op, uint32_t index);
extern uint8_t BV_set_by_index(bitvector op, uint32_t index, uint8_t bit_value);
extern uint8_t BV_push_bit(bitvector op, uint8_t bit_value);
extern uint8_t BV_push_bytes(bitvector op, uint8_t * bytes, uint32_t bytes_size, uint32_t significant_bits_count);
extern uint8_t BV_fill(bitvector op);
extern void BV_close(bitvector *op);
extern uint8_t BV_flush(bitvector op);

#endif // BITVECTOR_H_INCLUDED
