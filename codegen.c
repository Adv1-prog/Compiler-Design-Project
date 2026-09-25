#include <stdio.h>
#include "ast.h"

void generate_c_code(ASTNode *root, FILE *out) {
    fprintf(out, "// Auto-Generated C Code by RuleC Compiler (Phase 2 Output)\n");
    fprintf(out, "#include <stdio.h>\n#include <stdbool.h>\n\n");

    // Generate Sensor & Actuator Struct
    fprintf(out, "typedef struct {\n");
    ASTNode *curr = root;
    while (curr) {
        if (curr->type == AST_DECLARATION) {
            fprintf(out, "    %s %s;\n", curr->datatype, curr->name);
        }
        curr = curr->next;
    }
    fprintf(out, "} SensorData_t;\n\n");

    // Action Stubs
    fprintf(out, "// Action Driver Stubs\n");
    fprintf(out, "void trigger_alert(const char* msg) { printf(\"[ALERT]: %%s\\n\", msg); }\n");
    fprintf(out, "void set_actuator(const char* name, int state) { printf(\"[ACTUATE] %%s -> %%d\\n\", name, state); }\n\n");

    // Evaluation Engine
    fprintf(out, "void evaluate_rules(SensorData_t *sensors) {\n");
    curr = root;
    while (curr) {
        if (curr->type == AST_RULE) {
            fprintf(out, "    // Rule: %s\n", curr->name);
            fprintf(out, "    if (");
            // Traversal of condition tree emitting C-compatible logic
            ASTNode *cond = curr->left;
            if (cond->type == AST_BINARY_EXPR) {
                fprintf(out, "sensors->%s %s %.2f", cond->left->name, cond->op.lexeme, cond->right->val.float_val);
            }
            fprintf(out, ") {\n");
            
            // Generate Actions
            ASTNode *act = curr->right;
            while (act) {
                fprintf(out, "        trigger_alert(\"%s\");\n", act->val.str_val);
                act = act->next;
            }
            fprintf(out, "    }\n\n");
        }
        curr = curr->next;
    }
    fprintf(out, "}\n");
}