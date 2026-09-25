#ifndef TOKENS_H
#define TOKENS_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_KEYWORD_SENSOR,   // SENSOR
    TOKEN_KEYWORD_ACTUATOR, // ACTUATOR
    TOKEN_KEYWORD_RULE,     // RULE
    TOKEN_KEYWORD_IF,       // IF
    TOKEN_KEYWORD_THEN,     // THEN
    TOKEN_KEYWORD_ELSE,     // ELSE
    TOKEN_TYPE_FLOAT,       // float
    TOKEN_TYPE_INT,         // int
    TOKEN_TYPE_BOOL,        // bool
    TOKEN_IDENTIFIER,       // e.g. temp, alert_fan
    TOKEN_INT_LITERAL,      // e.g. 42
    TOKEN_FLOAT_LITERAL,    // e.g. 30.5
    TOKEN_STRING_LITERAL,   // e.g. "High Temperature Alert"
    TOKEN_OP_GT,            // >
    TOKEN_OP_LT,            // <
    TOKEN_OP_GE,            // >=
    TOKEN_OP_LE,            // <=
    TOKEN_OP_EQ,            // ==
    TOKEN_OP_NE,            // !=
    TOKEN_OP_AND,           // AND
    TOKEN_OP_OR,            // OR
    TOKEN_OP_NOT,           // NOT
    TOKEN_COLON,            // :
    TOKEN_SEMICOLON,        // ;
    TOKEN_LPAREN,           // (
    TOKEN_RPAREN,           // )
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    union {
        int int_val;
        double float_val;
    } value;
    int line;
    int column;
} Token;

#endif // TOKENS_H