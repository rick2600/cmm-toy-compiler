#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "scanner.h"
#include "ast.h"
#include "sym_table.h"

extern token_stream_t token_stream;
uint32_t cur_token;
bool parser_error;
bool panic_mode;
bool sym_error;
sym_table_t* global_scope;
sym_table_t* current_scope;

static ast_node_t* parse_expr();

static void debug_token(token_t* token) {
    char type[20];
    char *s;

    s = tokentype2str(token->type);
    memset(type, ' ', sizeof(type)-1);
    memcpy(type, s, strlen(s));
    type[sizeof(type)-1] = '\0';

    printf("[%s]  '%.*s'\n", type, token->length, token->start);
}

static void show_tokens() {
    int current = 0;

    for (;;) {
        if (token_stream.tokens[current].type == TOKEN_EOF)
            break;

        debug_token(&token_stream.tokens[current]);
        current++;
    }
    printf("\n\n");
}

static void advance() {
    if (cur_token < token_stream.count)
        cur_token++;
}

token_t* next_token() {
    return &token_stream.tokens[cur_token];
}

static void error_at(token_t* token, const char* msg) {
    if (panic_mode)
        return;

    panic_mode = true;
    parser_error = true;

    fprintf(stderr, "Line: %d: error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
    // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", msg);
}

static token_t* peek(uint32_t lookahead) {
    if ((cur_token + lookahead) >= token_stream.count)
        return &token_stream.tokens[token_stream.count - 1];
    else
        return &token_stream.tokens[cur_token + lookahead];
}

static token_t* consume() {
    token_t* token = next_token();
    advance();
    return token;
}

static token_t* expect(token_type_t type, const char* msg) {
    if (peek(0)->type == type) {
        return consume();
    }
    error_at(next_token(), msg);
    return NULL;
}

bool match_any(int n, ...) {
    bool status = false;
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        if (peek(0)->type == va_arg(args, int)) {
            status = true;
            break;
        }
    }
    va_end(args);
    return status;
}

static bool match(token_type_t type) {
    return peek(0)->type == type;
}

static void synchronize() {
    panic_mode = false;

    while (!match(TOKEN_EOF)) {
        if (match_any(5, TOKEN_FOR, TOKEN_IF,
                      TOKEN_WHILE, TOKEN_RETURN, TOKEN_SEMICOLON)) {
            return;
        }
        consume();
    }
}

static void synchronize_decl() {
    panic_mode = false;

    while (!match(TOKEN_EOF)) {
        if (match_any(3, TOKEN_INT, TOKEN_CHAR, TOKEN_VOID)) {
            return;
        }
        consume();
    }
}

static char* lexeme(token_t* token) {
    char *s = malloc(token->length+1);
    if (s == NULL) {
        fprintf(stderr, "Could not allocate memory for lexeme\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s, token->start, token->length);
    s[token->length] = '\0';
    return s;
}

static decl_type_t tokentype_2_decltype(token_type_t type) {;
    decl_type_t decl_type = TYPE_VOID;

    if (type == TOKEN_INT) decl_type = TYPE_INT;
    else if (type == TOKEN_CHAR) decl_type = TYPE_CHAR;

    return decl_type;
}

static ast_node_t* parse_factor() {
    ast_node_t* node = NULL;

    if (match(TOKEN_NUMBER)) {
        token_t* token = consume();
        node = create_ast_node_number(token);

    } else if (match(TOKEN_IDENTIFIER)) {
        token_t* token = consume();
        ast_node_t* ident = create_ast_node_ident(token);

        if (sym_lookup(current_scope, ident->as.ident.value) == NULL
            && sym_lookup(global_scope, ident->as.ident.value) == NULL ) {
            //fprintf(stderr, "'%s' undeclared\n", node_ident->as.ident.value);
            error_at(token, "undeclared");
            sym_error = true;
        }

        if (match(TOKEN_LEFT_PAREN)) {
            consume();
            node = create_ast_node_funccall(ident);

            if (!match(TOKEN_RIGHT_PAREN)) {
                add_param(node->as.funccall.params, parse_expr());
                while (match(TOKEN_COMMA)) {
                    consume();
                    add_param(node->as.funccall.params, parse_expr());
                }
            }
            expect(TOKEN_RIGHT_PAREN, "expected ')'");
        } else if (match(TOKEN_LEFT_BRACKET)) {
            consume();
            node = create_ast_node_arrayaccess(ident, parse_expr());
            expect(TOKEN_RIGHT_BRACKET, "expected ']'");
        } else {
            return ident;
        }

    } else if (match(TOKEN_STRING)) {
        token_t* token = consume();
        node = create_ast_node_string(token);
    } else if (match(TOKEN_CHAR)) {
        token_t* token = consume();
        node = create_ast_node_char(token);
    } else if (match(TOKEN_LEFT_PAREN)) {
        consume();
        node = parse_expr();
        expect(TOKEN_RIGHT_PAREN, "expected ')'");
    } else if (match(TOKEN_BANG)) {
        token_t* token = consume();
        node = create_ast_node_unary(token, parse_factor());
    }
    return node;
}

static ast_node_t* parse_term() {
    ast_node_t* node = parse_factor();

    while (match_any(3, TOKEN_STAR, TOKEN_SLASH, TOKEN_AND)) {
        token_t* token = consume();
        node = create_ast_node_binary(token, node, parse_factor());
    }
    return node;
}

static ast_node_t* parse_expr_simp() {
    ast_node_t* node;

    if (match(TOKEN_PLUS)) {
        consume();
    } else if  (match(TOKEN_MINUS)) {
        token_t* token = consume();
        node = create_ast_node_unary(token, parse_term());
    } else {
        node = parse_term();
    }

    while (match_any(3, TOKEN_PLUS, TOKEN_MINUS, TOKEN_OR)) {
        token_t* token = consume();
        node = create_ast_node_binary(token, node, parse_term());
    }

    return node;
}

static ast_node_t* parse_expr() {
    ast_node_t* node = parse_expr_simp();

    while (match_any(3, TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL, TOKEN_LESS_EQUAL)
          || match_any(3, TOKEN_LESS, TOKEN_GREATER_EQUAL, TOKEN_GREATER)) {

        token_t* token = consume();
        node = create_ast_node_binary(token, node, parse_expr_simp());
    }
    return node;
}

static ast_node_t* parse_assign() {
    token_t* token = consume();
    ast_node_t* node_ident = create_ast_node_ident(token);
    ast_node_t* node_assign = NULL;


    if (match(TOKEN_LEFT_BRACKET)) {
        consume();
        ast_node_t* arrayaccess = create_ast_node_arrayaccess(node_ident, parse_expr());
        expect(TOKEN_RIGHT_BRACKET, "expected ']'");
        expect(TOKEN_EQUAL, "expected '='");
        node_assign = create_ast_node_assign(arrayaccess, parse_expr());
    } else if (match(TOKEN_EQUAL)) {
        consume();
        node_assign = create_ast_node_assign(node_ident, parse_expr());
    }
    if (sym_lookup(current_scope, node_ident->as.ident.value) == NULL
        && sym_lookup(global_scope, node_ident->as.ident.value) == NULL ) {
        //fprintf(stderr, "'%s' undeclared\n", node_ident->as.ident.value);
        error_at(token, "undeclared");
        sym_error = true;
    }
    return node_assign;
}

static void parse_stmt(ast_node_t* parent) {

    if (panic_mode)
        synchronize();

    ast_node_t* node = NULL;

    if (match(TOKEN_IF)) {
        token_t* token_if = consume();
        expect(TOKEN_LEFT_PAREN, "expected '('");
        node = create_ast_node_if(token_if, parse_expr());

        expect(TOKEN_RIGHT_PAREN, "expected ')'");
        parse_stmt(node->as.ifstmt._if);
        if (match(TOKEN_ELSE)) {
            consume();
            parse_stmt(node->as.ifstmt._else);
        }
        add_stmt(parent, node);
    } else if (match(TOKEN_WHILE)) {
        token_t* token_while = consume();
        expect(TOKEN_LEFT_PAREN, "expected '('");
        node = create_ast_node_while(token_while, parse_expr());
        expect(TOKEN_RIGHT_PAREN, "expected ')'");
        parse_stmt(node->as.whilestmt.stmts);
        add_stmt(parent, node);
    } else if (match(TOKEN_FOR)) {
        ast_node_t* init = NULL;
        ast_node_t* cond = NULL;
        ast_node_t* incr = NULL;

        token_t* token_for = consume();
        expect(TOKEN_LEFT_PAREN, "expected '('");

        if (!match(TOKEN_SEMICOLON)) init = parse_assign();
        expect(TOKEN_SEMICOLON, "expected ';'");

        if (!match(TOKEN_SEMICOLON)) cond = parse_expr();
        expect(TOKEN_SEMICOLON, "expected ';'");

        if (!match(TOKEN_RIGHT_PAREN)) incr = parse_assign();
        expect(TOKEN_RIGHT_PAREN, "expected ')'");

        node = create_ast_node_for(token_for, init, cond, incr);
        parse_stmt(node->as.forstmt.stmts);
        add_stmt(parent, node);
    } else if (match(TOKEN_RETURN)) {
        token_t* token_return = consume();
        if (match(TOKEN_SEMICOLON)) {
            consume();
            node = create_ast_node_return(token_return, NULL);
            add_stmt(parent, node);
        } else {
            node = create_ast_node_return(token_return, parse_expr());
            expect(TOKEN_SEMICOLON, "expected ';'");
            add_stmt(parent, node);
        }
    } else if (match(TOKEN_LEFT_BRACE)) {
        consume();
        while (match_any(4, TOKEN_IF, TOKEN_WHILE, TOKEN_FOR, TOKEN_RETURN)
               || match_any(3, TOKEN_IDENTIFIER, TOKEN_LEFT_BRACE, TOKEN_SEMICOLON)) {
            parse_stmt(parent);
        }
        expect(TOKEN_RIGHT_BRACE, "expected '}'");
    } else if (match(TOKEN_IDENTIFIER)) {
        if (peek(1)->type == TOKEN_LEFT_PAREN) {
            token_t* token_ident = consume();
            ast_node_t* ident = create_ast_node_ident(token_ident);
            consume(); // -> (
            node = create_ast_node_funccall(ident);

            if (sym_lookup(current_scope, ident->as.ident.value) == NULL
                && sym_lookup(global_scope, ident->as.ident.value) == NULL ) {
                error_at(token_ident, "undeclared");
                sym_error = true;
            }

            if (!match(TOKEN_RIGHT_PAREN)) {
                add_stmt(node->as.funccall.params, parse_expr());
                while (match(TOKEN_COMMA)) {
                    consume();
                    add_stmt(node->as.funccall.params, parse_expr());
                }
            }
            expect(TOKEN_RIGHT_PAREN, "expected ')'");
            expect(TOKEN_SEMICOLON, "expected ';'");
            add_stmt(parent, node);
        } else {
            add_stmt(parent, parse_assign());
            expect(TOKEN_SEMICOLON, "expected ';'");
        }
    } else if (match(TOKEN_SEMICOLON)) {
        consume();
    }
    else {
        error_at(peek(0), "unexpected token");
    }
}

static ast_node_t* parse_param_type() {
    ast_node_t* node = NULL;
    token_t* token_type;
    token_t* token_ident;
    bool is_array = false;

    if (match_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_type = consume();
        if (match(TOKEN_IDENTIFIER)) {
            token_ident = consume();
            ast_node_t* ident = create_ast_node_ident(token_ident);
            decl_type_t argtype = tokentype_2_decltype(token_type->type);
            if (match(TOKEN_LEFT_BRACKET)) {
                consume();
                is_array = true;
                expect(TOKEN_RIGHT_BRACKET, "expected ']'");
            }
            node = create_ast_node_paramdecl(argtype, ident, is_array);
        }
    } else if (match(TOKEN_VOID)) {
        consume();
    }
    return node;
}

static ast_node_t* parse_param_types() {
    ast_node_t* node = create_ast_node_paramdecl_list();
    ast_node_t* param = parse_param_type();
    if (param != NULL) {
        add_paramdecl(node, param);
        while (match(TOKEN_COMMA)) {
            consume();
            add_paramdecl(node, parse_param_type());
        }
    }
    return node;
}

/*
static ast_node_t* parse_func() {
    ast_node_t* node;

    if (match_any(3, TOKEN_VOID, TOKEN_INT, TOKEN_CHAR)) {
        token_t* token_type = consume();
        if (match(TOKEN_IDENTIFIER)) {
            token_t* token_ident = consume();
            expect(TOKEN_LEFT_PAREN, "expected '('");
            token_type_t type = tokentype_2_decltype(token_type->type);
            ast_node_t* ident = create_ast_node_ident(token_ident);
            node = create_ast_node_funcdecl(type, ident);
            node->as.funcdecl.params = parse_param_types();
            expect(TOKEN_RIGHT_PAREN, "expected ')'");
            expect(TOKEN_LEFT_BRACE, "expected '{'");

            while (!match(TOKEN_RIGHT_BRACE)) {
                parse_stmt(node->as.funcdecl.stmts);
            }

            expect(TOKEN_RIGHT_BRACE, "expected '}'");

        }
    }

    return node;
}
*/

static void parse_vardecls(ast_node_t* parent,
                           token_t* token_type, token_t* token_ident) {

    token_type_t type = tokentype_2_decltype(token_type->type);
    ast_node_t* ident = create_ast_node_ident(token_ident);
    int array_size = 0;

    bool is_array = false;

    if (match(TOKEN_LEFT_BRACKET)) {
        consume();
        is_array = true;
        if (match(TOKEN_NUMBER)) {
            char* s = lexeme(consume());
            array_size = atoi(s);
            free(s);
        }

        expect(TOKEN_RIGHT_BRACKET, "expected ']'");
    }

    ast_node_t* node = create_ast_node_vardecl(type, ident, is_array, array_size);
    add_stmt(parent, node);
    insert_sym_from_vardecl_node(global_scope, node);
}

static void parse_vardecls_for_func(ast_node_t* func_node, ast_node_t* parent) {
    token_type_t type;
    ast_node_t* ident;
    bool is_array = false;
    int array_size = 0;

    sym_entry_t* sym_func = sym_lookup(global_scope,
                                    func_node->as.funcdecl.ident->as.ident.value);

    if (match_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_t* token_type = consume();
        type = tokentype_2_decltype(token_type->type);
        if (match(TOKEN_IDENTIFIER)) {
            ident = create_ast_node_ident(consume());
            if (match(TOKEN_LEFT_BRACKET)) {
                consume();
                is_array = true;
                if (match(TOKEN_NUMBER)) {
                    char* s = lexeme(consume());
                    array_size = atoi(s);
                    free(s);
                }
                expect(TOKEN_RIGHT_BRACKET, "expected ']'");
            }
            ast_node_t* node = create_ast_node_vardecl(type, ident, is_array, array_size);
            add_stmt(parent, node);
            insert_sym_from_vardecl_node(sym_func->as.func.sym_table, node);

            while (match(TOKEN_COMMA)) {
                consume();
                if (match(TOKEN_IDENTIFIER)) {
                    ident = create_ast_node_ident(consume());
                    if (match(TOKEN_LEFT_BRACKET)) {
                        consume();
                        is_array = true;
                        if (match(TOKEN_NUMBER)) {
                            char* s = lexeme(consume());
                            array_size = atoi(s);
                            free(s);
                        }


                        expect(TOKEN_RIGHT_BRACKET, "expected ']'");
                    }
                    ast_node_t* node = create_ast_node_vardecl(type, ident, is_array, array_size);
                    add_stmt(parent, node);
                    insert_sym_from_vardecl_node(sym_func->as.func.sym_table, node);
                }
            }
        } else {
            error_at(consume(), "expected an identifier");
        }
        expect(TOKEN_SEMICOLON, "expected ';'");
    } else {
        error_at(consume(), "expected 'int' or 'char'");
    }
}

static void parse_funcdecl(ast_node_t* parent,
                           token_t* token_type, token_t* token_ident) {
    consume(); // '('
    token_type_t type = tokentype_2_decltype(token_type->type);
    ast_node_t* ident = create_ast_node_ident(token_ident);
    ast_node_t* params = parse_param_types();
    ast_node_t* node = create_ast_node_funcdecl(type, ident);
    node->as.funcdecl.params = params;

    expect(TOKEN_RIGHT_PAREN, "expected ')'");
    add_stmt(parent, node);

    if (match(TOKEN_COMMA)) {
        while (match(TOKEN_COMMA)) {
            consume();
            if (match(TOKEN_IDENTIFIER)) {
                token_t* token_ident2 = consume();
                ident = create_ast_node_ident(token_ident2);
                expect(TOKEN_LEFT_PAREN, "expected '('");
                params = parse_param_types();
                expect(TOKEN_RIGHT_PAREN, "expected ')'");
                node = create_ast_node_funcdecl(type, ident);
                node->as.funcdecl.params = params;
                add_stmt(parent, node);
                if (insert_sym_from_funcdecl_node(global_scope, node, true) == false)
                    sym_error = true;
            }
        }
        expect(TOKEN_SEMICOLON, "expected ';'");
    } else if (match(TOKEN_LEFT_BRACE)) {
            consume();

            if (insert_sym_from_funcdecl_node(global_scope, node, false) == false)
                sym_error = true;

            sym_entry_t* entry;
            entry = sym_lookup(global_scope, node->as.funcdecl.ident->as.ident.value);
            current_scope = entry->as.func.sym_table;

            while(match_any(2, TOKEN_INT, TOKEN_CHAR)) {
                parse_vardecls_for_func(node, node->as.funcdecl.stmts);
            }
            while (!match(TOKEN_RIGHT_BRACE)) {
                parse_stmt(node->as.funcdecl.stmts);
            }

            expect(TOKEN_RIGHT_BRACE, "expected '}'");
    } else {
        if (insert_sym_from_funcdecl_node(global_scope, node, true) == false)
            sym_error = true;
        expect(TOKEN_SEMICOLON, "expected ';'");
    }
}

static void parse_func_or_decl(ast_node_t* parent) {
    token_t* token_type;
    token_t* token_ident;
    current_scope = global_scope;

    if (panic_mode)
        synchronize_decl();

    if (match_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_type = consume();
        if (match(TOKEN_IDENTIFIER)) {
            token_ident = consume();

            if (match(TOKEN_LEFT_PAREN)) {
                parse_funcdecl(parent, token_type, token_ident);
            } else {
                parse_vardecls(parent, token_type, token_ident);
                while(match(TOKEN_COMMA)) {
                    consume();
                    if (match(TOKEN_IDENTIFIER)) {
                        token_ident = consume();
                        parse_vardecls(parent, token_type, token_ident);
                    }
                }
                expect(TOKEN_SEMICOLON, "expected ';'");
            }
        }
    }
    else if (match(TOKEN_VOID)) {
        token_type = consume();
        if (match(TOKEN_IDENTIFIER)) {
            token_ident = consume();

            if (match(TOKEN_LEFT_PAREN)) {
                parse_funcdecl(parent, token_type, token_ident);
            }
            /*
            else {
                parse_vardecls(parent, token_type, token_ident);
                while(match(TOKEN_COMMA)) {
                    consume();
                    if (match(TOKEN_IDENTIFIER)) {
                        token_ident = consume();
                        parse_vardecls(parent, token_type, token_ident);
                    }
                }
                expect(TOKEN_SEMICOLON, "expected ';'");
            }*/
        }
    }
    else {
        error_at(peek(0), "unexpected token");
        synchronize_decl();
    }
}

ast_node_t* parse(char *buffer) {
    parser_error = false;
    panic_mode = false;
    sym_error = false;
    global_scope = create_sym_table(NULL);
    current_scope = global_scope;

    get_tokens(buffer);
    //show_tokens();

    ast_node_t* ast = create_ast_node_root();

    while(!match(TOKEN_EOF)) {
        parse_func_or_decl(ast->as.root.stmts);
    }

    while (!match(TOKEN_EOF)) {
        token_t* token = consume();
        error_at(token, "unexpected token");
    }


    if (parser_error || token_stream.error || sym_error) {
        return NULL;
    }

    //show_sym_table(global_scope);

    return ast;
}
