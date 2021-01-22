#ifndef cmm_analyzer_h
#define cmm_analyzer_h

#include "parser.h"
#include "ast.h"
#include "sym_table.h"


bool has_semantic_errors(ast_node_t* ast, sym_table_t *sym_table);


#endif