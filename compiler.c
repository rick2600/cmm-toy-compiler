#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "scanner.h"
#include "parser.h"
#include "ast.h"
#include "ast_visitor.h"

static size_t get_file_size(FILE* fp) {
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    return size;
}

static char* read_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }
    size_t file_size = get_file_size(fp);

    char* buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, fp);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0';
    fclose(fp);
    return buffer;
}

void compile(const char* filename) {
    char* buffer = read_file(filename);
    ast_node_t* ast = parse(buffer);
    free(buffer);
    //if (ast != NULL) visit_ast(ast);
}