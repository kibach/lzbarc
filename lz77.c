#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bitvector.h"
#include "lz77.h"

uint8_t lz77_find_longest_match(uint8_t * la_buf, uint16_t * best_match_distance, uint8_t * best_match_length, uint16_t l, uint16_t r, uint16_t cur) {
    *best_match_distance = 0;
    *best_match_length = 0;
    uint8_t * subs_buf = malloc(sizeof(uint8_t));
    for (uint16_t j = cur + 2; j < r; ++j) {
        uint16_t subs_len = j - cur;
        subs_buf = realloc(subs_buf, subs_len);
        memcpy(subs_buf, la_buf + cur, j - cur);
        for (uint16_t i = l; i < cur; ++i) {
            //uint16_t repetitions = subs_len / (cur - i);
            //uint16_t last = subs_len % (cur - i);
            if (subs_len > *best_match_length) {
                uint8_t bcnt = 0;
                uint8_t passes = 1;
                while (bcnt < subs_len) {
                    if (subs_buf[bcnt] != la_buf[i + bcnt % (cur - i)]) {
                        passes = 0;
                        break;
                    }
                    ++bcnt;
                }
                if (passes) {
                    *best_match_distance = cur - i;
                    *best_match_length = subs_len;
                }
            }
        }
    }
    free(subs_buf);
    if ((*best_match_distance > 0) && (*best_match_length > 0)) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t lz77_helper_shift_array(uint8_t * arr, uint8_t * suffix, uint8_t arr_size, uint8_t suffix_len) {
    for (uint8_t i = suffix_len; i < arr_size; ++i) {
        arr[i - suffix_len] = arr[i];
    }
    for (uint8_t i = arr_size - suffix_len; i < arr_size; ++i) {
        arr[i] = suffix[i + suffix_len - arr_size];
    }
    return LZ77_SUCCESS;
}

uint8_t lz77_compress(FILE * infile, FILE * outfile, uint32_t insize, char * fname, uint8_t (* header_func) (FILE *, char *, uint32_t, uint32_t)) {
    uint8_t lookahead[LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1];
    uint8_t fbuf[LOOKAHEAD_BUFFER_SIZE + 1];
    uint32_t i = 0;
    uint16_t best_match_distance;
    uint16_t l = LEMPEL_WINDOW_SIZE;
    uint16_t r;
    if (insize < LOOKAHEAD_BUFFER_SIZE) {
        r = LEMPEL_WINDOW_SIZE + insize;
    } else {
        r = LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE;
    }
    uint16_t cur = LEMPEL_WINDOW_SIZE;
    uint8_t best_match_length;
    bitvector bits;
    if (BV_init(&bits, outfile, NULL, 0) == BV_MEMORY_ERROR) {
        fprintf(stderr, "ERROR: bitvector initialization failed\n");
        return LZ77_FAILED;
    }
    for (uint8_t i = 0; i < LEMPEL_WINDOW_SIZE; ++i) {
        lookahead[i] = 0;
    }
    fread(lookahead + LEMPEL_WINDOW_SIZE, sizeof(uint8_t), LOOKAHEAD_BUFFER_SIZE + 1, infile);
    while (i < insize) {
        if (lz77_find_longest_match(lookahead, &best_match_distance, &best_match_length, l, r, cur)) {
            if (BV_push_bit(bits, 1) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            if (BV_push_bytes(bits, (uint8_t *) &best_match_distance, sizeof(uint8_t) * 2, 12) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            if (BV_push_bytes(bits, (uint8_t *) &best_match_length, sizeof(uint8_t), 4) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            //printf("<1, %d, %d>\n", best_match_distance, best_match_length);
            if (!feof(infile)) {
                uint16_t read;
                memset(fbuf, 0, sizeof(uint8_t) * best_match_length);
                if ((read = fread(fbuf, sizeof(uint8_t), best_match_length, infile)) == 0) {
                    return LZ77_FAILED;
                }
                r -= best_match_length - read;
            } else {
                memset(fbuf, 0, sizeof(uint8_t) * best_match_length);
                r -= best_match_length;
            }
            lz77_helper_shift_array(lookahead, fbuf, LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1, best_match_length);
            i += best_match_length;
            if (l > 0) {
                if (best_match_length < l)
                    l -= best_match_length;
                else
                    l = 0;
            }
        } else {
            if (BV_push_bit(bits, 0) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            if (BV_push_bytes(bits, (uint8_t *)lookahead + LEMPEL_WINDOW_SIZE, sizeof(uint8_t), BITS_PER_BYTE) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            //printf("<0, 0x%02x>\n", lookahead[LEMPEL_WINDOW_SIZE]);
            if (!feof(infile)) {
                if (fread(fbuf, sizeof(uint8_t), 1, infile) == 0) {
                    return LZ77_FAILED;
                }
            } else {
                fbuf[0] = 0;
                --r;
            }
            lz77_helper_shift_array(lookahead, fbuf, LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1, 1);
            ++i;
            if (l > 0)
                --l;
        }
    }
    if (BV_fill(bits) != BV_SUCCESS) {
        return LZ77_FAILED;
    }
    if (header_func != NULL) {
        (*header_func)(outfile, fname, insize, bits->allocated_bytes);
    }
    if (BV_flush(bits) != BV_SUCCESS) {
        return LZ77_FAILED;
    }
    BV_close(&bits);
    return LZ77_SUCCESS;
}

uint8_t lz77_decompress(FILE * infile, FILE * outfile, uint32_t insize) {
    uint32_t decsize = 0;
    uint8_t lookahead[LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1];
    memset(lookahead, 0, sizeof(uint8_t) * (LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1));
    bitvector bits;
    if (BV_init(&bits, NULL, infile, insize) != BV_SUCCESS) {
        return LZ77_FAILED;
    }
    if (BV_load(bits) != BV_SUCCESS) {
        return LZ77_FAILED;
    }
    while (!BV_empty(bits)) {
        uint8_t flag = BV_pop_bit(bits);
        if (flag == BV_RANGE_ERROR) {
            return LZ77_FAILED;
        }
        if (!flag) {
            uint8_t byte = 0;
            if (BV_pop_bytes(bits, &byte, BITS_PER_BYTE) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            fwrite(&byte, sizeof(uint8_t), 1, outfile);
            lz77_helper_shift_array(lookahead, &byte, LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1, 1);
            //printf("<0, 0x%02x>\n", byte);
            ++decsize;
        } else {
            uint8_t match_length = 0;
            uint16_t match_distance = 0;
            if (BV_pop_bytes(bits, (uint8_t *) &match_distance, 12) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            if (BV_pop_bytes(bits, &match_length, 4) != BV_SUCCESS) {
                return LZ77_FAILED;
            }
            uint8_t * uncompr = malloc(sizeof(uint8_t) * match_length);
            for (uint8_t i = 0; i < match_length; ++i) {
                uncompr[i] = lookahead[LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE - match_distance + (i % match_distance) + 1];
            }
            lz77_helper_shift_array(lookahead, uncompr, LEMPEL_WINDOW_SIZE + LOOKAHEAD_BUFFER_SIZE + 1, match_length);
            fwrite(uncompr, sizeof(uint8_t), match_length, outfile);
            //printf("<1, %d, %d>\n", match_distance, match_length);
            free(uncompr);
            decsize += match_length;
        }
    }
    return LZ77_SUCCESS;
}
