#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include "ast.h"

typedef struct {
    char op[8];
    char arg1[32];
    char arg2[32];
    char result[32];
} TACInstr;

static TACInstr tac_buffer[128];
static int tac_count = 0;
static int temp_var_count = 0;
static int label_count = 0;

char* new_temp() {
    static char temp[16];
    sprintf(temp, "t%d", temp_var_count++);
    return strdup(temp);
}

char* new_label() {
    static char lbl[16];
    sprintf(lbl, "L%d", label_count++);
    return strdup(lbl);
}

char* generate_tac_expr(ASTNode *expr) {
    if (expr->type == AST_IDENTIFIER) return expr->name;
    if (expr->type == AST_LITERAL) {
        char *val_str = malloc(16);
        if (strcmp(expr->datatype, "float") == 0) sprintf(val_str, "%.2f", expr->val.float_val);
        else sprintf(val_str, "%d", expr->val.int_val);
        return val_str;
    }

    if (expr->type == AST_BINARY_EXPR) {
        char *left = generate_tac_expr(expr->left);
        char *right = generate_tac_expr(expr->right);
        char *temp = new_temp();

        strcpy(tac_buffer[tac_count].op, expr->op.lexeme);
        strcpy(tac_buffer[tac_count].arg1, left);
        strcpy(tac_buffer[tac_count].arg2, right);
        strcpy(tac_buffer[tac_count].result, temp);
        tac_count++;

        return temp;
    }
    return "";
}