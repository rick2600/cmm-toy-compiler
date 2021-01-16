#ifndef ast_h
#define ast_h

#include <stdint.h>
#include <stdbool.h>
#include "token.h"

typedef enum {
    NODE_ROOT,
    NODE_STMTSLIST,
    NODE_INT, NODE_CHAR, NODE_STRING, NODE_IDENT,
    NODE_UNARYOP, NODE_BINOP,
    NODE_FUNCDECL, NODE_PARAMDECL, NODE_PARAMDECL_LIST,
    NODE_FUNCCALL, NODE_PARAM_LIST,
    NODE_IF, NODE_FOR, NODE_WHILE, NODE_RETURN, NODE_ASSIGN, NODE_ARRAYACCESS,
    NODE_VARDECL, NODE_VARDECL_LIST,

} ast_node_type_t;

typedef enum {
    OP_PLUS, OP_MINUS, OP_MULT, OP_DIV, OP_NOT, OP_OR, OP_AND,
    OP_EQ, OP_NEQ, OP_LE, OP_LT, OP_GE, OP_GT
} op_t;


typedef enum {
    TYPE_INT, TYPE_CHAR, TYPE_VOID
} decl_type_t;

typedef struct ast_node {
    ast_node_type_t type;
    struct ast_node* next;
    uint32_t line;

    union {
        //uint32_t number;
        //char* character; // TODO: change to char
        //char* string;
        //char* ident;

        struct {
            struct ast_node* stmts;
        } root;

        struct {
            uint32_t value;
        } number;

        struct {
            char* value;
        } character;

        struct {
            char* value;
        } string;

        struct {
            char* value;
        } ident;

        struct {
            decl_type_t type;
            struct ast_node* ident;
            struct ast_node* params;
            struct ast_node* stmts;
        } funcdecl;

        struct {
            struct ast_node_list* list;
        } paramslist;

        struct {
            struct ast_node_list* list;
        } paramsdecllist;

        struct {
            decl_type_t type;
            struct ast_node* ident;
            bool is_array;
        } paramdecl;

        struct {
            decl_type_t type;
            struct ast_node* ident;
            bool is_array;
            int size;
        } vardecl;

        struct {
            struct ast_node_list* list;
        } stmtslist;

        struct {
            //decl_type_t type;
            struct ast_node* ident;
            struct ast_node* params;
        } funccall;

        struct {
            struct ast_node* cond;
            struct ast_node* _if;
            struct ast_node* _else;
        } ifstmt;

        struct {
            struct ast_node* cond;
            struct ast_node* stmts;
        } whilestmt;

        struct {
            struct ast_node* init;
            struct ast_node* cond;
            struct ast_node* incr;
            struct ast_node* stmts;
        } forstmt;

        struct {
            struct ast_node* expr;
        } _return;

        struct {
            op_t op;
            struct ast_node* expr;
        } unary;

        struct {
            op_t op;
            struct ast_node* left;
            struct ast_node* right;
        } binary;

        struct {
            struct ast_node* ident;
            struct ast_node* expr;
        } arrayaccess;

        struct {
            struct ast_node* left;
            struct ast_node* right;
        } assign;
    } as;
} ast_node_t;

typedef struct ast_node_list {
    struct ast_node* head;
    struct ast_node* tail;
} ast_node_list_t ;

ast_node_t* create_ast_node(ast_node_type_t type);
ast_node_list_t* create_ast_node_list();
void add_stmt(ast_node_t* parent, ast_node_t* stmt);
void add_param(ast_node_t* parent, ast_node_t* child);
void add_paramdecl(ast_node_t* parent, ast_node_t* child);
ast_node_t* create_ast_node_number(token_t* token);
ast_node_t* create_ast_node_ident(token_t* token);
ast_node_t* create_ast_node_string(token_t* token);
ast_node_t* create_ast_node_char(token_t* token);
ast_node_t* create_ast_node_unary(token_t* token, ast_node_t* expr);
ast_node_t* create_ast_node_binary(token_t* token, ast_node_t* left, ast_node_t* right);
ast_node_t* create_ast_node_funccall(ast_node_t* ident);
ast_node_t* create_ast_node_param_list();
ast_node_t* create_ast_node_stmtlist();
ast_node_t* create_ast_node_if(token_t* token, ast_node_t *cond);
ast_node_t* create_ast_node_while(token_t* token, ast_node_t *cond);
ast_node_t* create_ast_node_assign(ast_node_t* left, ast_node_t* right);
ast_node_t* create_ast_node_arrayaccess(ast_node_t* ident, ast_node_t* expr);
ast_node_t* create_ast_node_root();
ast_node_t* create_ast_node_return(token_t* token, ast_node_t *expr);
ast_node_t* create_ast_node_for(token_t* token, ast_node_t *init, ast_node_t *cond, ast_node_t *incr);
ast_node_t* create_ast_node_paramdecl_list();
ast_node_t* create_ast_node_paramdecl(decl_type_t type, ast_node_t* ident, bool is_array);
ast_node_t* create_ast_node_vardecl(decl_type_t type, ast_node_t* ident, bool is_array, int size);
ast_node_t* create_ast_node_funcdecl(decl_type_t type, ast_node_t* ident);

#endif


