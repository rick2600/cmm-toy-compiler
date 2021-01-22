#include "analyzer.h"
#include "ast_visitor.h"
#include "sym_table.h"


sym_table_t* g_sym_table;
char* g_current_func;
bool error;

/*
static print_error(uint32_t line, const char* msg) {
    fprintf(stderr, "Line: %d: error: %s is not a function\n", sym);
}
*/

static bool is_callable(ast_node_t* node) {
    bool status = false;
    char* sym = node->as.funccall.ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(g_sym_table, sym);
    if (entry != NULL) {
        if (entry->type != SYM_FUNC) {
            fprintf(stderr, "Line: %d: error: \"%s\" is not a function\n",
                node->line, sym);
            status = false;
        }
        status = true;
    }
    return status;
}

static bool number_of_params_match(ast_node_t *node) {
    bool status = true;
    char* sym = node->as.funccall.ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(g_sym_table, sym);
     if (entry != NULL) {
        ast_node_t* params = node->as.funccall.params;
        ast_node_t* param = params->as.paramslist.list->head;
        int count = 0;
        while (param) {
            param = param->next;
            count++;
        }
        if (count != entry->as.func.n_params) {
            fprintf(stderr,
                "Line: %d: error: wrong number of params for \"%s\", expected %d given %d\n",
                node->line, sym, entry->as.func.n_params, count);
            status = false;
        }
    }
    return status;
}

sym_entry_t* param_entry(char* func_name, ast_node_t* node) {
    sym_entry_t* entry = NULL;
    sym_entry_t* func_entry = sym_lookup(g_sym_table, func_name);
    if (func_entry != NULL) {
        entry = sym_lookup(func_entry->as.func.sym_table, node->as.ident.value);
        if (entry == NULL) {
            entry = sym_lookup(g_sym_table, node->as.ident.value);
        }
    }
    return entry;
}

static char *param_to_str(sym_entry_t* entry) {
    if (entry->as.var.type == TYPE_INT) {
        if (entry->as.var.is_array == true)
            return "int[]";
        else
            return "int";
    }
    if (entry->as.var.type == TYPE_CHAR) {
        if (entry->as.var.is_array == true)
            return "char[]";
        else
            return "char";
    }
    return "unkown";
}

static bool params_match(ast_node_t *node) {
    bool status = true;
    char* sym = node->as.funccall.ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(g_sym_table, sym);
    if (entry != NULL) {
        ast_node_t* params = node->as.funccall.params;
        ast_node_t* param = params->as.paramslist.list->head;

        int i = 0;
        while (param) {
            if (param->type != NODE_IDENT) { // Only checking ident for now
                param = param->next;
                continue;
            }

            sym_entry_t* given = param_entry(g_current_func, param);
            sym_entry_t* expected = entry->as.func.params[i];

            if ((given->as.var.is_array != expected->as.var.is_array)) {
                fprintf(stderr,
                "Line: %d: error: parameter mismatch for \"%s\", expected %s given %s\n",
                node->line, sym, param_to_str(expected), param_to_str(given));
                return false;
            }

            if ((given->as.var.is_array == expected->as.var.is_array) &&
                (given->as.var.type != expected->as.var.type)) {
                    fprintf(stderr,
                    "Line: %d: error: parameter mismatch for \"%s\", expected %s given %s\n",
                    node->line, sym, param_to_str(expected), param_to_str(given));
                    return false;
            }

            param = param->next;
            i++;
        }

    }

    return status;
}

static void analyze_funccall(ast_node_t* node) {
    if (is_callable(node) == false) {
        error = true;
        return;
    }
    if (number_of_params_match(node) == false) {
        error = true;
        return;
    }
    if (params_match(node) == false) {
        error = true;
        return;
    }
}

static void start_funccall_analysis(ast_node_t* node) {
    if (node->type == NODE_FUNCCALL) {
        analyze_funccall(node);
    }
}

void start_analyze_funcdecl(ast_node_t* node) {
    g_current_func = node->as.funcdecl.ident->as.ident.value;
    visit_ast(node, start_funccall_analysis);
}

void start_analysis_callback(ast_node_t* node) {
    if (node->type == NODE_FUNCDECL) {
        start_analyze_funcdecl(node);
    }
}

bool has_semantic_errors(ast_node_t* ast, sym_table_t *sym_table) {
    error = false;
    g_sym_table = sym_table;
    visit_ast(ast, start_analysis_callback);
    return error;
}

