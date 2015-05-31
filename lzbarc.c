#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>
#include "dotlzb.h"
#include "lz77.h"

enum {
    LZB_OPT_COMPRESS = 1,
    LZB_OPT_DECOMPRESS = 2,
    LZB_OPT_HELP = 3
};

int main(int argc, char *argv[]) {
    uint8_t opindex = 0;
    char *infile = NULL;
    char *outfile = NULL;
    char ** file_list;
    uint16_t file_list_size = 0;
    char c;

    opterr = 0;
    while ((c = getopt(argc, argv, "cdhi:o:")) != -1) {
        switch (c) {
            case 'c':
                opindex = LZB_OPT_COMPRESS;
                break;
            case 'd':
                opindex = LZB_OPT_DECOMPRESS;
                break;
            case 'h':
                opindex = LZB_OPT_HELP;
                break;
            case 'i':
                file_list = malloc(sizeof(char *));
                char * tmp;
                optind--;
                for(; optind < argc && *argv[optind] != '-'; optind++){
                    file_list = realloc(file_list, sizeof(char *) * ++file_list_size);
                    tmp = malloc(sizeof(char) * (strlen(argv[optind]) + 1));
                    strcpy(tmp, argv[optind]);
                    file_list[file_list_size - 1] = tmp;
                }
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
                break;
            default:
                opindex = LZB_OPT_HELP;
                break;
        }
    }
    FILE * inp_f;
    FILE * out_f;
    switch (opindex) {
        case LZB_OPT_COMPRESS:
            printf("lzbarc compress to %s\n", outfile);
            if (DOTLZB_create_file(&out_f, outfile) == DOTLZB_SUCCESS) {
                DOTLZB_compress_files(out_f, "./", file_list, file_list_size);
                fclose(out_f);
            }
            for (uint16_t j = 0; j < file_list_size; ++j) {
                free(file_list[j]);
            }
            free(file_list);
            break;
        case LZB_OPT_DECOMPRESS:
            infile = *file_list;
            printf("lzbarc decompress from %s\n", infile);
            if (DOTLZB_open_file(&inp_f, infile) == DOTLZB_SUCCESS) {
                DOTLZB_decompress_file(inp_f, "./", 0);
                fclose(inp_f);
            }
            break;
        case LZB_OPT_HELP:
            printf("lzbarc 0.1\n");
            printf("Options:\n");
            printf("  -c    Compress files\n");
            printf("        Arguments:\n");
            printf("        -i <input files>  Input files or file mask\n");
            printf("        -o <filename>     Output archive name\n");
            printf("  -d    Decompress archive\n");
            printf("        Arguments:\n");
            printf("        -i <filename>     Input archive name\n");
            printf("  -h    Show this help\n");
            break;
        default:
            printf("Unrecognized option\n");
            break;
    }

    return 0;
}

