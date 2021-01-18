#ifndef cmm_opt_parser_h
#define cmm_opt_parser_h

#include <stdbool.h>

typedef struct {
    bool tokens;
    bool ast;
    bool symbols;
    char* filename;
} opts_t;

opts_t* parse_opts(int argc, char** argv);

#endif
