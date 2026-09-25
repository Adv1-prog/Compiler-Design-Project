#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol-table.h"

static Symbol *symbol_table = NULL;

int symbol_insert(const char *name, const char *type, SymbolKind kind, int line) {
    Symbol *curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            printf("[Semantic Error] Line %d: Symbol '%s' is already declared.\n", line, name);
            return 0; // Failure: Duplicate Symbol
        }
        curr = curr->next;
    }
    Symbol *sym = (Symbol*)malloc(sizeof(Symbol));
    strcpy(sym->name, name);
    strcpy(sym->type, type);
    sym->kind = kind;
    sym->line = line;
    sym->next = symbol_table;
    symbol_table = sym;
    return 1;
}

Symbol* symbol_lookup(const char *name) {
    Symbol *curr = symbol_table;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL; // Undefined symbol
}