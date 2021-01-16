#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"



ast_node_list_t* create_ast_node_list() {
    ast_node_list_t* ast_node_list = malloc(sizeof(ast_node_list_t));
    if (ast_node_list == NULL) {
        fprintf(stderr, "Could not allocate memory for ast_node_list\n");
        exit(EXIT_FAILURE);
    }
    memset(ast_node_list, 0, sizeof(ast_node_list_t));
    return ast_node_list;
}

ast_node_t* create_ast_node(ast_node_type_t type) {
    ast_node_t* node = malloc(sizeof(ast_node_t));
    if (node == NULL) {
        fprintf(stderr, "Could not allocate memory for ast_node\n");
        exit(EXIT_FAILURE);
    }
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    return node;
}

ast_node_t* create_ast_node_root() {
    ast_node_t* node =  create_ast_node(NODE_ROOT);
    node->as.root.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_number(int number) {
    ast_node_t* node =  create_ast_node(NODE_INT);
    node->as.number = number;
    return node;
}

ast_node_t* create_ast_node_ident(char* ident) {
    ast_node_t* node =  create_ast_node(NODE_IDENT);
    node->as.ident = ident;
    return node;
}

ast_node_t* create_ast_node_string(char* string) {
    ast_node_t* node =  create_ast_node(NODE_STRING);
    node->as.string = string;
    return node;
}

ast_node_t* create_ast_node_char(char* string) {
    ast_node_t* node =  create_ast_node(NODE_CHAR);
    node->as.character = string;
    return node;
}

ast_node_t* create_ast_node_funccall(ast_node_t* ident) {
    ast_node_t* node =  create_ast_node(NODE_FUNCCALL);
    node->as.funccall.ident = ident;
    node->as.funccall.params = create_ast_node_param_list();
    return node;
}

ast_node_t* create_ast_node_funcdecl(decl_type_t type, ast_node_t* ident) {
    ast_node_t* node =  create_ast_node(NODE_FUNCDECL);
    node->as.funcdecl.type = type;
    node->as.funcdecl.ident = ident;
    node->as.funcdecl.params = create_ast_node_paramdecl_list();
    node->as.funcdecl.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_if(ast_node_t *cond) {
    ast_node_t* node =  create_ast_node(NODE_IF);
    node->as.ifstmt.cond = cond;
    node->as.ifstmt._if = create_ast_node_stmtlist();
    node->as.ifstmt._else = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_while(ast_node_t *cond) {
    ast_node_t* node =  create_ast_node(NODE_WHILE);
    node->as.whilestmt.cond = cond;
    node->as.whilestmt.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_for(ast_node_t *init, ast_node_t *cond, ast_node_t *incr) {
    ast_node_t* node =  create_ast_node(NODE_FOR);
    node->as.forstmt.init = init;
    node->as.forstmt.cond = cond;
    node->as.forstmt.incr = incr;
    node->as.forstmt.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_return(ast_node_t *expr) {
    ast_node_t* node =  create_ast_node(NODE_RETURN);
    node->as._return.expr = expr;
    return node;
}


ast_node_t* create_ast_node_assign(ast_node_t* left, ast_node_t* right) {
    ast_node_t* node =  create_ast_node(NODE_ASSIGN);
    node->as.assign.left = left;
    node->as.assign.right = right;
    return node;
}

ast_node_t* create_ast_node_arrayaccess(ast_node_t* ident, ast_node_t* expr) {
    ast_node_t* node =  create_ast_node(NODE_ARRAYACCESS);
    node->as.arrayaccess.ident = ident;
    node->as.arrayaccess.expr = expr;
    return node;
}

ast_node_t* create_ast_node_stmtlist() {
    ast_node_t* node =  create_ast_node(NODE_STMTSLIST);
    node->as.stmtslist.list = malloc(sizeof(ast_node_list_t));
    if (node->as.stmtslist.list == NULL) {
        fprintf(stderr, "Could not allocate memory for ast_node_list\n");
        exit(EXIT_FAILURE);
    }
    node->as.stmtslist.list->head = NULL;
    node->as.stmtslist.list->tail = NULL;
    return node;
}

ast_node_t* create_ast_node_paramdecl_list() {
    ast_node_t* node =  create_ast_node(NODE_PARAMDECL_LIST);
    node->as.paramsdecllist.list = malloc(sizeof(ast_node_list_t));
    if (node->as.paramsdecllist.list == NULL) {
        fprintf(stderr, "Could not allocate memory for ast_node_list\n");
        exit(EXIT_FAILURE);
    }
    node->as.paramsdecllist.list->head = NULL;
    node->as.paramsdecllist.list->tail = NULL;
    return node;
}

ast_node_t* create_ast_node_paramdecl(decl_type_t type, ast_node_t* ident, bool is_array) {
    ast_node_t* node =  create_ast_node(NODE_PARAMDECL);
    node->as.paramdecl.type = type;
    node->as.paramdecl.ident = ident;
    node->as.paramdecl.is_array = is_array;
    return node;
}

ast_node_t* create_ast_node_vardecl(decl_type_t type, ast_node_t* ident, bool is_array, int size) {
    ast_node_t* node =  create_ast_node(NODE_VARDECL);
    node->as.vardecl.type = type;
    node->as.vardecl.ident = ident;
    node->as.vardecl.is_array = is_array;
    node->as.vardecl.size = size;
    return node;
}


ast_node_t* create_ast_node_param_list() {
    ast_node_t* node =  create_ast_node(NODE_PARAM_LIST);
    node->as.paramslist.list = malloc(sizeof(ast_node_list_t));
    if (node->as.paramslist.list == NULL) {
        fprintf(stderr, "Could not allocate memory for ast_node_list\n");
        exit(EXIT_FAILURE);
    }
    node->as.paramslist.list->head = NULL;
    node->as.paramslist.list->tail = NULL;
    return node;
}


ast_node_t* create_ast_node_unary(op_t op, ast_node_t* expr) {
    ast_node_t* node =  create_ast_node(NODE_UNARYOP);
    node->as.unary.op = op;
    node->as.unary.expr = expr;
    return node;
}

ast_node_t* create_ast_node_binary(op_t op, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node =  create_ast_node(NODE_BINOP);
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

void add_param(ast_node_t* parent, ast_node_t* child) {
    if (parent->as.paramslist.list->head == NULL) {
        parent->as.paramslist.list->head = child;
    } else {
        parent->as.paramslist.list->tail->next = child;
    }
    parent->as.paramslist.list->tail = child;
}

void add_paramdecl(ast_node_t* parent, ast_node_t* child) {
    if (parent->as.paramsdecllist.list->head == NULL) {
        parent->as.paramsdecllist.list->head = child;
    } else {
        parent->as.paramsdecllist.list->tail->next = child;
    }
    parent->as.paramsdecllist.list->tail = child;
}

void add_stmt(ast_node_t* parent, ast_node_t* child) {
    if (parent->as.stmtslist.list->head == NULL) {
        parent->as.stmtslist.list->head = child;
    } else {
        parent->as.stmtslist.list->tail->next = child;
    }
    parent->as.stmtslist.list->tail = child;
}

