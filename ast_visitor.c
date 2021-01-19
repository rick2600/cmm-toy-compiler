#include <stdio.h>
#include <string.h>

#include "ast_visitor.h"


static void do_visit_ast(ast_node_t* node, void (*callback)(ast_node_t*)) {

    if (node == NULL) return;

    callback(node);

    if (node->type == NODE_ROOT) {
        do_visit_ast(node->as.root.stmts, callback);
    }
    else if (node->type == NODE_BINOP) {
        do_visit_ast(node->as.binary.left, callback);
        do_visit_ast(node->as.binary.right, callback);
    }
    else if (node->type == NODE_UNARYOP) {
        do_visit_ast(node->as.unary.expr, callback);
    }
    else if (node->type == NODE_INT) {
    }
    else if (node->type == NODE_CHAR) {
    }
    else if (node->type == NODE_IDENT) {
    }
    else if (node->type == NODE_STRING) {
    }
    else if (node->type == NODE_FUNCCALL) {
        do_visit_ast(node->as.funccall.ident, callback);
        do_visit_ast(node->as.funccall.params, callback);
    }
    else if (node->type == NODE_PARAM_LIST) {
        ast_node_t* param = node->as.paramslist.list->head;
        while (param) {
            do_visit_ast(param, callback);
            param = param->next;
        }
    }
    else if (node->type == NODE_PARAMDECL_LIST) {
        ast_node_t* param = node->as.paramsdecllist.list->head;
        while (param) {
            do_visit_ast(param, callback);
            param = param->next;
        }
    }
    else if (node->type == NODE_IF) {
        do_visit_ast(node->as.ifstmt.cond, callback);
        do_visit_ast(node->as.ifstmt._if, callback);
        do_visit_ast(node->as.ifstmt._else, callback);
    }
    else if (node->type == NODE_WHILE) {
        do_visit_ast(node->as.whilestmt.cond, callback);
        do_visit_ast(node->as.whilestmt.stmts, callback);
    }
    else if (node->type == NODE_FOR) {
        do_visit_ast(node->as.forstmt.init, callback);
        do_visit_ast(node->as.forstmt.cond, callback);
        do_visit_ast(node->as.forstmt.incr, callback);
        do_visit_ast(node->as.forstmt.stmts, callback);
    }
    else if (node->type == NODE_FUNCDECL) {
        do_visit_ast(node->as.funcdecl.ident, callback);
        do_visit_ast(node->as.funcdecl.params, callback);
        do_visit_ast(node->as.funcdecl.stmts, callback);
    }
    else if (node->type == NODE_RETURN) {
        do_visit_ast(node->as._return.expr, callback);
    }
    else if (node->type == NODE_STMTSLIST) {
        ast_node_t* stmt = node->as.stmtslist.list->head;
        while (stmt) {
            do_visit_ast(stmt, callback);
            stmt = stmt->next;
        }
    }
    else if (node->type == NODE_ASSIGN) {
        do_visit_ast(node->as.assign.left, callback);
        do_visit_ast(node->as.assign.right, callback);
    }
    else if (node->type == NODE_ARRAYACCESS) {
        do_visit_ast(node->as.arrayaccess.ident, callback);
        do_visit_ast(node->as.arrayaccess.expr, callback);
    }
    else if (node->type == NODE_PARAMDECL) {
        do_visit_ast(node->as.paramdecl.ident, callback);
    }
    else if (node->type == NODE_VARDECL) {
        do_visit_ast(node->as.vardecl.ident, callback);
    }
}

void visit_ast(ast_node_t* node, void (*callback)(ast_node_t*)) {
    do_visit_ast(node, callback);
}

