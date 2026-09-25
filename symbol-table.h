#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef enum { SYMBOL_SENSOR, SYMBOL_ACTUATOR } SymbolKind;

typedef struct Symbol {
    char name[64];
    char type[16];
    SymbolKind kind;
    int line;
    struct Symbol *next;
} Symbol;

int symbol_insert(const char *name, const char *type, SymbolKind kind, int line);
Symbol* symbol_lookup(const char *name);

#endif // SYMBOL_TABLE_H
