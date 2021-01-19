#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sym_table.h"
#include "ast.h"

bool defining_a_declaration;

static uint32_t hash(char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
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
    sym_table->accepts_new_var = true;
    return sym_table;
}

bool insert_sym_from_vardecl_node(sym_table_t* scope, ast_node_t* node) {

    if (!scope->accepts_new_var)
        return true;

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

bool insert_sym_from_funcdecl_prototype_node(sym_table_t* scope, ast_node_t *node) {
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
        if (defining_a_declaration) {
            return true;
        } else {
            fprintf(stderr, "Line: %d: error: previous declaration of \"%s\" at line %d\n",
                node->line, sym, entry->line);
            return false;
        }
    }
}

bool insert_sym_from_funcdef_node(sym_table_t* scope, ast_node_t *node) {
    defining_a_declaration = true;
    char *sym = node->as.funcdecl.ident->as.ident.value;
    sym_entry_t* entry = sym_lookup(scope, sym);
    if (entry == NULL) {
        insert_sym_from_funcdecl_prototype_node(scope, node);
        entry = sym_lookup(scope, sym);
        entry->as.func.defined = true;
        defining_a_declaration = false;
        return true;
    } else {
        if (entry->as.func.defined) {
            fprintf(stderr, "Line: %d: error: previous definition of \"%s\" at line %d\n",
                node->line, sym, entry->line);
            defining_a_declaration = false;
            return false;
        } else {
            if (prev_funcdecl_match(scope, entry, node)) {
                entry->as.func.defined = true;
                entry->line = node->line;
                defining_a_declaration = false;
                return true;
            } else {
                defining_a_declaration = false;
                fprintf(stderr,
                        "Line: %d: error: conflicting with previous declaration of \"%s\" at line %d\n",
                    node->line, sym, entry->line);
                return false;
            }
        }
    }
}

static char* type_to_typestr(decl_type_t type) {
    if (type == TYPE_INT) return "int";
    else if (type == TYPE_CHAR) return "char";
    else if (type == TYPE_VOID) return "void";
    return "unknown";
}

static void show_only(sym_entry_t* entry) {
    if (entry->type == SYM_VAR) {
        printf("    variable | type: %-4s%s | sym: %-20s\n",
            type_to_typestr(entry->as.var.type),
            entry->as.var.is_array ? "[]":"  ",
            entry->sym);

    } else if (entry->type == SYM_FUNC) {
        printf("    function | type: %-4s   | sym: %-20s | n_params: %d\n",
            type_to_typestr(entry->as.func.type),
            entry->sym,
            entry->as.func.n_params);
    }
}

static void do_show_sym_table(sym_table_t* scope) {
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (scope->entries[i] != NULL) {
            sym_entry_t* entry = scope->entries[i];
            while (entry) {
                show_only(entry);
                entry = entry->next;
            }
        }
    }
    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (scope->entries[i] != NULL) {
            sym_entry_t* entry = scope->entries[i];
            while (entry) {
                if (entry->type == SYM_FUNC) {
                    puts("--------------------------------------------------------------------------------");
                    printf("Scope: %s\n", entry->sym);
                    do_show_sym_table(entry->as.func.sym_table);

                }
                entry = entry->next;
            }
        }
    }
}

void show_sym_table(sym_table_t* scope) {
    puts("================================= Symbol Table =================================");
    puts("Scope: <global>");
    do_show_sym_table(scope);
    puts("================================================================================\n");
}