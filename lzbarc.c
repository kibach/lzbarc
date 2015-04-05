#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>
#include "bitvector.h"

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

    bitvector testbv;

    switch (opindex) {
        case LZB_OPT_COMPRESS:
            //lzjb_compress();
            printf("ptr is %p\n", testbv);
            BV_init(&testbv);
            printf("ptr is %p now, bits at %p\n", testbv, testbv->bits);
            int bt = 1 << 5;
            printf("ret code is %d\n", BV_push_bytes(testbv, (uint8_t *) &bt, 4, 8));
            for (int i = 1; i <= 8; ++i) {
                printf("%dth bit is %d\n", i, BV_get_by_index(testbv, i));
            }
            BV_close(&testbv);
            printf("ptr is %p and freed\n", testbv);
            printf("lzjb compress from=%s to=%s\n", infile, outfile);
            break;
        case LZB_OPT_DECOMPRESS:
            //lzjb_decompress();
            printf("lzjb decompress from=%s to=%s\n", infile, outfile);
            break;
        default:
            printf("Unrecognized option\n");
            break;
    }

    return 0;
}

