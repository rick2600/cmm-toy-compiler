#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sym_table.h"
#include "ast.h"


static uint32_t hash(char *str) {
    uint32_t hash = 5381;
    int c;
    while (c = *str++) hash = ((hash << 5) + hash) + c;
    return hash;
}

sym_entry_t* sym_lookup(sym_table_t* scope, char *sym) {
    uint32_t pos = hash(sym) % MAX_ENTRIES;
    sym_entry_t* entry = scope->entries[pos];

    while (entry) {
        if (!strcmp(sym, entry->sym)) return entry;
        entry = entry->next;
    }
    return NULL;
}

static sym_entry_t* create_sym_entry(char* sym, sym_type_t type) {
    sym_entry_t* entry = malloc(sizeof(sym_entry_t));
    if (entry == NULL) {
        fprintf(stderr, "Could not allocate memory for sym_entry\n");
        exit(EXIT_FAILURE);
    }
    entry->type = type;
    entry->sym = strdup(sym);
    entry->next = NULL;
    return entry;
}

sym_table_t* create_sym_table(sym_table_t* parent) {
    sym_table_t* sym_table = malloc(sizeof(sym_table_t));
    if (sym_table == NULL) {
        fprintf(stderr, "Could not allocate memory for sym_table\n");
        exit(EXIT_FAILURE);
    }
    memset(sym_table, 0, sizeof(sym_table_t));
    sym_table->parent = parent;
    return sym_table;
}

bool insert_sym_from_vardecl_node(sym_table_t* scope, ast_node_t* node) {
    ast_node_t* ident = node->as.vardecl.ident;
    char* sym =  ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(scope, sym);

    if (entry == NULL) {
        uint32_t pos = hash(sym) % MAX_ENTRIES;
        sym_entry_t* new_entry = create_sym_entry(sym, SYM_VAR);
        new_entry->line = node->line;
        new_entry->as.var.type = node->as.vardecl.type;
        new_entry->as.var.is_array = node->as.vardecl.is_array;

        new_entry->next = scope->entries[pos];
        scope->entries[pos] = new_entry;
        return true;
    } else {
        fprintf(stderr, "Line: %d: error: previous declaration of \"%s\" at line %d\n",
                node->line, sym, entry->line);
        return false;
    }
}


bool insert_sym_from_paramdecl_node(sym_table_t* scope, ast_node_t* node) {
    ast_node_t* ident = node->as.paramdecl.ident;
    char* sym =  ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(scope, sym);

    if (entry == NULL) {
        uint32_t pos = hash(sym) % MAX_ENTRIES;
        sym_entry_t* new_entry = create_sym_entry(sym, SYM_VAR);
        new_entry->line = node->line;
        new_entry->as.var.type = node->as.paramdecl.type;
        new_entry->as.var.is_array = node->as.paramdecl.is_array;

        new_entry->next = scope->entries[pos];
        scope->entries[pos] = new_entry;
        return true;
    } else {
        fprintf(stderr, "Line: %d: error: previous declaration of \"%s\" at line %d\n",
                node->line, sym, entry->line);
        return false;
    }
}

bool prev_funcdecl_match(sym_table_t* scope, sym_entry_t* entry, ast_node_t *node) {
    int n = 0;
    ast_node_t *param = node->as.funcdecl.params->as.paramsdecllist.list->head;

    if (entry->as.func.type != node->as.funcdecl.type)
        return false;

    while (param) {
        n++;
        if (n > entry->as.func.n_params)
            return false;

        sym_entry_t* param_entry = entry->as.func.params[n-1];
        if (strcmp(param_entry->sym, param->as.paramdecl.ident->as.ident.value)
            || param_entry->as.var.type != param->as.paramdecl.type
            || param_entry->as.var.is_array != param->as.paramdecl.is_array) {
                return false;
        }
        param = param->next;
    }

    return (n == entry->as.func.n_params) ? true : false;
}

static bool insert_sym_from_funcdecl_node_prototype(sym_table_t* scope, ast_node_t *node) {
    ast_node_t* ident = node->as.funcdecl.ident;
    char* sym =  ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(scope, sym);

    if (entry == NULL) {
        sym_entry_t* entry = create_sym_entry(sym, SYM_FUNC);
        entry->line = ident->line;
        entry->as.func.sym_table = create_sym_table(scope);
        entry->as.func.type = node->as.funcdecl.type;
        entry->as.func.n_params = 0;
        entry->as.func.params = NULL;
        entry->as.func.defined = false;

        uint32_t pos = hash(sym) % MAX_ENTRIES;
        entry->next = scope->entries[pos];
        scope->entries[pos] = entry;

        ast_node_t *param = node->as.funcdecl.params->as.paramsdecllist.list->head;
        while (param) {
            insert_sym_from_paramdecl_node(entry->as.func.sym_table, param);
            entry->as.func.n_params++;
            entry->as.func.params = realloc(entry->as.func.params,
                                            entry->as.func.n_params * sizeof(sym_entry_t*));

            sym_entry_t* param_sym = sym_lookup(entry->as.func.sym_table,
                                                param->as.paramdecl.ident->as.ident.value);
            entry->as.func.params[entry->as.func.n_params-1] = param_sym;

            param = param->next;
        }
        return true;
    } else {
        fprintf(stderr, "Line: %d: error: previous declaration of \"%s\" at line %d\n",
                node->line, sym, entry->line);
        return false;
    }
}

static bool insert_sym_from_funcdef_node(sym_table_t* scope, ast_node_t *node) {
    char *sym = node->as.funcdecl.ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(scope, sym);
    if (entry == NULL) {
        insert_sym_from_funcdecl_node_prototype(scope, node);
        entry = sym_lookup(scope, sym);
        entry->as.func.defined = true;
        return true;
    } else {
        if (entry->as.func.defined) {
            fprintf(stderr, "Line: %d: error: previous definition of \"%s\" at line %d\n",
                    node->line, sym, entry->line);
            return false;
        } else {
            if (prev_funcdecl_match(scope, entry, node)) {
                entry->as.func.defined = true;
                entry->line = node->line;
                return true;
            } else {
                fprintf(stderr,
                        "Line: %d: error: conflicting with previous declaration of \"%s\" at line %d\n",
                    node->line, sym, entry->line);
                return false;
            }
        }
    }
}

bool insert_sym_from_funcdecl_node(sym_table_t* scope, ast_node_t *node, bool prototype) {
    if (prototype) {
        return insert_sym_from_funcdecl_node_prototype(scope, node);
    } else {
        return insert_sym_from_funcdef_node(scope, node);
    }
}

static char* type_to_typestr(decl_type_t type) {
    if (type == TYPE_INT) return "int";
    else if (type == TYPE_CHAR) return "char";
    else if (type == TYPE_VOID) return "void";
    return "unknown";
}

static void do_show_sym_table(sym_table_t* scope, int level) {
    char fmtstr[512];

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (scope->entries[i] != NULL) {
            sym_entry_t* entry = scope->entries[i];
            while (entry) {
                if (entry->type == SYM_VAR) {
                    sprintf(fmtstr,
                            "%%-%dssym type: var  | decl type: %%-4s%%s | sym: %%-20s\n", level);
                    printf(fmtstr,
                           "",
                           type_to_typestr(entry->as.var.type),
                           entry->as.var.is_array ? "[]":"  ",
                           entry->sym);

                } else if (entry->type == SYM_FUNC) {
                    sprintf(fmtstr,
                            "%%-%dssym type: func | decl type: %%-4s   | sym: %%-20s | n_params: %%d\n", level);
                    printf(fmtstr,
                           "",
                           type_to_typestr(entry->as.func.type),
                           entry->sym,
                           entry->as.func.n_params);
                    do_show_sym_table(entry->as.func.sym_table, level+4);
                }
                entry = entry->next;
            }
        }
    }
}

void show_sym_table(sym_table_t* scope) {
    do_show_sym_table(scope, 0);
}