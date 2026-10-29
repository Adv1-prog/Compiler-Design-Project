# ============================================================
#  RuleC Compiler — Makefile
#  Usage:
#    make           → compile the RuleC source + build demo
#    make test      → run the full test suite
#    make clean     → remove generated artefacts and binaries
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
PYTHON  = python3

RC_SRC    = overheat.rc
OUT_DIR   = .
DEMO_BIN  = rulec_demo

.PHONY: all compile build test clean

all: compile build

## Step 1 – run the Python RuleC compiler to emit evaluate_rules.c/.h
compile:
	@echo "==> [RuleC] Compiling $(RC_SRC) ..."
	$(PYTHON) compiler.py $(RC_SRC) -o $(OUT_DIR)

## Step 2 – build the C demo binary from the generated files
build: compile
	@echo "==> [GCC] Building $(DEMO_BIN) ..."
	$(CC) $(CFLAGS) -o $(DEMO_BIN) test_harness.c evaluate_rules.c

## Run the demo binary (5 test cases) and the semantic-error case
test: build
	@echo ""
	@echo "============================================================"
	@echo "  TEST SUITE — Valid program (overheat.rc)"
	@echo "============================================================"
	./$(DEMO_BIN)
	@echo ""
	@echo "============================================================"
	@echo "  TEST SUITE — Semantic error detection (bad_contradiction.rc)"
	@echo "============================================================"
	@$(PYTHON) compiler.py bad_contradiction.rc -o /tmp/rulec_bad_out 2>&1 || true
	@echo ""
	@echo "All tests completed."

clean:
	@echo "==> Cleaning ..."
	rm -f $(DEMO_BIN) evaluate_rules.c evaluate_rules.h
	rm -f *.o
