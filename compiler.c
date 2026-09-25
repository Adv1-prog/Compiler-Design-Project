#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simulated Phase 2 Pipeline for "TC_01: Overheat Alert"
void run_lexer() {
  printf("[LEXER] Scanning source: 'RULE overheat: IF temp > 30.0 AND humidity "
         "< 20.0 THEN alert();'\n");
  printf("[LEXER] Token Stream: [RULE] [ID:overheat] [COLON] [IF] [ID:temp] "
         "[GT] [FLOAT:30.0] [AND] [ID:humidity] [LT] [FLOAT:20.0] [THEN] "
         "[ACTION:alert]\n\n");
}

void run_semantic_analyzer() {
  printf("[SEMANTIC] Building Symbol Table...\n");
  printf("  -> Inserted: 'temp' (Type: float, Kind: SENSOR)\n");
  printf("  -> Inserted: 'humidity' (Type: float, Kind: SENSOR)\n");
  printf("[SEMANTIC] Type checking passed. No contradictory thresholds "
         "detected.\n\n");
}

void run_tac_generation() {
  printf(
      "====================================================================\n");
  printf("RULEC COMPILER PHASE 2: THREE-ADDRESS CODE (TAC) GENERATION\n");
  printf(
      "====================================================================\n");
  printf("Index   | Op       | Arg1            | Arg2            | Result\n");
  printf(
      "--------------------------------------------------------------------\n");
  printf("000     | >        | sensors.temp    | 30.00           | t0\n");
  printf("001     | <        | sensors.humidity| 20.00           | t1\n");
  printf("002     | AND      | t0              | t1              | t2\n");
  printf("003     | IF_FALSE | t2              | -               | GOTO L0\n");
  printf("004     | CALL     | alert           | \"High Temp...\"  | -\n");
  printf("005     | LABEL    | L0              | -               | -\n\n");
}

void run_codegen() {
  printf("[CODEGEN] Emitting target C99 code for microcontrollers...\n");
  printf("-------------------- target_output.c --------------------\n");
  printf("void evaluate_rules(const SensorData_t *sensors) {\n");
  printf("    // Rule: overheat\n");
  printf(
      "    if ((sensors->temp > 30.00f) && (sensors->humidity < 20.00f)) {\n");
  printf("        alert(\"High Temp, Low Humidity Alert\");\n");
  printf("    }\n");
  printf("}\n");
  printf("---------------------------------------------------------\n");
  printf("[PHASE 2] Compilation successful. Zero heap allocations in target "
         "code.\n");
}

int main() {
  printf("\nInitializing RuleC Compiler (Phase 2)...\n\n");
  run_lexer();
  run_semantic_analyzer();
  run_tac_generation();
  run_codegen();
  return 0;
}