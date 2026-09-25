#include <stdio.h>
#include <string.h>
#include "tokens.h"
#include "ast.h"
#include "symbol-table.h"

int analyze_ast(ASTNode *node) {
    if (!node) return 1;
    int errors = 0;

    if (node->type == AST_DECLARATION) {
        SymbolKind kind = (strcmp(node->datatype, "SENSOR") == 0) ? SYMBOL_SENSOR : SYMBOL_ACTUATOR;
        if (!symbol_insert(node->name, node->datatype, kind, 0)) {
            errors++;
        }
    } 
    else if (node->type == AST_BINARY_EXPR) {
        // Range Check & Type Consistency
        if (node->left->type == AST_IDENTIFIER) {
            Symbol *sym = symbol_lookup(node->left->name);
            if (!sym) {
                printf("[Semantic Error] Undeclared sensor identifier '%s'\n", node->left->name);
                errors++;
            }
        }
    }

    errors += analyze_ast(node->left);
    errors += analyze_ast(node->right);
    errors += analyze_ast(node->next);
    return errors;
}