#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "tokens.h"

static const char *src;
static int pos = 0;
static int line = 1;
static int col = 1;

void lexer_init(const char *source) {
    src = source;
    pos = 0; line = 1; col = 1;
}

static char peek() { return src[pos]; }
static char advance() { col++; return src[pos++]; }

Token next_token() {
    while (peek() != '\0') {
        if (isspace(peek())) {
            if (peek() == '\n') { line++; col = 1; }
            advance();
            continue;
        }
        
        // Single-line comments
        if (peek() == '/' && src[pos + 1] == '/') {
            while (peek() != '\0' && peek() != '\n') advance();
            continue;
        }

        int start_col = col;
        
        // Identifier or Keyword
        if (isalpha(peek()) || peek() == '_') {
            char buf[64]; int len = 0;
            while (isalnum(peek()) || peek() == '_') {
                if (len < 63) buf[len++] = advance();
                else advance();
            }
            buf[len] = '\0';

            Token t; t.line = line; t.column = start_col;
            strcpy(t.lexeme, buf);

            if (strcmp(buf, "SENSOR") == 0) t.type = TOKEN_KEYWORD_SENSOR;
            else if (strcmp(buf, "ACTUATOR") == 0) t.type = TOKEN_KEYWORD_ACTUATOR;
            else if (strcmp(buf, "RULE") == 0) t.type = TOKEN_KEYWORD_RULE;
            else if (strcmp(buf, "IF") == 0) t.type = TOKEN_KEYWORD_IF;
            else if (strcmp(buf, "THEN") == 0) t.type = TOKEN_KEYWORD_THEN;
            else if (strcmp(buf, "ELSE") == 0) t.type = TOKEN_KEYWORD_ELSE;
            else if (strcmp(buf, "float") == 0) t.type = TOKEN_TYPE_FLOAT;
            else if (strcmp(buf, "int") == 0) t.type = TOKEN_TYPE_INT;
            else if (strcmp(buf, "bool") == 0) t.type = TOKEN_TYPE_BOOL;
            else if (strcmp(buf, "AND") == 0) t.type = TOKEN_OP_AND;
            else if (strcmp(buf, "OR") == 0) t.type = TOKEN_OP_OR;
            else if (strcmp(buf, "NOT") == 0) t.type = TOKEN_OP_NOT;
            else t.type = TOKEN_IDENTIFIER;
            return t;
        }

        // Numeric Literals
        if (isdigit(peek())) {
            char buf[64]; int len = 0; int is_float = 0;
            while (isdigit(peek()) || (peek() == '.' && !is_float)) {
                if (peek() == '.') is_float = 1;
                buf[len++] = advance();
            }
            buf[len] = '\0';
            Token t; t.line = line; t.column = start_col;
            strcpy(t.lexeme, buf);
            if (is_float) {
                t.type = TOKEN_FLOAT_LITERAL;
                t.value.float_val = atof(buf);
            } else {
                t.type = TOKEN_INT_LITERAL;
                t.value.int_val = atoi(buf);
            }
            return t;
        }

        // String Literals
        if (peek() == '"') {
            advance();
            char buf[64]; int len = 0;
            while (peek() != '"' && peek() != '\0') {
                buf[len++] = advance();
            }
            advance(); // consume closing "
            buf[len] = '\0';
            Token t; t.type = TOKEN_STRING_LITERAL;
            t.line = line; t.column = start_col;
            strcpy(t.lexeme, buf);
            return t;
        }

        // Operators & Punctuation
        char c = advance();
        Token t; t.line = line; t.column = start_col;
        t.lexeme[0] = c; t.lexeme[1] = '\0';

        switch (c) {
            case '>': 
                if (peek() == '=') { advance(); strcpy(t.lexeme, ">="); t.type = TOKEN_OP_GE; }
                else t.type = TOKEN_OP_GT;
                return t;
            case '<': 
                if (peek() == '=') { advance(); strcpy(t.lexeme, "<="); t.type = TOKEN_OP_LE; }
                else t.type = TOKEN_OP_LT;
                return t;
            case '=': 
                if (peek() == '=') { advance(); strcpy(t.lexeme, "=="); t.type = TOKEN_OP_EQ; }
                return t;
            case '!': 
                if (peek() == '=') { advance(); strcpy(t.lexeme, "!="); t.type = TOKEN_OP_NE; }
                return t;
            case ':': t.type = TOKEN_COLON; return t;
            case ';': t.type = TOKEN_SEMICOLON; return t;
            case '(': t.type = TOKEN_LPAREN; return t;
            case ')': t.type = TOKEN_RPAREN; return t;
            default:
                t.type = TOKEN_ERROR;
                return t;
        }
    }
    Token t; t.type = TOKEN_EOF; t.line = line; t.column = col;
    return t;
}