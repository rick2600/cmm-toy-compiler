#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast_show.h"

#define LEVEL_STEP 2

static char* node_type_to_str(ast_node_type_t type) {
    switch (type) {
        case NODE_ROOT: return "(root)";
        case NODE_BINOP: return "(binop)";
        case NODE_UNARYOP: return "(unaryop)";
        case NODE_INT: return "(int)";
        case NODE_IDENT: return "(ident)";
        case NODE_STRING: return "(string)";
        case NODE_CHAR: return "(char)";
        case NODE_FUNCCALL: return "(funccall)";
        case NODE_FUNCDECL: return "(funcdecl)";
        case NODE_PARAM_LIST: return "(param_list)";
        case NODE_PARAMDECL: return "(paramdecl)";
        case NODE_PARAMDECL_LIST: return "(paramdecl_list)";
        case NODE_VARDECL: return "(vardecl)";
        case NODE_IF: return "(if)";
        case NODE_WHILE: return "(while)";
        case NODE_FOR: return "(for)";
        case NODE_RETURN: return "(return)";
        case NODE_STMTSLIST: return "(stmtslist)";
        case NODE_ASSIGN: return "(assign)";
        case NODE_ARRAYACCESS: return "(arrayaccess)";
        default: return "(unknown)";
    }
}

static char* decltype_to_str(decl_type_t type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_CHAR: return "char";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}

static char* op_to_str(op_t op) {
    switch (op) {
        case OP_PLUS: return "+";
        case OP_MINUS: return "-";
        case OP_MULT: return "*";
        case OP_DIV: return "/";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LE: return "<=";
        case OP_LT: return "<";
        case OP_GE: return ">=";
        case OP_GT: return ">";
        case OP_NOT: return "!";
        default: return "unknown";
    }
}

void print_with_indent(int line, char* str, int level) {
    printf("Line: %d -> %*s%s\n", line, level, "", str);
}

static void do_show_ast(char *field, ast_node_t* node, int level) {
    char str[1024];

    if (node == NULL) return;

    if (node->type == NODE_ROOT) {
        //printf("root:\n");
        do_show_ast("stmts", node->as.root.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_BINOP) {
        sprintf(str, "%s: %s op: %s",
            field, node_type_to_str(node->type), op_to_str(node->as.binary.op));

        print_with_indent(node->line, str, level);

        do_show_ast("left", node->as.binary.left, level + LEVEL_STEP);
        do_show_ast("right", node->as.binary.right, level + LEVEL_STEP);
    }
    else if (node->type == NODE_UNARYOP) {
        sprintf(str, "%s: %s op: %s",
            field, node_type_to_str(node->type), op_to_str(node->as.unary.op));
        print_with_indent(node->line, str, level);
        do_show_ast("expr", node->as.unary.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_INT) {
        sprintf(str, "%s: %s value: %d",
            field, node_type_to_str(node->type), node->as.number.value);
        print_with_indent(node->line, str, level);
    }
    else if (node->type == NODE_CHAR) {
        sprintf(str, "%s: %s value: '%s'",
            field, node_type_to_str(node->type), node->as.character.value);
        print_with_indent(node->line, str, level);
    }
    else if (node->type == NODE_IDENT) {
        size_t size = strlen(node->as.string.value) + 64;
        char* dynamic_str = malloc(size * sizeof(char));
        if (dynamic_str == NULL) {
            fprintf(stderr, "Could not allocate memory for NODE_STRING\n");
            exit(EXIT_FAILURE);
        }
        sprintf(dynamic_str, "%s: %s value: '%s'",
            field, node_type_to_str(node->type), node->as.ident.value);
        print_with_indent(node->line, dynamic_str, level);
    }
    else if (node->type == NODE_STRING) {
        size_t size = strlen(node->as.string.value) + 64;
        char* dynamic_str = malloc(size * sizeof(char));
        if (dynamic_str == NULL) {
            fprintf(stderr, "Could not allocate memory for NODE_STRING\n");
            exit(EXIT_FAILURE);
        }
        sprintf(dynamic_str, "%s: %s value: \"%s\"",
            field, node_type_to_str(node->type), node->as.string.value);
        print_with_indent(node->line, dynamic_str, level);
    }
    else if (node->type == NODE_FUNCCALL) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("ident", node->as.funccall.ident, level + LEVEL_STEP);
        do_show_ast("params", node->as.funccall.params, level + LEVEL_STEP);
    }
    else if (node->type == NODE_PARAM_LIST) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        ast_node_t* param = node->as.paramslist.list->head;
        while (param) {
            do_show_ast("param", param, level + LEVEL_STEP);
            param = param->next;
        }
    }
    else if (node->type == NODE_PARAMDECL_LIST) {
        if (node->as.paramsdecllist.list->head == NULL) return;
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        ast_node_t* param = node->as.paramsdecllist.list->head;
        while (param) {
            do_show_ast("param", param, level + LEVEL_STEP);
            param = param->next;
        }
    }
    else if (node->type == NODE_IF) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("cond", node->as.ifstmt.cond, level + LEVEL_STEP);
        do_show_ast("_if", node->as.ifstmt._if, level + LEVEL_STEP);
        do_show_ast("_else", node->as.ifstmt._else, level + LEVEL_STEP);
    }
    else if (node->type == NODE_WHILE) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("cond", node->as.whilestmt.cond, level + LEVEL_STEP);
        do_show_ast("stmts", node->as.whilestmt.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_FOR) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("init", node->as.forstmt.init, level + LEVEL_STEP);
        do_show_ast("cond", node->as.forstmt.cond, level + LEVEL_STEP);
        do_show_ast("incr", node->as.forstmt.incr, level + LEVEL_STEP);
        do_show_ast("stmts", node->as.forstmt.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_FUNCDECL) {
        sprintf(str, "%s: %s type: %s", field, node_type_to_str(node->type),
            decltype_to_str(node->as.funcdecl.type));
        print_with_indent(node->line, str, level);
        do_show_ast("ident", node->as.funcdecl.ident, level + LEVEL_STEP);
        do_show_ast("params", node->as.funcdecl.params, level + LEVEL_STEP);
        do_show_ast("stmts", node->as.funcdecl.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_RETURN) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("expr", node->as._return.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_STMTSLIST) {
        if (node->as.stmtslist.list->head == NULL) return;
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        ast_node_t* stmt = node->as.stmtslist.list->head;
        while (stmt) {
            do_show_ast("stmt", stmt, level + LEVEL_STEP);
            stmt = stmt->next;
        }
    }
    else if (node->type == NODE_ASSIGN) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("left", node->as.assign.left, level + LEVEL_STEP);
        do_show_ast("right", node->as.assign.right, level + LEVEL_STEP);
    }
    else if (node->type == NODE_ARRAYACCESS) {
        sprintf(str, "%s: %s", field, node_type_to_str(node->type));
        print_with_indent(node->line, str, level);
        do_show_ast("ident", node->as.arrayaccess.ident, level + LEVEL_STEP);
        do_show_ast("expr", node->as.arrayaccess.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_PARAMDECL) {
        sprintf(str, "%s: %s type: %s %s",
            field,
            node_type_to_str(node->type),
            decltype_to_str(node->as.paramdecl.type),
            node->as.paramdecl.is_array ? "[]":""
        );
        print_with_indent(node->line, str, level);
        do_show_ast("ident", node->as.paramdecl.ident, level + LEVEL_STEP);
    }
    else if (node->type == NODE_VARDECL) {
        char s[16];
        sprintf(s, "[%d]", node->as.vardecl.size);

        sprintf(str, "%s: %s type: %s %s",
            field,
            node_type_to_str(node->type),
            decltype_to_str(node->as.vardecl.type),
            node->as.vardecl.is_array ? s : ""
        );
        print_with_indent(node->line, str, level);
        do_show_ast("ident", node->as.vardecl.ident, level + LEVEL_STEP);
    }
}

void show_ast(ast_node_t *node) {
    puts("========================== Abstract Syntax Tree (AST) ==========================");
    do_show_ast("", node, 0);
    puts("================================================================================\n");
}

