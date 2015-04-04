#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>

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
                    fprintf(stderr, "Unknown character `%s`\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    switch (opindex) {
        case LZB_OPT_COMPRESS:
            //lzjb_compress();
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

