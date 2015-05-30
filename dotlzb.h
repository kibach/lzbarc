#ifndef DOTLZB_H_INCLUDED
#define DOTLZB_H_INCLUDED
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "lz77.h"

struct dotlzb_file_header {
    uint16_t file_name_size;
    uint16_t file_count;
    uint32_t file_compressed_size;
    uint32_t file_original_size;
};

enum {
    DOTLZB_SUCCESS = 120,
    DOTLZB_FILE_ERROR = 121,
    DOTLZB_SIGN_INVALID = 122,
    DOTLZB_LESS_THAN_EXPECTED = 123,
    DOTLZB_NOT_EXISTS = 124
};

typedef struct dotlzb_file_header * dotlzb_file_header;

uint8_t DOTLZB_create_file(FILE ** file, char * filename);
uint8_t DOTLZB_open_file(FILE ** file, char * filename);
uint8_t DOTLZB_compress_files(FILE * file, char * dir_name, char ** file_list, uint16_t file_count);
uint8_t DOTLZB_decompress_file(FILE * file, char * dirname, uint16_t file_count);

#endif // DOTLZB_H_INCLUDED
