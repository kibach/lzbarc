#ifndef LZ77_H_INCLUDED
#define LZ77_H_INCLUDED

enum {
    LZ77_SUCCESS = 110,
    LZ77_FAILED = 111
};

enum {
    LEMPEL_WINDOW_SIZE = 20,
    LOOKAHEAD_BUFFER_SIZE = 15,
    FILE_BLOCK_SIZE = 1024
};

uint8_t lz77_find_longest_match(uint8_t * la_buf, uint16_t * best_match_distance, uint8_t * best_match_length, uint16_t l, uint16_t r, uint16_t cur);
uint8_t lz77_helper_shift_array(uint8_t * arr, uint8_t * suffix, uint8_t arr_size, uint8_t suffix_len);
extern uint8_t lz77_compress(FILE * infile, FILE * outfile, uint32_t insize);
extern uint8_t lz77_decompress(FILE * infile, FILE * outfile, uint32_t insize);

#endif // LZ77_H_INCLUDED
