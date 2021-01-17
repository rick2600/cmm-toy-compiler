#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "scanner.h"

#define CAPACITY_STEP 64

scanner_t scanner;
token_stream_t token_stream;


static void ensure_token_stream_capacity() {
    if ((token_stream.count + 1) >= token_stream.capacity) {
        size_t new_capacity =  token_stream.capacity + CAPACITY_STEP;

        token_stream.tokens = realloc(token_stream.tokens,
                                      new_capacity * sizeof(token_t));

        if (token_stream.tokens == NULL) {
            fprintf(stderr, "Could not allocate memory for token_stream\n");
            exit(EXIT_FAILURE);
        }

        token_stream.capacity = new_capacity;
    }
}

static token_t* error_token(const char* message) {
    ensure_token_stream_capacity();
    token_t *token = &token_stream.tokens[token_stream.count];
    token->type = TOKEN_ERROR;
    token->start = message;
    token->length = (uint32_t)strlen(message);
    token->line = scanner.line;
    fprintf(stderr, "Line: %d: %s\n", token->line, message);
    token_stream.error = true;
    return token;
}

static token_t* make_token(token_type_t type) {
    ensure_token_stream_capacity();
    token_t *token = &token_stream.tokens[token_stream.count];
    token->type = type;
    token->start = scanner.start;
    token->length = (uint32_t)(scanner.current - scanner.start);
    token->line = scanner.line;
    token_stream.count++;
    return token;
}

static bool is_end() {
    return *scanner.current == '\0';
}

static bool match(char expected) {
    if (is_end()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peek_next() {
    if (is_end()) return '\0';
    return scanner.current[1];
}

static void skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            case '\n':
                scanner.line++;
                advance();
                break;

            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_end()) advance();
                } else {
                    return;
                }
                break;

            default:
            return;
        }
    }
}

static void init_scanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static void init_token_stream() {
    token_stream.count = 0;
    token_stream.capacity = 0;
    token_stream.tokens = NULL;
    token_stream.error = false;
}

static token_t* character() {
    if (peek() == '\\') {
        advance();
    }
    advance();

    if (peek() == '\'') {
        advance();
        token_t* token = make_token(TOKEN_CHAR);
        token->start++;
        token->length -= 2;
        return token;
    }

    return error_token("Unclosed char.");
}

static token_t* string() {
    while (peek() != '"' && !is_end()) {
        if (peek() == '\n')
            scanner.line++;
        advance();
    }
    if (is_end())
        return error_token("Unterminated string.");

    // The closing quote.
    advance();
    token_t* token = make_token(TOKEN_STRING);
    token->start++;
    token->length -= 2;

    return token;
}

static token_t* number() {
    while (isdigit(peek())) advance();
    return make_token(TOKEN_NUMBER);
}

static token_type_t check_keyword(int start, int length,
    const char* rest, token_type_t type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

  return TOKEN_IDENTIFIER;
}

static token_type_t identifier_type() {
    switch (scanner.start[0]) {
        case 'c': return check_keyword(1, 3, "har", TOKEN_CHAR);
        case 'e':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'l': return check_keyword(2, 2, "se", TOKEN_ELSE);
                    case 'x': return check_keyword(2, 4, "tern", TOKEN_EXTERN);
                }
            }
            break;
        case 'f': return check_keyword(1, 2, "or", TOKEN_FOR);
        case 'i':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'f': return check_keyword(2, 0, "", TOKEN_IF);
                    case 'n': return check_keyword(2, 1, "t", TOKEN_INT);
                }
            }
            break;

        case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 'v': return check_keyword(1, 3, "oid", TOKEN_VOID);
        case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static token_t* identifier() {
    while (isalpha(peek()) || isdigit(peek())) advance();
    return make_token(identifier_type());
}

static token_t* scan_token() {
    skip_whitespace();
    scanner.start = scanner.current;

    if (is_end()) return make_token(TOKEN_EOF);
    char c = advance();

    if (isalpha(c)) return identifier();
    if (isdigit(c)) return number();

   //printf("[%c]\n", c);
   char* err_msg = "Unexpected character.";

    switch (c) {
        case '(': return make_token(TOKEN_LEFT_PAREN);
        case ')': return make_token(TOKEN_RIGHT_PAREN);
        case '{': return make_token(TOKEN_LEFT_BRACE);
        case '}': return make_token(TOKEN_RIGHT_BRACE);
        case '[': return make_token(TOKEN_LEFT_BRACKET);
        case ']': return make_token(TOKEN_RIGHT_BRACKET);
        case ';': return make_token(TOKEN_SEMICOLON);
        case ',': return make_token(TOKEN_COMMA);
        case '-': return make_token(TOKEN_MINUS);
        case '+': return make_token(TOKEN_PLUS);
        case '/': return make_token(TOKEN_SLASH);
        case '*': return make_token(TOKEN_STAR);
        case '!': return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '&': return  match('&') ? make_token(TOKEN_AND) : error_token(err_msg);
        case '|': return  match('|') ? make_token(TOKEN_OR) : error_token(err_msg);
        case '"': return string();
        case '\'': return character();
    }

    return error_token(err_msg);
}

token_stream_t* get_tokens(const char* source) {
    init_scanner(source);
    init_token_stream();
    token_t* token;

    for (;;) {
        token = scan_token();
        if (token_stream.count > 0) {
            if (token->type == TOKEN_EOF)
                break;
        }
    }

    return &token_stream;
}

char *tokentype2str(token_type_t type) {
    switch(type) {
        case TOKEN_LEFT_PAREN: return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "TOKEN_RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "TOKEN_RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "TOKEN_LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "TOKEN_RIGHT_BRACKET";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_SLASH: return "TOKEN_SLASH";
        case TOKEN_STAR: return "TOKEN_STAR";
        case TOKEN_BANG: return "TOKEN_BANG";
        case TOKEN_BANG_EQUAL: return "TOKEN_BANG_EQUAL";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL: return "TOKEN_EQUAL_EQUAL";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL: return "TOKEN_LESS_EQUAL";
        case TOKEN_AND: return "TOKEN_AND";
        case TOKEN_OR: return "TOKEN_OR";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_VOID: return "TOKEN_VOID";
        case TOKEN_CHAR: return "TOKEN_CHAR";
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_EXTERN: return "TOKEN_EXTERN";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_FOR: return "TOKEN_FOR";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_EOF: return "TOKEN_EOF";
        default: return "unkown";
    }
}
