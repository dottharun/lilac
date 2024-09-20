#include "util.c"

enum tok_Type {
    tok_ILLEGAL,
    tok_EOF,

    // Identifiers + literals
    // add, foobar, x, y, ...
    tok_IDENT,
    // 123456
    tok_INT,

    // Operators
    tok_ASSIGN,
    tok_PLUS,

    // Delimiters
    tok_COMMA,
    tok_SEMICOLON,
    tok_LPAREN,
    tok_RPAREN,
    tok_LBRACE,
    tok_RBRACE,

    // Keywords
    tok_FUNCTION,
    tok_LET,
};

struct Token {
    enum tok_Type type;
    sstring literal;
};
