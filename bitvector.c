#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "bitvector.h"
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

/*
 * Initializes bitvector.
 */
uint8_t BV_init(bitvector *op, FILE * flush) {
    (*op) = malloc(sizeof(struct bitarray));
    if (unlikely(((*op)->bits = malloc(sizeof(uint8_t))) == NULL)) {
        return BV_MEMORY_ERROR;
    }
    (*op)->bits[0] = 0;
    (*op)->bitsize = 0;
    (*op)->allocated_bytes = 0;
    (*op)->fifo_start = 0;
    (*op)->flush_to = flush;
    return BV_SUCCESS;
}

/*
 * Gets bit value by its index in bitvector.
 * Uses easy-to-understand formula using bit shifts and conjunction.
 */
uint8_t BV_get_by_index(bitvector op, uint32_t index) {
    if (unlikely(index > op->bitsize)) {
        return BV_RANGE_ERROR;
    }
    return (op->bits[index / BITS_PER_BYTE] >> ((index % BITS_PER_BYTE) - 1)) & 1;
}

/*
 * Sets bit by its index to bit_value..
 * Used formula found somewhere on StackOverflow.
 */
uint8_t BV_set_by_index(bitvector op, uint32_t index, uint8_t bit_value) {
    if (unlikely(bit_value > 1)) {
        return BV_INVALID_BIT_VALUE_ERROR;
    }
    if (unlikely(index > op->bitsize)) {
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
    if (/*(op->bitsize > 0) && */(op->bitsize % BITS_PER_BYTE == 0)) {
        uint8_t * new_bits;
        if ((new_bits = realloc(op->bits, ((op->allocated_bytes++) + 1) * sizeof(uint8_t))) == NULL) {
            fprintf(stderr, "ERROR: realloc failed\n");
            return BV_MEMORY_ERROR;
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
    if (unlikely(significant_bits_count > bytes_size * BITS_PER_BYTE)) {
        return BV_BUFFER_ERROR;
    }
    if (ceil((op->bitsize + significant_bits_count) / (float) BITS_PER_BYTE) > op->allocated_bytes) {
        uint8_t * new_bits;
        if (unlikely((new_bits = realloc(op->bits, ceil((op->bitsize + significant_bits_count) / (float) BITS_PER_BYTE) * sizeof(uint8_t))) == NULL)) {
            fprintf(stderr, "ERROR: realloc failed\n");
            return BV_MEMORY_ERROR;
        }
        op->allocated_bytes = ceil((op->bitsize + significant_bits_count) / (float) BITS_PER_BYTE);
        op->bits = new_bits;
    }
    uint32_t last_bit = op->bitsize;
    op->bitsize += significant_bits_count;
    uint32_t changed_bits_count = 0;
    uint8_t bits_from_current_byte = 0;
    while (changed_bits_count < significant_bits_count) {
        uint8_t ret = BV_set_by_index(op, last_bit + changed_bits_count, ((*bytes) >> bits_from_current_byte) & 1);
        if (unlikely(ret != BV_SUCCESS)) {
            return ret;
        }
        bits_from_current_byte++;
        changed_bits_count++;
        if (unlikely(bits_from_current_byte == BITS_PER_BYTE)) {
            bytes++;
            bits_from_current_byte = 0;
        }
    }
    return BV_SUCCESS;
}

/*
 * Fills bitvector to fully fill last byte.
 * Used for exporting bitvector to byte array.
 */
uint8_t BV_fill(bitvector op) {
    uint8_t nil = 0;
    return BV_push_bytes(op, &nil, sizeof(uint8_t), (op->allocated_bytes * BITS_PER_BYTE - op->bitsize));
}

/*
 * Frees specified bitvector.
 */
void BV_close(bitvector *op) {
    free((*op)->bits);
    free((*op));
    return;
}

/*
 * Force flush bitvector to file.
 */
uint8_t BV_flush(bitvector op) {
    if (op->flush_to == 0) {
        return BV_FLUSH_ERROR;
    }
    fwrite(op->bits, sizeof(uint8_t), op->allocated_bytes, op->flush_to);
    return BV_SUCCESS;
}
