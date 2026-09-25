#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "tokens.h"

typedef enum {
    AST_PROGRAM,
    AST_DECLARATION,
    AST_RULE,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_ACTION_CALL
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char name[64];
    char datatype[16];
    Token op;
    union {
        int int_val;
        double float_val;
        char str_val[64];
    } val;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next; // Linked list for declarations/rules/actions
} ASTNode;

static inline ASTNode* create_node(ASTNodeType type) {
    ASTNode *node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = type;
    return node;
}

#endif // AST_H