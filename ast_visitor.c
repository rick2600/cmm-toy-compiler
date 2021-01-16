#include <stdio.h>
#include <string.h>

#include "ast_visitor.h"

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

static void do_visit_ast(char *field, ast_node_t* node, int level) {
    // FIXME: I know there's a overflow here :P
    char str[1024];

    memset(str, 0, sizeof(str));
    memset(str, ' ', level);

    if (node == NULL) return;

    //printf("[%d]\n", node->type);

    if (node->type == NODE_ROOT) {
        //sprintf(str+level, "root: %s", node_type_to_str(node->type));
        //printf("Line: %d -> %s\n", node->line, str);
        printf("root:\n");
        do_visit_ast("stmts", node->as.root.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_BINOP) {
        sprintf(str+level, "%s: %s op: %s",
            field, node_type_to_str(node->type), op_to_str(node->as.binary.op));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("left", node->as.binary.left, level + LEVEL_STEP);
        do_visit_ast("right", node->as.binary.right, level + LEVEL_STEP);
    }
    else if (node->type == NODE_UNARYOP) {
        sprintf(str+level, "%s: %s op: %s",
            field, node_type_to_str(node->type), op_to_str(node->as.unary.op));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("expr", node->as.unary.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_INT) {
        sprintf(str+level, "%s: %s value: %d",
            field, node_type_to_str(node->type), node->as.number.value);
        printf("Line: %d -> %s\n", node->line, str);
    }
    else if (node->type == NODE_CHAR) {
        sprintf(str+level, "%s: %s value: '%s'",
            field, node_type_to_str(node->type), node->as.character.value);
        printf("Line: %d -> %s\n", node->line, str);
    }
    else if (node->type == NODE_IDENT) {
        sprintf(str+level, "%s: %s value: '%s'",
            field, node_type_to_str(node->type), node->as.ident.value);
        printf("Line: %d -> %s\n", node->line, str);
    }
    else if (node->type == NODE_STRING) {
        sprintf(str+level, "%s: %s val: \"%s\"",
            field, node_type_to_str(node->type), node->as.string.value);
        printf("Line: %d -> %s\n", node->line, str);
    }
    else if (node->type == NODE_FUNCCALL) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("ident", node->as.funccall.ident, level + LEVEL_STEP);
        do_visit_ast("params", node->as.funccall.params, level + LEVEL_STEP);
    }
    else if (node->type == NODE_PARAM_LIST) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        ast_node_t* param = node->as.paramslist.list->head;
        while (param) {
            do_visit_ast("param", param, level + LEVEL_STEP);
            param = param->next;
        }
    }
    else if (node->type == NODE_PARAMDECL_LIST) {
        if (node->as.paramsdecllist.list->head == NULL) return;
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        ast_node_t* param = node->as.paramsdecllist.list->head;
        while (param) {
            do_visit_ast("param", param, level + LEVEL_STEP);
            param = param->next;
        }
    }
    else if (node->type == NODE_IF) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("cond", node->as.ifstmt.cond, level + LEVEL_STEP);
        do_visit_ast("_if", node->as.ifstmt._if, level + LEVEL_STEP);
        do_visit_ast("_else", node->as.ifstmt._else, level + LEVEL_STEP);
    }
    else if (node->type == NODE_WHILE) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("cond", node->as.whilestmt.cond, level + LEVEL_STEP);
        do_visit_ast("stmts", node->as.whilestmt.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_FOR) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("init", node->as.forstmt.init, level + LEVEL_STEP);
        do_visit_ast("cond", node->as.forstmt.cond, level + LEVEL_STEP);
        do_visit_ast("incr", node->as.forstmt.incr, level + LEVEL_STEP);
        do_visit_ast("stmts", node->as.forstmt.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_FUNCDECL) {
        sprintf(str+level, "%s: %s type: %s", field, node_type_to_str(node->type),
            decltype_to_str(node->as.funcdecl.type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("ident", node->as.funcdecl.ident, level + LEVEL_STEP);
        do_visit_ast("params", node->as.funcdecl.params, level + LEVEL_STEP);
        do_visit_ast("stmts", node->as.funcdecl.stmts, level + LEVEL_STEP);
    }
    else if (node->type == NODE_RETURN) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("expr", node->as._return.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_STMTSLIST) {
        if (node->as.stmtslist.list->head == NULL) return;
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        ast_node_t* stmt = node->as.stmtslist.list->head;
        while (stmt) {
            do_visit_ast("stmt", stmt, level + LEVEL_STEP);
            stmt = stmt->next;
        }
    }
    else if (node->type == NODE_ASSIGN) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("left", node->as.assign.left, level + LEVEL_STEP);
        do_visit_ast("right", node->as.assign.right, level + LEVEL_STEP);
    }
    else if (node->type == NODE_ARRAYACCESS) {
        sprintf(str+level, "%s: %s", field, node_type_to_str(node->type));
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("ident", node->as.arrayaccess.ident, level + LEVEL_STEP);
        do_visit_ast("expr", node->as.arrayaccess.expr, level + LEVEL_STEP);
    }
    else if (node->type == NODE_PARAMDECL) {

        sprintf(str+level, "%s: %s type: %s %s",
                field,
                node_type_to_str(node->type),
                decltype_to_str(node->as.paramdecl.type),
                node->as.paramdecl.is_array ? "[]":""
               );
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("ident", node->as.paramdecl.ident, level + LEVEL_STEP);
    }
    else if (node->type == NODE_VARDECL) {
        char s[64];
        sprintf(s, "[%d]", node->as.vardecl.size);

        sprintf(str+level, "%s: %s type: %s %s",
                field,
                node_type_to_str(node->type),
                decltype_to_str(node->as.vardecl.type),
                node->as.vardecl.is_array ? s : ""
               );
        printf("Line: %d -> %s\n", node->line, str);
        do_visit_ast("ident", node->as.vardecl.ident, level + LEVEL_STEP);
    }

}

static int do_evaluate_ast(ast_node_t* node) {
    if (node->type == NODE_BINOP) {
        ast_node_t* left = node->as.binary.left;
        ast_node_t* right = node->as.binary.right;

        switch (node->as.binary.op) {
            case OP_PLUS: return do_evaluate_ast(left) + do_evaluate_ast(right);
            case OP_MINUS: return do_evaluate_ast(left) - do_evaluate_ast(right);
            case OP_MULT: return do_evaluate_ast(left) * do_evaluate_ast(right);
            case OP_DIV: return do_evaluate_ast(left) / do_evaluate_ast(right);
            case OP_AND: return do_evaluate_ast(left) && do_evaluate_ast(right);
            case OP_OR: return do_evaluate_ast(left) || do_evaluate_ast(right);
            case OP_EQ: return do_evaluate_ast(left) == do_evaluate_ast(right);
            case OP_NEQ: return do_evaluate_ast(left) != do_evaluate_ast(right);
            case OP_LE: return do_evaluate_ast(left) <= do_evaluate_ast(right);
            case OP_LT: return do_evaluate_ast(left) < do_evaluate_ast(right);
            case OP_GE: return do_evaluate_ast(left) >= do_evaluate_ast(right);
            case OP_GT: return do_evaluate_ast(left) > do_evaluate_ast(right);
            default: break;
        }

    }
    if (node->type == NODE_UNARYOP) {
        ast_node_t* expr = node->as.unary.expr;
        switch (node->as.unary.op) {
            case OP_MINUS: return -do_evaluate_ast(expr);
            case OP_NOT: return !do_evaluate_ast(expr);
            default: break;
        }
    }
    else if (node->type == NODE_INT) {
        return node->as.number.value;
    }
}



void visit_ast(ast_node_t *node) {
    do_visit_ast("", node, 0);
    //printf("result: %d\n", do_evaluate_ast(node));
}

