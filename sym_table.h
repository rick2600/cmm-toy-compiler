#ifndef cmm_sym_table_h
#define cmm_sym_table_h

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"
#define MAX_ENTRIES 1024

typedef enum {
    SYM_FUNC,
    SYM_VAR,
} sym_type_t;

typedef struct sym_entry {
    struct sym_entry* next;
    sym_type_t type;
    char* sym;
    uint32_t line;

    union {
        struct {
            decl_type_t type;
            bool is_array;
        } var;

        struct {
            decl_type_t type;
            uint32_t n_params;
            struct sym_entry** params;
            struct _sym_table *sym_table;
            bool defined;
        } func;
    } as;

} sym_entry_t;


typedef struct _sym_table {
    struct _sym_table* parent;
    sym_entry_t* entries[MAX_ENTRIES];
} sym_table_t;

void show_sym_table(sym_table_t* scope);
sym_table_t* create_sym_table(sym_table_t* parent);
bool insert_sym_from_funcdecl_node(sym_table_t* scope, ast_node_t *node, bool prototype);
bool insert_sym_from_vardecl_node(sym_table_t* scope, ast_node_t* node);
sym_entry_t* sym_lookup(sym_table_t* scope, char* sym);

#endif