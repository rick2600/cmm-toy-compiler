#ifndef cmm_parser_h
#define cmm_parser_h

#include "ast.h"
#include "sym_table.h"


typedef struct {
    uint32_t cur_position;
    token_stream_t* token_stream;
    bool panic_mode;
    bool had_error;
    sym_table_t* global_sym_table;
    sym_table_t* cur_sym_table;
} parser_t;

ast_node_t* parse(char *buffer);

#endif