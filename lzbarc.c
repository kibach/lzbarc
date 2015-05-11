#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>
#include "lz77.h"

enum {
    LZB_OPT_COMPRESS = 1,
    LZB_OPT_DECOMPRESS = 2
};

int main(int argc, char *argv[]) {
    uint8_t opindex = 0;
    char *infile = NULL;
    char *outfile = NULL;
    char c;

    opterr = 0;

    while ((c = getopt(argc, argv, "cdi:o:")) != -1) {
        switch (c) {
            case 'c':
                opindex = LZB_OPT_COMPRESS;
                break;
            case 'd':
                opindex = LZB_OPT_DECOMPRESS;
                break;
            case 'i':
                infile = optarg;
                break;
            case 'o':
                outfile = optarg;
                break;
            case '?':
                if ((optopt == 'i') || (optopt == 'o'))
                    fprintf(stderr, "Option -%c requires an argument\n", optopt);
                else if (isprint(c))
                    fprintf(stderr, "Unknown option `-%c`\n", optopt);
                else
                    fprintf(stderr, "Unknown character `%c`\n", optopt);
                return 1;
            default:
                abort();
        }
    }
    FILE * inp_f;
    FILE * out_f;
    uint32_t inp_sz;
    uint8_t lz_res;
    switch (opindex) {
        case LZB_OPT_COMPRESS:
            printf("lz77 compress from=%s to=%s\n", infile, outfile);
            inp_f = fopen(infile, "r");
            out_f = fopen(outfile, "w");
            fseek(inp_f, 0L, SEEK_END);
            inp_sz = ftell(inp_f);
            fseek(inp_f, 0L, SEEK_SET);
            lz_res = lz77_compress(inp_f, out_f, inp_sz);
            if (lz_res == LZ77_SUCCESS) {
                printf("lz77 compressed!\n");
            }
            fclose(inp_f);
            fclose(out_f);
            break;
        case LZB_OPT_DECOMPRESS:
            printf("lz77 decompress from=%s to=%s\n", infile, outfile);
            inp_f = fopen(infile, "r");
            out_f = fopen(outfile, "w");
            fseek(inp_f, 0L, SEEK_END);
            inp_sz = ftell(inp_f);
            fseek(inp_f, 0L, SEEK_SET);
            lz_res = lz77_decompress(inp_f, out_f, inp_sz);
            if (lz_res == LZ77_SUCCESS) {
                printf("lz77 decompressed!\n");
            }
            fclose(inp_f);
            fclose(out_f);
            break;
        default:
            printf("Unrecognized option\n");
            break;
    }

    return 0;
}

