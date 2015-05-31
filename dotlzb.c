#include "dotlzb.h"

const char * DOTLZB_SIGN = "LZB~";

uint8_t DOTLZB_create_file(FILE ** file, char * filename) {
    *file = fopen(filename, "w");
    if (*file == NULL) {
        perror("Archive creation error");
        return DOTLZB_FILE_ERROR;
    }
    fwrite(DOTLZB_SIGN, sizeof(char), strlen(DOTLZB_SIGN), *file);
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_open_file(FILE ** file, char * filename) {
    *file = fopen(filename, "r");
    if (*file == NULL) {
        perror("Archive opening error");
        return DOTLZB_FILE_ERROR;
    }
    char sigbuf[4];
    fread(sigbuf, sizeof(char), sizeof(sigbuf), *file);
    if (strcmp(sigbuf, DOTLZB_SIGN) != 0) {
        return DOTLZB_SIGN_INVALID;
    }
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_enum_file(FILE * file, dotlzb_file_header file_header) {
    size_t readen = fread(file_header, sizeof(struct dotlzb_file_header), 1, file);
    if (readen != 1) {
        return DOTLZB_LESS_THAN_EXPECTED;
    }
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_write_dir_header(FILE * file, char * dir_name, uint16_t file_count) {
    dotlzb_file_header dir_header = malloc(sizeof(struct dotlzb_file_header));
    dir_header->file_count = file_count;
    dir_header->file_compressed_size = 0;
    dir_header->file_name_size = strlen(dir_name) + 1;
    dir_header->file_original_size = 0;
    if (fwrite(dir_header, sizeof(struct dotlzb_file_header), 1, file) != 1) {
        free(dir_header);
        return DOTLZB_LESS_THAN_EXPECTED;
    }
    free(dir_header);
    if (fwrite(dir_name, sizeof(char), strlen(dir_name) + 1, file) != strlen(dir_name) + 1) {
        return DOTLZB_LESS_THAN_EXPECTED;
    }
    char * ns = "";
    fwrite(ns, sizeof(uint8_t), 1, file);
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_write_file_header(FILE * file, char * file_name, uint32_t original_size, uint32_t compressed_size) {
    dotlzb_file_header file_header = malloc(sizeof(struct dotlzb_file_header));
    file_header->file_count = 0;
    file_header->file_compressed_size = compressed_size;
    file_header->file_name_size = strlen(file_name) + 1;
    file_header->file_original_size = original_size;
    size_t written = fwrite(file_header, sizeof(struct dotlzb_file_header), 1, file);
    if (written != 1) {
        free(file_header);
        return DOTLZB_LESS_THAN_EXPECTED;
    }
    free(file_header);
    if (fwrite(file_name, sizeof(char), strlen(file_name) + 1, file) != strlen(file_name) + 1) {
        return DOTLZB_LESS_THAN_EXPECTED;
    }
    //char * ns = "";
    //fwrite(ns, sizeof(uint8_t), 1, file);
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_compress_files(FILE * file, char * dir_name, char ** file_list, uint16_t file_count) {
    for (uint16_t i = 0; i < file_count; ++i) {
        char * path = malloc(5 + strlen(file_list[i]) + strlen(dir_name)); //To be enough for <dir_name><file_list[i]>/\0
        uint8_t is_dir = 1;
        strcpy(path, dir_name);
        strcat(path, file_list[i]);
        strcat(path, "/");
        if (0 != access(path, F_OK)) {
            if (ENOENT == errno) {
                printf("File opening error: No such file or directory\n");
                return DOTLZB_NOT_EXISTS;
            }
            if (ENOTDIR == errno) {
                is_dir = 0;
            }
        }
        if (is_dir) {
            char ** subdir_list = malloc(sizeof(char *));
            uint16_t subdir_list_size = 0;
            DIR * d;
            struct dirent * entry;
            char * tmp;
            d = opendir(path);
            if (d == NULL) {
                return DOTLZB_NOT_EXISTS;
            }
            while ((entry = readdir(d)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;
                if ((entry->d_type == DT_REG) || (entry->d_type == DT_DIR)) {
                    subdir_list = realloc(subdir_list, sizeof(char *) * ++subdir_list_size);
                    tmp = malloc(sizeof(char) * (strlen(entry->d_name) + 1));
                    strcpy(tmp, entry->d_name);
                    subdir_list[subdir_list_size - 1] = tmp;
                }
            }
            closedir(d);
            DOTLZB_write_dir_header(file, file_list[i], subdir_list_size);
            printf("adding directory: %s\n", path);
            uint8_t retcode = DOTLZB_compress_files(file, path, subdir_list, subdir_list_size);
            if (retcode != DOTLZB_SUCCESS) {
                return retcode;
            }
            for (uint16_t j = 0; j < subdir_list_size; ++j) {
                free(subdir_list[j]);
            }
            free(subdir_list);
        } else {
            strcpy(path, dir_name);
            strcat(path, file_list[i]);
            FILE * inp_f = fopen(path, "r");
            fseek(inp_f, 0L, SEEK_END);
            uint32_t inp_sz = ftell(inp_f);
            fseek(inp_f, 0L, SEEK_SET);
            printf("adding file: %s\n", path);
            lz77_compress(inp_f, file, inp_sz, file_list[i], *DOTLZB_write_file_header);
            fclose(inp_f);
        }
        free(path);
    }
    return DOTLZB_SUCCESS;
}

uint8_t DOTLZB_decompress_file(FILE * file, char * dirname, uint16_t file_count) {
    uint16_t file_processed;
    while ((file_count == 0 && !feof(file)) || (file_processed < file_count)) {
        dotlzb_file_header file_header = calloc(1, sizeof(struct dotlzb_file_header));
        DOTLZB_enum_file(file, file_header);
        char * fname = malloc(file_header->file_name_size + 1);
        fread(fname, sizeof(char), file_header->file_name_size + 1, file);
        char * path = malloc(strlen(dirname) + strlen(fname) + 2);
        strcpy(path, dirname);
        strcat(path, fname);
        if (file_header->file_name_size == 0 || strcmp(fname, "") == 0) { //to avoid empty files
            break;
        }
        if (file_header->file_count > 0) {
            //its a directory header
            mkdir(path, 0777);
            strcat(path, "/");
            printf("extracting directory: %s\n", path);
            DOTLZB_decompress_file(file, path, file_header->file_count);
        } else {
            FILE * out_f = fopen(path, "w");
            printf("extracting file: %s\n", path);
            if (out_f != NULL) {
                lz77_decompress(file, out_f, file_header->file_compressed_size);
                fclose(out_f);
            } else {
                perror("Error");
            }
        }
        file_processed++;
    }
    return DOTLZB_SUCCESS;
}
