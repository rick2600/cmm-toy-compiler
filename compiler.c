#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "scanner.h"
#include "parser.h"
#include "token.h"
#include "ast.h"
#include "ast_show.h"
#include "opt_parser.h"


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

static void debug_token(token_t* token) {
    printf("[%d: %-18s]  '%.*s'\n",
        token->line,
        stringify_token_type(token->type),
        token->length, token->start
    );
}

static void show_tokens(token_stream_t* token_stream) {
    for (int i = 0; i < token_stream->count; i++) {
        debug_token(&token_stream->tokens[i]);
    }
    printf("\n");
}

void compile(opts_t* opts) {

    if (opts->filename == NULL) {
        fprintf(stderr, "No source file passed!");
        exit(EXIT_FAILURE);
    }

    char* buffer = read_file(opts->filename);
    parser_t* parser = parse(buffer);

    if (opts->tokens && parser->token_stream != NULL)
        show_tokens(parser->token_stream);

    if (opts->symbols && parser->global_sym_table != NULL)
        show_sym_table(parser->global_sym_table);

    if (opts->ast) {
        if (parser->ast == NULL || parser->had_error) {
            printf("An error occured - AST not generated!\n");
        } else {
            show_ast(parser->ast);
        }
    }


    free(buffer);

}