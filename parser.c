#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "scanner.h"
#include "ast.h"
#include "sym_table.h"
#include "ast_visitor.h"


parser_t parser;

static void parse_stmt(ast_node_t* parent);
static ast_node_t* parse_expr();

static void fatal_error(const char* msg) {
    fprintf(stderr, "fatal error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static token_t* peek(uint32_t dist) {
    if ((parser.cur_position + dist) >= parser.token_stream->count)
        return &parser.token_stream->tokens[parser.token_stream->count - 1];
    else
        return &parser.token_stream->tokens[parser.cur_position + dist];
}

static void debug_token(token_t* token) {
    printf("[%-18s]  '%.*s'\n",
        stringify_token_type(token->type),
        token->length, token->start
    );
}

static void show_tokens() {
    int current = 0;
    for (;;) {
        if (peek(current)->type == TOKEN_EOF) break;
        debug_token(peek(current));
        current++;
    }
    printf("\n\n");
}

static void advance() {
    if (parser.cur_position < parser.token_stream->count)
        parser.cur_position++;
}

static token_t* last_token() {
    if ((int32_t)parser.cur_position > 0)
        return &parser.token_stream->tokens[parser.cur_position - 1];
    else
        fatal_error("index error in last_token()");
}

static token_t* next_token() {
    if (parser.cur_position < parser.token_stream->count) {
        advance();
        return last_token();
    }
    else {
        // EOF token.
        return &parser.token_stream->tokens[parser.token_stream->count];
    }
}

static void error_at(token_t* token, const char* msg) {
    if (parser.panic_mode)
        return;

    parser.panic_mode = true;
    parser.had_error = true;

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

bool match(token_type_t type) {
    char msg[128];
    token_t* token = next_token();
    if (token->type == type) {
        return true;
    } else {
        sprintf(msg, "expected '%s'", token_type_str(type));
        error_at(token, msg);
        return false;
    }
}

bool is_next_token(token_type_t type) {
    return peek(0)->type == type;
}

bool is_next_token_any(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
        if (peek(0)->type == va_arg(args, int))
            return true;
    va_end(args);
    return false;
}

static void synchronize() {
    parser.panic_mode = false;

    while (peek(0)->type != TOKEN_EOF) {
        if (is_next_token_any(4,
            TOKEN_FOR, TOKEN_IF, TOKEN_WHILE, TOKEN_RETURN))
                return;

        /*
        if ((int32_t)parser.cur_position > 0)
            if (last_token()->type == TOKEN_SEMICOLON)
                return;
        */

        advance();
    }
}

static void synchronize_global() {
    parser.panic_mode = false;

    while (peek(0)->type != TOKEN_EOF) {
        bool next_is_char_int_void = is_next_token_any(3,
            TOKEN_VOID, TOKEN_CHAR, TOKEN_INT);

        if ((int32_t)parser.cur_position > 0)
            if (last_token()->type == TOKEN_SEMICOLON && next_is_char_int_void)
                return;
        advance();
    }
}

static void check_use_before_decl(ast_node_t* ident) {
    char* sym = ident->as.ident.value;

    if (sym_lookup(parser.cur_sym_table, sym) != NULL)
        return;

    if (sym_lookup(parser.global_sym_table, sym) != NULL)
        return;

    fprintf(stderr,
        "Line: %d: error: \"%s\" used before a declaration\n",
        ident->line, sym);

    parser.had_error = true;
}

static ast_node_t* parse_funccall(ast_node_t* ident) {
    match(TOKEN_LEFT_PAREN);
    ast_node_t* node = create_ast_node_funccall(ident);

    check_use_before_decl(ident);

    if (!is_next_token(TOKEN_RIGHT_PAREN)) {
        add_param(node->as.funccall.params, parse_expr());

        while (is_next_token(TOKEN_COMMA)) {
            match(TOKEN_COMMA);
            add_param(node->as.funccall.params, parse_expr());
        }
    }
    match(TOKEN_RIGHT_PAREN);
    return node;
}

static ast_node_t* parse_arrayaccess(ast_node_t* ident) {
    match(TOKEN_LEFT_BRACKET);
    ast_node_t* node = create_ast_node_arrayaccess(ident, parse_expr());
    match(TOKEN_RIGHT_BRACKET);
    return node;
}

static ast_node_t* parse_assign() {
    ast_node_t* node_assign = NULL;

    if (match(TOKEN_IDENT)) {

        token_t* token_ident = last_token();
        ast_node_t* node_ident = create_ast_node_ident(token_ident);

        check_use_before_decl(node_ident);

        if (is_next_token(TOKEN_LEFT_BRACKET)) {
            ast_node_t* arrayaccess = parse_arrayaccess(node_ident);
            match(TOKEN_EQUAL);
            node_assign = create_ast_node_assign(arrayaccess, parse_expr());
        } else if (is_next_token(TOKEN_EQUAL)) {
            match(TOKEN_EQUAL);
            node_assign = create_ast_node_assign(node_ident, parse_expr());
        }
    }
    return node_assign;
}

static ast_node_t* parse_factor() {
    //printf("parse_factor> "); debug_token(peek(0));

    ast_node_t* node = NULL;
    if (is_next_token(TOKEN_IDENT)) {
        ast_node_t* ident = create_ast_node_ident(next_token());

        check_use_before_decl(ident);

        if (is_next_token(TOKEN_LEFT_PAREN)) {
            node = parse_funccall(ident);
        } else if (is_next_token(TOKEN_LEFT_BRACKET)) {
            node = parse_arrayaccess(ident);
        } else {
            node = ident;
        }
    } else if (is_next_token(TOKEN_NUMBER)) {
        node = create_ast_node_number(next_token());
    } else if (is_next_token(TOKEN_STRING)) {
        node = create_ast_node_string(next_token());
    } else if (is_next_token(TOKEN_CHARCONST)) {
        node = create_ast_node_char(next_token());
    } else if (is_next_token(TOKEN_LEFT_PAREN)) {
        match(TOKEN_LEFT_PAREN);
        node = parse_expr();
        match(TOKEN_RIGHT_PAREN);
    } else if (is_next_token(TOKEN_BANG)) {
        token_t* token_op = next_token();
        node = create_ast_node_unary(token_op, parse_factor());
    } else {
        error_at(next_token(), "unexpected token");
    }
    return node;
}

static ast_node_t* parse_term() {
    ast_node_t* node = parse_factor();

    while (is_next_token_any(3, TOKEN_STAR, TOKEN_SLASH, TOKEN_AND)) {
        token_t* token_op = next_token();
        node = create_ast_node_binary(token_op, node, parse_factor());
    }
    return node;
}

static ast_node_t* parse_expr_simp() {
    ast_node_t* node;

    if (is_next_token(TOKEN_PLUS)) {
        match(TOKEN_PLUS);
    } else if  (is_next_token(TOKEN_MINUS)) {
        token_t* token_op = next_token();
        node = create_ast_node_unary(token_op, parse_term());
    } else {
        node = parse_term();
    }

    while (is_next_token_any(3, TOKEN_PLUS, TOKEN_MINUS, TOKEN_OR)) {
        token_t* token_op = next_token();
        node = create_ast_node_binary(token_op, node, parse_term());
    }
    return node;
}

static ast_node_t* parse_expr() {
    ast_node_t* node = parse_expr_simp();

    while (is_next_token_any(3, TOKEN_EQUAL_EQUAL, TOKEN_BANG_EQUAL, TOKEN_LESS_EQUAL) ||
        is_next_token_any(3, TOKEN_LESS, TOKEN_GREATER_EQUAL, TOKEN_GREATER)) {
            token_t* token_op = next_token();
            node = create_ast_node_binary(token_op, node, parse_expr_simp());
    }
    return node;
}

static void parse_if_stmt(ast_node_t* parent) {
    ast_node_t* node = NULL;
    match(TOKEN_IF);
    token_t* token_if = last_token();

    if (match(TOKEN_LEFT_PAREN)) {
        node = create_ast_node_if(token_if, parse_expr());
        match(TOKEN_RIGHT_PAREN);
        parse_stmt(node->as.ifstmt._if);

        if (is_next_token(TOKEN_ELSE)) {
            match(TOKEN_ELSE);
            parse_stmt(node->as.ifstmt._else);
        }
        add_stmt(parent, node);
    }
}

static void parse_while_stmt(ast_node_t* parent) {
    ast_node_t* node = NULL;
    match(TOKEN_WHILE);
    token_t* token_while = last_token();

    if (match(TOKEN_LEFT_PAREN)) {
        node = create_ast_node_while(token_while, parse_expr());
        match(TOKEN_RIGHT_PAREN);
        parse_stmt(node->as.whilestmt.stmts);
        add_stmt(parent, node);
    }
}

static void parse_for_stmt(ast_node_t* parent) {
    ast_node_t* node = NULL;
    ast_node_t* init = NULL;
    ast_node_t* cond = NULL;
    ast_node_t* incr = NULL;

    match(TOKEN_FOR);
    token_t* token_for = last_token();
    if (match(TOKEN_LEFT_PAREN)) {

        if (!is_next_token(TOKEN_SEMICOLON)) init = parse_assign();
        match(TOKEN_SEMICOLON);

        if (!is_next_token(TOKEN_SEMICOLON)) cond = parse_expr();
        match(TOKEN_SEMICOLON);

        if (!is_next_token(TOKEN_RIGHT_PAREN)) incr = parse_assign();
        match(TOKEN_RIGHT_PAREN);

        node = create_ast_node_for(token_for, init, cond, incr);
        parse_stmt(node->as.forstmt.stmts);
        add_stmt(parent, node);
    }
}

static void parse_return_stmt(ast_node_t* parent) {
    ast_node_t* node = NULL;
    match(TOKEN_RETURN);
    token_t* token_return = last_token();

    if (is_next_token(TOKEN_SEMICOLON)) {
        match(TOKEN_SEMICOLON);
        node = create_ast_node_return(token_return, NULL);
        add_stmt(parent, node);
    } else {
        node = create_ast_node_return(token_return, parse_expr());
        match(TOKEN_SEMICOLON);
        add_stmt(parent, node);
    }
}

static void parse_brace_stmt(ast_node_t* parent) {
    match(TOKEN_LEFT_BRACE);
    while (
        is_next_token_any(4, TOKEN_IF, TOKEN_WHILE, TOKEN_FOR, TOKEN_RETURN) ||
        is_next_token_any(3, TOKEN_IDENT, TOKEN_LEFT_BRACE, TOKEN_SEMICOLON)) {
            parse_stmt(parent);
    }
    match(TOKEN_RIGHT_BRACE);
}

static void parse_ident_stmt(ast_node_t* parent) {
    if (peek(1)->type == TOKEN_LEFT_PAREN) {
        match(TOKEN_IDENT);
        token_t* token_ident = last_token();
        ast_node_t* node = parse_funccall(create_ast_node_ident(token_ident));
        add_stmt(parent, node);
    } else {
        add_stmt(parent, parse_assign());
        match(TOKEN_SEMICOLON);
    }
}

static void parse_stmt(ast_node_t* parent) {
    if (parser.panic_mode)
        synchronize();

    if (is_next_token(TOKEN_IF)) {
        parse_if_stmt(parent);

    } else if (is_next_token(TOKEN_WHILE)) {
        parse_while_stmt(parent);

    } else if (is_next_token(TOKEN_FOR)) {
        parse_for_stmt(parent);

    } else if (is_next_token(TOKEN_RETURN)) {
        parse_return_stmt(parent);

    } else if (is_next_token(TOKEN_LEFT_BRACE)) {
        parse_brace_stmt(parent);

    } else if (is_next_token(TOKEN_IDENT)) {
        parse_ident_stmt(parent);

    } else if (is_next_token(TOKEN_SEMICOLON)) {
        match(TOKEN_SEMICOLON);
    } else {
        error_at(next_token(), "unexpected token");
    }
}

static void parse_vardecls_for_func(ast_node_t* func_node, ast_node_t* parent) {
    if (is_next_token_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_t* token_type = next_token();
        token_type_t type = tokentype_2_decltype(token_type->type);
        if (match(TOKEN_IDENT)) {
            ast_node_t* ident = create_ast_node_ident(last_token());
            bool is_array = false;
            int array_size = 0;
            if (is_next_token(TOKEN_LEFT_BRACKET)) {
                match(TOKEN_LEFT_BRACKET);
                is_array = true;
                if (match(TOKEN_NUMBER)) {
                    char* s = lexeme(last_token());
                    array_size = atoi(s);
                    free(s);
                }
                match(TOKEN_RIGHT_BRACKET);
            }
            ast_node_t* node = create_ast_node_vardecl(
                type, ident, is_array, array_size);

            sym_entry_t* entry = sym_lookup(parser.global_sym_table,
                func_node->as.funcdecl.ident->as.ident.value);


            if (!insert_sym_from_vardecl_node(entry->as.func.sym_table, node)) {
                parser.had_error = true;
            }

            add_stmt(parent, node);
        }
    }
}

static void begin_parse_vardecls_for_func(
    ast_node_t* func_node, ast_node_t* parent) {

    parse_vardecls_for_func(func_node, parent);
    while (is_next_token(TOKEN_COMMA)) {
        advance();
        parse_vardecls_for_func(func_node, parent);
    }
    match(TOKEN_SEMICOLON);
}

static ast_node_t* parse_param() {
    ast_node_t* node = NULL;
    token_t* token_type;
    token_t* token_ident;
    bool is_array = false;

    if (is_next_token_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_type = next_token();
        if (match(TOKEN_IDENT)) {
            token_ident = last_token();
            ast_node_t* ident = create_ast_node_ident(token_ident);
            decl_type_t argtype = tokentype_2_decltype(token_type->type);
            if (is_next_token(TOKEN_LEFT_BRACKET)) {
                match(TOKEN_LEFT_BRACKET);
                is_array = true;
                match(TOKEN_RIGHT_BRACKET);
            }
            node = create_ast_node_paramdecl(argtype, ident, is_array);
        }
    } else if (is_next_token(TOKEN_VOID)) {
        next_token();
    } else {
        error_at(next_token(), "expected 'int', 'char' or 'void'");
    }
    return node;
}

static ast_node_t* parse_params() {
    ast_node_t* node = create_ast_node_paramdecl_list();
    ast_node_t* param = parse_param();
    if (param != NULL) {
        add_paramdecl(node, param);
        while (is_next_token(TOKEN_COMMA)) {
            match(TOKEN_COMMA);
            add_paramdecl(node, parse_param());
        }
    }
    return node;
}

static ast_node_t* parse_funcdecl(ast_node_t* parent, token_t* token_type) {
    if (match(TOKEN_IDENT)) {
        token_t* token_ident = last_token();
        match(TOKEN_LEFT_PAREN);
        token_type_t type = tokentype_2_decltype(token_type->type);
        ast_node_t* ident = create_ast_node_ident(token_ident);
        ast_node_t* params = parse_params();
        ast_node_t* node = create_ast_node_funcdecl(type, ident);
        node->as.funcdecl.params = params;
        match(TOKEN_RIGHT_PAREN);
        add_stmt(parent, node);
        if (!insert_sym_from_funcdecl_prototype_node(parser.global_sym_table, node)) {
            parser.had_error = true;
        }
        return node;
        // TODO: add symbol
    }
    return NULL; // TODO: return a func anyway?
}

static void set_sym_scope_to_func(ast_node_t* node) {
    sym_entry_t* entry = sym_lookup(parser.global_sym_table,
            node->as.funcdecl.ident->as.ident.value);

    parser.cur_sym_table = entry->as.func.sym_table;
}

// Do not allow any var for this symbol anymore.
static void lock_sym_table(ast_node_t* node) {
    sym_entry_t* entry = sym_lookup(parser.global_sym_table,
            node->as.funcdecl.ident->as.ident.value);
    entry->as.func.sym_table->accepts_new_var = false;
}

static void begin_parse_funcdecl(ast_node_t* parent, token_t* token_type) {
    if (parser.panic_mode) return;
    ast_node_t* node = parse_funcdecl(parent, token_type);
    if (node == NULL)
        return;

    if (is_next_token(TOKEN_COMMA)) {
        while (match(TOKEN_COMMA)) {
            parse_funcdecl(parent, token_type);
        }
        match(TOKEN_SEMICOLON);
    } else if (is_next_token(TOKEN_LEFT_BRACE)) {
        if (!insert_sym_from_funcdef_node(parser.global_sym_table, node)) {
            parser.had_error = true;
        }

        set_sym_scope_to_func(node);

        match(TOKEN_LEFT_BRACE);

        while (is_next_token_any(2, TOKEN_INT, TOKEN_CHAR)) {
            begin_parse_vardecls_for_func(node, node->as.funcdecl.stmts);
        }

        while (!is_next_token(TOKEN_RIGHT_BRACE) && !is_next_token(TOKEN_EOF)) {
            parse_stmt(node->as.funcdecl.stmts);
        }

        lock_sym_table(node);

        match(TOKEN_RIGHT_BRACE);
    } else if (is_next_token(TOKEN_SEMICOLON)) {
        match(TOKEN_SEMICOLON);
    } else {
        error_at(next_token(), "expected ';' or '{'");
    }
}

static void parse_vardecls(ast_node_t* parent, token_t* token_type) {
    if (parser.panic_mode) return;
    token_type_t type = tokentype_2_decltype(token_type->type);

    if (is_next_token(TOKEN_IDENT)) {
        ast_node_t* ident = create_ast_node_ident(next_token());
        int array_size = 0;
        bool is_array = false;

        if (is_next_token(TOKEN_LEFT_BRACKET)) {
            match(TOKEN_LEFT_BRACKET);
            if (match(TOKEN_NUMBER)) {
                char* s = lexeme(last_token());
                array_size = atoi(s);
                free(s);
            }
            match(TOKEN_RIGHT_BRACKET);
        }
        ast_node_t* node = create_ast_node_vardecl(
            type, ident, is_array, array_size);

        add_stmt(parent, node);
        if (!insert_sym_from_vardecl_node(parser.global_sym_table, node)) {
            parser.had_error = true;
        }
    }
}

static void begin_parse_vardecls(ast_node_t* parent, token_t* token_type) {
    if (parser.panic_mode) return;

    parse_vardecls(parent, token_type);
    while (is_next_token(TOKEN_COMMA)) {
        match(TOKEN_COMMA);
        parse_vardecls(parent, token_type);
    }
    match(TOKEN_SEMICOLON);
}

static void parse_func_or_decl(ast_node_t* parent) {
    token_t* token_type;

    if (parser.panic_mode)
        synchronize_global();

    if (is_next_token_any(2, TOKEN_INT, TOKEN_CHAR)) {
        token_type = next_token();
        if (is_next_token(TOKEN_IDENT)) {
            if (peek(1)->type == TOKEN_LEFT_PAREN)
                begin_parse_funcdecl(parent, token_type);
            else
                begin_parse_vardecls(parent, token_type);
        } else {
            error_at(next_token(), "expected 'identifier'");
        }
    } else if (is_next_token(TOKEN_VOID)) {
        token_type = next_token();
        begin_parse_funcdecl(parent, token_type);
    } else {
        if (!is_next_token(TOKEN_EOF))
            error_at(next_token(), "expected 'int' or 'char' or 'void'");
    }
}

static void init_parser() {
    parser.cur_position = 0;
    parser.token_stream = NULL;
    parser.panic_mode = NULL;
    parser.had_error = false;
    parser.global_sym_table = create_sym_table(NULL);
    parser.cur_sym_table = NULL;
}

ast_node_t* parse(char *buffer) {
    init_parser();
    parser.token_stream = get_tokens(buffer);
    //show_tokens();
    ast_node_t* ast = create_ast_node_root();

    while (!is_next_token(TOKEN_EOF)) {
        parse_func_or_decl(ast->as.root.stmts);
    }

    //show_sym_table(parser.global_sym_table);

    if (parser.had_error)
        return NULL;

    return ast;
}
