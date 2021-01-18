#ifndef cmm_scanner_h
#define cmm_scanner_h

#include <stdio.h>
#include <stdint.h>
#include "token.h"

#define CAPACITY_STEP 32

typedef struct {
    const char* start;
    const char* current;
    uint32_t line;
} scanner_t;

token_stream_t* get_tokens(const char* source);
char *stringify_token_type(token_type_t type);
char* token_type_str(token_type_t type);
char* lexeme(token_t* token);

#endif
