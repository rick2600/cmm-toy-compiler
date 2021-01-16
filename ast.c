#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "token.h"



static op_t tokentype_to_op(token_type_t type) {
    if (type == TOKEN_PLUS)       return OP_PLUS;
    else if (type == TOKEN_MINUS) return OP_MINUS;
    else if (type == TOKEN_SLASH) return OP_DIV;
    else if (type == TOKEN_STAR)  return OP_MULT;
    else if (type == TOKEN_OR)    return OP_OR;
    else if (type == TOKEN_AND)   return OP_AND;
    else if (type == TOKEN_BANG)  return OP_NOT;
    else if (type == TOKEN_EQUAL_EQUAL)     return OP_EQ;
    else if (type == TOKEN_BANG_EQUAL)      return OP_NEQ;
    else if (type == TOKEN_LESS_EQUAL)      return OP_LE;
    else if (type == TOKEN_LESS)            return OP_LT;
    else if (type == TOKEN_GREATER_EQUAL)   return OP_GE;
    else if (type == TOKEN_GREATER)         return OP_GT;
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

ast_node_t* create_ast_node_number(token_t* token) {
    ast_node_t* node =  create_ast_node(NODE_INT);
    char *str = lexeme(token);
    node->line = token->line;
    node->as.number.value = atoi(str);
    free(str);
    return node;
}

ast_node_t* create_ast_node_ident(token_t* token) {
    ast_node_t* node =  create_ast_node(NODE_IDENT);
    node->line = token->line;
    node->as.ident.value = lexeme(token);
    return node;
}

ast_node_t* create_ast_node_string(token_t* token) {
    ast_node_t* node =  create_ast_node(NODE_STRING);
    node->line = token->line;
    node->as.string.value = lexeme(token);
    return node;
}

ast_node_t* create_ast_node_char(token_t* token) {
    ast_node_t* node =  create_ast_node(NODE_CHAR);
    node->line = token->line;
    node->as.character.value = lexeme(token);
    return node;
}

ast_node_t* create_ast_node_funccall(ast_node_t* ident) {
    ast_node_t* node =  create_ast_node(NODE_FUNCCALL);
    node->line = ident->line;
    node->as.funccall.ident = ident;
    node->as.funccall.params = create_ast_node_param_list();
    return node;
}

ast_node_t* create_ast_node_funcdecl(decl_type_t type, ast_node_t* ident) {
    ast_node_t* node =  create_ast_node(NODE_FUNCDECL);
    node->line = ident->line;
    node->as.funcdecl.type = type;
    node->as.funcdecl.ident = ident;
    node->as.funcdecl.params = create_ast_node_paramdecl_list();
    node->as.funcdecl.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_if(token_t* token, ast_node_t *cond) {
    ast_node_t* node =  create_ast_node(NODE_IF);
    node->line = token->line;
    node->as.ifstmt.cond = cond;
    node->as.ifstmt._if = create_ast_node_stmtlist();
    node->as.ifstmt._else = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_while(token_t* token, ast_node_t *cond) {
    ast_node_t* node =  create_ast_node(NODE_WHILE);
    node->line = token->line;
    node->as.whilestmt.cond = cond;
    node->as.whilestmt.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_for(token_t* token, ast_node_t *init,
                                ast_node_t *cond, ast_node_t *incr) {
    ast_node_t* node =  create_ast_node(NODE_FOR);
    node->line = token->line;
    node->as.forstmt.init = init;
    node->as.forstmt.cond = cond;
    node->as.forstmt.incr = incr;
    node->as.forstmt.stmts = create_ast_node_stmtlist();
    return node;
}

ast_node_t* create_ast_node_return(token_t* token, ast_node_t *expr) {
    ast_node_t* node =  create_ast_node(NODE_RETURN);
    node->line = token->line;
    node->line = expr->line;
    node->as._return.expr = expr;
    return node;
}


ast_node_t* create_ast_node_assign(ast_node_t* left, ast_node_t* right) {
    ast_node_t* node =  create_ast_node(NODE_ASSIGN);
    node->line = left->line;
    node->as.assign.left = left;
    node->as.assign.right = right;
    return node;
}

ast_node_t* create_ast_node_arrayaccess(ast_node_t* ident, ast_node_t* expr) {
    ast_node_t* node = create_ast_node(NODE_ARRAYACCESS);
    node->line = ident->line;
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
    node->line = ident->line;
    node->as.paramdecl.type = type;
    node->as.paramdecl.ident = ident;
    node->as.paramdecl.is_array = is_array;
    return node;
}

ast_node_t* create_ast_node_vardecl(decl_type_t type, ast_node_t* ident, bool is_array, int size) {
    ast_node_t* node =  create_ast_node(NODE_VARDECL);
    node->line = ident->line;
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


ast_node_t* create_ast_node_unary(token_t* token, ast_node_t* expr) {
    op_t op = tokentype_to_op(token->type);
    ast_node_t* node =  create_ast_node(NODE_UNARYOP);
    node->line = token->line;
    node->as.unary.op = op;
    node->as.unary.expr = expr;
    return node;
}

ast_node_t* create_ast_node_binary(token_t* token, ast_node_t* left, ast_node_t* right) {
    op_t op = tokentype_to_op(token->type);
    ast_node_t* node =  create_ast_node(NODE_BINOP);
    node->line = token->line;
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
    if (parent->line == 0) {
        parent->line = child->line;
    }
}

void add_paramdecl(ast_node_t* parent, ast_node_t* child) {
    if (parent->as.paramsdecllist.list->head == NULL) {
        parent->as.paramsdecllist.list->head = child;
    } else {
        parent->as.paramsdecllist.list->tail->next = child;
    }
    parent->as.paramsdecllist.list->tail = child;
    if (parent->line == 0) {
        parent->line = child->line;
    }
}

void add_stmt(ast_node_t* parent, ast_node_t* child) {
    if (parent->as.stmtslist.list->head == NULL) {
        parent->as.stmtslist.list->head = child;
    } else {
        parent->as.stmtslist.list->tail->next = child;
    }
    parent->as.stmtslist.list->tail = child;
    if (parent->line == 0) {
        parent->line = child->line;
    }
}

