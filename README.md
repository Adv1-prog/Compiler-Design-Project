# RuleC — Phase 1 Prototype

A working compiler for **RuleC**, a small DSL for expressing IoT sensor trigger
rules, that generates portable C code.

## Pipeline

```
.rc source --> Lexer --> Parser (AST) --> Semantic Analyzer --> C Code Generator --> evaluate_rules.c/.h
```

## Files

```
src/compiler.py        The RuleC compiler (lexer, parser, semantic analyzer, codegen)
src/test_harness.c     Demo driver: mock sensor data + action implementations
examples/overheat.rc   Sample RuleC program (3 sensors, 3 rules)
examples/bad_contradiction.rc   Deliberately invalid program (triggers a semantic error)
generated/             Output directory (compiler output + compiled demo binary)
```

## Running it

1. **Compile a RuleC program to C:**
   ```
   python3 src/compiler.py examples/overheat.rc -o generated
   ```
   This prints each compiler phase (lexing, parsing, semantic analysis, codegen)
   and writes `generated/evaluate_rules.c` + `generated/evaluate_rules.h`.

2. **See semantic error detection in action** (contradictory rule):
   ```
   python3 src/compiler.py examples/bad_contradiction.rc -o /tmp/bad_out
   ```
   This fails at the semantic-analysis phase with a clear diagnostic — it never
   reaches code generation.

3. **Build and run the generated code against mock sensor data:**
   ```
   gcc -Wall -o generated/rulec_demo src/test_harness.c generated/evaluate_rules.c -Igenerated
   ./generated/rulec_demo
   ```
   This feeds 5 test cases (hot/dry, comfortable, motion+safe, motion+too-hot,
   cold) through the compiler-generated `evaluate_rules()` and prints which
   rules fire — confirming the generated C behaves exactly as the RuleC source
   specifies, including correctly *not* firing rules whose conditions aren't met.

## What this demonstrates (mapped to Phase 1 objectives)

| Objective | Where |
|---|---|
| Grammar for sensor rules | `examples/overheat.rc`, grammar implemented in `Parser` class |
| Lexer + parser → AST | `tokenize()` + `Parser` in `compiler.py` |
| Semantic analysis (types, undeclared sensors, contradictions) | `SemanticAnalyzer` class |
| C code generation | `CodeGenerator` class |
| Working prototype | `generated/evaluate_rules.c`, compiled and run via `test_harness.c` |

## Requirements

- Python 3 (no external packages)
- `gcc` (or any C99 compiler) to build the generated code
