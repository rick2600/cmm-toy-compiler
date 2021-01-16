#ifndef cmm_tokens_h
#define cmm_tokens_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_AND, TOKEN_OR,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Types.
    TOKEN_VOID, TOKEN_CHAR, TOKEN_INT,

    // Keywords.
    TOKEN_EXTERN,
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_WHILE, TOKEN_FOR,
    TOKEN_RETURN,

    TOKEN_ERROR,
    TOKEN_EOF
} token_type_t;

typedef struct {
    token_type_t type;
    const char* start;
    uint32_t length;
    uint32_t line;
} token_t;

typedef struct {
    uint32_t count;
    uint32_t capacity;
    token_t* tokens;
    bool error;
} token_stream_t;

#endif
