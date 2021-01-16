#ifndef cmm_scanner_h
#define cmm_scanner_h

#include <stdio.h>
#include <stdint.h>
#include "token.h"

typedef struct {
    const char* start;
    const char* current;
    uint32_t line;
} scanner_t;

token_stream_t* get_tokens(const char* source);
char *tokentype2str(token_type_t type);

#endif
