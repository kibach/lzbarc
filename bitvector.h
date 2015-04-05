#ifndef BITVECTOR_H_INCLUDED
#define BITVECTOR_H_INCLUDED
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct bitarray {
    uint8_t * bits;
    uint32_t bitsize;
    uint32_t fifo_start;
};

typedef struct bitarray * bitvector;
enum {
    BV_SUCCESS = 100,
    BV_RANGE_ERROR = 101,
    BV_INVALID_BIT_VALUE_ERROR = 102,
    BV_BUFFER_ERROR = 103
};

const uint8_t BITS_PER_BYTE = 8;

/*
 * Initializes bitvector with 8 bits size and fills it with nulls.
 * TODO: pass bit size as optional argument.
 */
void BV_init(bitvector *op) {
    (*op) = malloc(sizeof(op));
    (*op)->bits = malloc(sizeof(uint8_t));
    (*op)->bits[0] = 0;
    (*op)->bitsize = 0;
    (*op)->fifo_start = 0;
    return;
}

/*
 * Gets bit value by its index in bitvector.
 * Uses easy-to-understand formula using bit shifts and conjunction.
 */
uint8_t BV_get_by_index(bitvector op, uint32_t index) {
    if (index > op->bitsize) {
        return BV_RANGE_ERROR;
    }
    return ((op->bits[index / BITS_PER_BYTE]) & (1 << ((index % BITS_PER_BYTE) - 1))) >> ((index % BITS_PER_BYTE) - 1);
}

/*
 * Sets bit by its index to bit_value..
 * Used formula found somewhere on StackOverflow.
 */
uint8_t BV_set_by_index(bitvector op, uint32_t index, uint8_t bit_value) {
    if (bit_value > 1) {
        return BV_INVALID_BIT_VALUE_ERROR;
    }
    if (index > op->bitsize) {
        return BV_RANGE_ERROR;
    }
    op->bits[index / BITS_PER_BYTE] ^= (-bit_value ^ op->bits[index / BITS_PER_BYTE]) & (1 << (index % BITS_PER_BYTE));
    return BV_SUCCESS;
}

/*
 * Adds bit with specified value to the end of bitvector.
 * Uses BV_set_by_index function.
 */
uint8_t BV_push_bit(bitvector op, uint8_t bit_value) {
    if ((op->bitsize > 0) && (op->bitsize % BITS_PER_BYTE == 0)) {
        uint8_t * new_bits;
        while ((new_bits = realloc(op->bits, ((op->bitsize / (float) BITS_PER_BYTE) + 1) * sizeof(uint8_t))) == NULL) {
            fprintf(stderr, "ERROR: realloc failed\n");
        }
        op->bits = new_bits;
    }
    return BV_set_by_index(op, op->bitsize++, bit_value);
}

/*
 * Adds significant_bits_count bits from byte array.
 * Don't uses BV_push_bit function in order to use more efficient
 * memory allocations.
 */
uint8_t BV_push_bytes(bitvector op, uint8_t * bytes, uint32_t bytes_size, uint32_t significant_bits_count) {
    if (significant_bits_count > bytes_size * BITS_PER_BYTE) {
        return BV_BUFFER_ERROR;
    }
    if ((op->bitsize > 0) && (((op->bitsize - 1) / BITS_PER_BYTE) < ((op->bitsize + significant_bits_count - 1) / BITS_PER_BYTE))) {
        uint32_t distance = ((op->bitsize + significant_bits_count - 1) / BITS_PER_BYTE) - ((op->bitsize - 1) / BITS_PER_BYTE);
        uint8_t * new_bits;
        while ((new_bits = realloc(op->bits, ((op->bitsize / (float) BITS_PER_BYTE) + distance) * sizeof(uint8_t))) == NULL) {
            fprintf(stderr, "ERROR: realloc failed\n");
        }
        op->bits = new_bits;
    }
    uint32_t last_bit = op->bitsize;
    op->bitsize += significant_bits_count;
    uint32_t changed_bits_count = 0;
    uint8_t bits_from_current_byte = 0;
    while (changed_bits_count < significant_bits_count) {
        uint8_t ret = BV_set_by_index(op, last_bit + changed_bits_count, ((*bytes) >> bits_from_current_byte) & 1);
        if (ret != BV_SUCCESS) {
            return ret;
        }
        bits_from_current_byte++;
        changed_bits_count++;
        if (bits_from_current_byte == 8) {
            bytes++;
            bits_from_current_byte = 0;
        }
    }
    return BV_SUCCESS;
}

/*
 * Frees specified bitvector.
 */
void BV_close(bitvector *op) {
    free((*op)->bits);
    free((*op));
    return;
}

#endif // BITVECTOR_H_INCLUDED
