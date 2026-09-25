#!/usr/bin/env python3
"""
RuleC Compiler — Phase 1 Prototype
====================================
A minimal compiler for the RuleC DSL (IoT sensor trigger rules).

Pipeline:  .rc source --> Lexer --> Parser (AST) --> Semantic Analyzer --> C Code Generator

Usage:
    python3 compiler.py <input.rc> -o <output_dir>

Produces <output_dir>/evaluate_rules.c and <output_dir>/evaluate_rules.h
"""

import sys
import os
import re
import argparse

# ---------------------------------------------------------------------------
# 1. LEXER
# ---------------------------------------------------------------------------

KEYWORDS = {"SENSOR", "RULE", "IF", "THEN", "AND", "OR", "NOT",
            "float", "int", "bool"}

TOKEN_SPEC = [
    ("COMMENT",  r"//[^\n]*"),
    ("NUMBER",   r"\d+\.\d+|\d+"),
    ("STRING",   r'"[^"]*"'),
    ("RELOP",    r">=|<=|==|!=|>|<"),
    ("IDENT",    r"[A-Za-z_][A-Za-z_0-9]*"),
    ("COLON",    r":"),
    ("SEMI",     r";"),
    ("LPAREN",   r"\("),
    ("RPAREN",   r"\)"),
    ("COMMA",    r","),
    ("NEWLINE",  r"\n"),
    ("SKIP",     r"[ \t\r]+"),
    ("MISMATCH", r"."),
]

TOKEN_RE = re.compile("|".join(f"(?P<{name}>{pattern})" for name, pattern in TOKEN_SPEC))


class Token:
    def __init__(self, kind, value, line):
        self.kind = kind
        self.value = value
        self.line = line

    def __repr__(self):
        return f"Token({self.kind}, {self.value!r}, line={self.line})"


class LexError(Exception):
    pass


def tokenize(source: str):
    tokens = []
    line = 1
    for m in TOKEN_RE.finditer(source):
        kind = m.lastgroup
        value = m.group()
        if kind == "NEWLINE":
            line += 1
            continue
        if kind in ("SKIP", "COMMENT"):
            continue
        if kind == "MISMATCH":
            raise LexError(f"Unexpected character {value!r} at line {line}")
        if kind == "IDENT" and value in KEYWORDS:
            kind = value  # promote to keyword token
        tokens.append(Token(kind, value, line))
    tokens.append(Token("EOF", None, line))
    return tokens


# ---------------------------------------------------------------------------
# 2. AST NODES
# ---------------------------------------------------------------------------

class SensorDecl:
    def __init__(self, name, type_, line):
        self.name, self.type, self.line = name, type_, line


class Rule:
    def __init__(self, name, condition, action, line):
        self.name, self.condition, self.action, self.line = name, condition, action, line


class BinOp:      # AND / OR
    def __init__(self, op, left, right):
        self.op, self.left, self.right = op, left, right


class UnaryOp:     # NOT
    def __init__(self, op, expr):
        self.op, self.expr = op, expr


class Comparison:  # sensor RELOP literal
    def __init__(self, sensor, op, literal, line):
        self.sensor, self.op, self.literal, self.line = sensor, op, literal, line


class Action:       # name(args...)
    def __init__(self, name, args, line):
        self.name, self.args, self.line = name, args, line


class Program:
    def __init__(self, sensors, rules):
        self.sensors, self.rules = sensors, rules


# ---------------------------------------------------------------------------
# 3. PARSER (recursive descent)
# ---------------------------------------------------------------------------

class ParseError(Exception):
    pass


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self):
        return self.tokens[self.pos]

    def advance(self):
        tok = self.tokens[self.pos]
        self.pos += 1
        return tok

    def expect(self, kind):
        tok = self.peek()
        if tok.kind != kind:
            raise ParseError(f"Line {tok.line}: expected {kind}, got {tok.kind} ({tok.value!r})")
        return self.advance()

    def parse_program(self):
        sensors = []
        rules = []
        while self.peek().kind != "EOF":
            if self.peek().kind == "SENSOR":
                sensors.append(self.parse_sensor_decl())
            elif self.peek().kind == "RULE":
                rules.append(self.parse_rule())
            else:
                tok = self.peek()
                raise ParseError(f"Line {tok.line}: unexpected token {tok.kind} ({tok.value!r})")
        return Program(sensors, rules)

    def parse_sensor_decl(self):
        line = self.peek().line
        self.expect("SENSOR")
        name = self.expect("IDENT").value
        self.expect("COLON")
        type_tok = self.advance()
        if type_tok.kind not in ("float", "int", "bool"):
            raise ParseError(f"Line {type_tok.line}: expected a type (float/int/bool), got {type_tok.value!r}")
        self.expect("SEMI")
        return SensorDecl(name, type_tok.kind, line)

    def parse_rule(self):
        line = self.peek().line
        self.expect("RULE")
        name = self.expect("IDENT").value
        self.expect("COLON")
        self.expect("IF")
        condition = self.parse_or_expr()
        self.expect("THEN")
        action = self.parse_action()
        self.expect("SEMI")
        return Rule(name, condition, action, line)

    def parse_or_expr(self):
        left = self.parse_and_expr()
        while self.peek().kind == "OR":
            self.advance()
            right = self.parse_and_expr()
            left = BinOp("OR", left, right)
        return left

    def parse_and_expr(self):
        left = self.parse_not_expr()
        while self.peek().kind == "AND":
            self.advance()
            right = self.parse_not_expr()
            left = BinOp("AND", left, right)
        return left

    def parse_not_expr(self):
        if self.peek().kind == "NOT":
            self.advance()
            return UnaryOp("NOT", self.parse_not_expr())
        return self.parse_primary_cond()

    def parse_primary_cond(self):
        if self.peek().kind == "LPAREN":
            self.advance()
            expr = self.parse_or_expr()
            self.expect("RPAREN")
            return expr
        return self.parse_comparison()

    def parse_comparison(self):
        line = self.peek().line
        sensor = self.expect("IDENT").value
        op_tok = self.expect("RELOP")
        lit_tok = self.advance()
        if lit_tok.kind not in ("NUMBER", "IDENT"):
            raise ParseError(f"Line {lit_tok.line}: expected a literal after {op_tok.value}, got {lit_tok.value!r}")
        return Comparison(sensor, op_tok.value, lit_tok.value, line)

    def parse_action(self):
        line = self.peek().line
        name = self.expect("IDENT").value
        self.expect("LPAREN")
        args = []
        if self.peek().kind != "RPAREN":
            args.append(self.parse_arg())
            while self.peek().kind == "COMMA":
                self.advance()
                args.append(self.parse_arg())
        self.expect("RPAREN")
        return Action(name, args, line)

    def parse_arg(self):
        tok = self.advance()
        if tok.kind not in ("STRING", "IDENT", "NUMBER"):
            raise ParseError(f"Line {tok.line}: invalid action argument {tok.value!r}")
        return tok


# ---------------------------------------------------------------------------
# 4. SEMANTIC ANALYZER
# ---------------------------------------------------------------------------

class SemanticError(Exception):
    pass


KNOWN_ACTIONS = {"alert": 1, "log": 1, "actuate": 2}  # name -> expected arg count


class SemanticAnalyzer:
    def __init__(self, program: Program):
        self.program = program
        self.symbol_table = {}  # sensor name -> type

    def analyze(self):
        self._build_symbol_table()
        seen_rule_names = set()
        for rule in self.program.rules:
            if rule.name in seen_rule_names:
                raise SemanticError(f"Line {rule.line}: duplicate rule name '{rule.name}'")
            seen_rule_names.add(rule.name)
            self._check_condition(rule.condition)
            self._check_action(rule.action)
            self._check_contradiction(rule)
        return self.symbol_table

    def _build_symbol_table(self):
        for s in self.program.sensors:
            if s.name in self.symbol_table:
                raise SemanticError(f"Line {s.line}: sensor '{s.name}' declared more than once")
            self.symbol_table[s.name] = s.type

    def _check_condition(self, node):
        if isinstance(node, Comparison):
            if node.sensor not in self.symbol_table:
                raise SemanticError(f"Line {node.line}: undeclared sensor '{node.sensor}'")
            sensor_type = self.symbol_table[node.sensor]
            # Type check: numeric ops need numeric sensor
            if node.op in (">", "<", ">=", "<=") and sensor_type not in ("float", "int"):
                raise SemanticError(
                    f"Line {node.line}: operator '{node.op}' requires a numeric sensor, "
                    f"but '{node.sensor}' is '{sensor_type}'")
        elif isinstance(node, BinOp):
            self._check_condition(node.left)
            self._check_condition(node.right)
        elif isinstance(node, UnaryOp):
            self._check_condition(node.expr)

    def _check_action(self, action: Action):
        if action.name not in KNOWN_ACTIONS:
            raise SemanticError(
                f"Line {action.line}: unknown action '{action.name}' "
                f"(known: {', '.join(KNOWN_ACTIONS)})")
        expected = KNOWN_ACTIONS[action.name]
        if len(action.args) != expected:
            raise SemanticError(
                f"Line {action.line}: action '{action.name}' expects {expected} argument(s), "
                f"got {len(action.args)}")

    def _check_contradiction(self, rule: Rule):
        """Flags a simple, detectable class of contradictions:
        sensor > X AND sensor < Y combined directly, where X >= Y."""
        def flatten_and(node, acc):
            if isinstance(node, BinOp) and node.op == "AND":
                flatten_and(node.left, acc)
                flatten_and(node.right, acc)
            elif isinstance(node, Comparison):
                acc.append(node)

        comps = []
        flatten_and(rule.condition, comps)
        by_sensor = {}
        for c in comps:
            by_sensor.setdefault(c.sensor, []).append(c)
        for sensor, cs in by_sensor.items():
            lowers, uppers = [], []
            for c in cs:
                try:
                    val = float(c.literal)
                except ValueError:
                    continue
                if c.op in (">", ">="):
                    lowers.append(val)
                elif c.op in ("<", "<="):
                    uppers.append(val)
            if lowers and uppers and max(lowers) >= min(uppers):
                raise SemanticError(
                    f"Rule '{rule.name}': contradictory condition on '{sensor}' "
                    f"(lower bound {max(lowers)} >= upper bound {min(uppers)}) — unreachable rule")


# ---------------------------------------------------------------------------
# 5. CODE GENERATOR  (AST -> C)
# ---------------------------------------------------------------------------

C_TYPE = {"float": "float", "int": "int", "bool": "int"}


class CodeGenerator:
    def __init__(self, program: Program, symbol_table: dict):
        self.program = program
        self.symbol_table = symbol_table

    def generate_header(self):
        lines = []
        lines.append("// AUTO-GENERATED by RuleC compiler. Do not edit by hand.")
        lines.append("#ifndef EVALUATE_RULES_H")
        lines.append("#define EVALUATE_RULES_H")
        lines.append("")
        lines.append("typedef struct {")
        for s in self.program.sensors:
            lines.append(f"    {C_TYPE[s.type]} {s.name};")
        lines.append("} SensorState;")
        lines.append("")
        lines.append("// Action callbacks — implement these in your application/harness.")
        lines.append('void action_alert(const char *message);')
        lines.append('void action_log(const char *message);')
        lines.append('void action_actuate(const char *device, const char *state);')
        lines.append("")
        lines.append("// Evaluates every compiled rule against the current sensor readings.")
        lines.append("void evaluate_rules(const SensorState *s);")
        lines.append("")
        lines.append("#endif // EVALUATE_RULES_H")
        return "\n".join(lines) + "\n"

    def generate_source(self, header_name="evaluate_rules.h"):
        lines = []
        lines.append("// AUTO-GENERATED by RuleC compiler. Do not edit by hand.")
        lines.append(f'#include "{header_name}"')
        lines.append("")
        lines.append("void evaluate_rules(const SensorState *s) {")
        for rule in self.program.rules:
            lines.append(f"    // RULE: {rule.name}")
            cond_c = self._gen_condition(rule.condition)
            lines.append(f"    if ({cond_c}) {{")
            lines.append(f"        {self._gen_action(rule.action)}")
            lines.append("    }")
            lines.append("")
        lines.append("}")
        return "\n".join(lines) + "\n"

    def _gen_condition(self, node):
        if isinstance(node, Comparison):
            lit = node.literal
            # boolean/int identifiers used as RHS literal (e.g. motion == 1) pass through
            return f"(s->{node.sensor} {node.op} {lit})"
        if isinstance(node, BinOp):
            c_op = "&&" if node.op == "AND" else "||"
            return f"({self._gen_condition(node.left)} {c_op} {self._gen_condition(node.right)})"
        if isinstance(node, UnaryOp):
            return f"(!{self._gen_condition(node.expr)})"
        raise CodeGenError(f"Unknown condition node: {node}")

    def _gen_action(self, action: Action):
        def render_arg(tok):
            if tok.kind == "STRING":
                return tok.value  # already double-quoted
            elif tok.kind in ("IDENT", "NUMBER"):
                return f'"{tok.value}"'  # device/state names passed as strings for the demo
            raise CodeGenError(f"Unsupported argument token: {tok}")

        args_c = ", ".join(render_arg(a) for a in action.args)
        return f"action_{action.name}({args_c});"


class CodeGenError(Exception):
    pass


# ---------------------------------------------------------------------------
# 6. DRIVER
# ---------------------------------------------------------------------------

def compile_file(input_path, output_dir):
    with open(input_path) as f:
        source = f.read()

    print(f"[1/4] Lexing {input_path} ...")
    tokens = tokenize(source)
    print(f"      -> {len(tokens) - 1} tokens")

    print("[2/4] Parsing ...")
    program = Parser(tokens).parse_program()
    print(f"      -> {len(program.sensors)} sensor declarations, {len(program.rules)} rules")

    print("[3/4] Semantic analysis ...")
    symbol_table = SemanticAnalyzer(program).analyze()
    print(f"      -> symbol table: {symbol_table}")
    print("      -> no errors")

    print("[4/4] Code generation ...")
    gen = CodeGenerator(program, symbol_table)
    os.makedirs(output_dir, exist_ok=True)
    header_path = os.path.join(output_dir, "evaluate_rules.h")
    source_path = os.path.join(output_dir, "evaluate_rules.c")
    with open(header_path, "w") as f:
        f.write(gen.generate_header())
    with open(source_path, "w") as f:
        f.write(gen.generate_source())
    print(f"      -> wrote {header_path}")
    print(f"      -> wrote {source_path}")
    print("\nCompilation successful.")


def main():
    ap = argparse.ArgumentParser(description="RuleC compiler (Phase 1 prototype)")
    ap.add_argument("input", help="Path to .rc source file")
    ap.add_argument("-o", "--output", default="generated", help="Output directory")
    args = ap.parse_args()

    try:
        compile_file(args.input, args.output)
    except (LexError, ParseError, SemanticError, CodeGenError) as e:
        print(f"\nCOMPILE ERROR: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
