#include "util.c"

#include <stdio.h>

#define ENUMERATE_TOKEN_TYPES \
    __ENUMERATE_TOKEN_TYPE(tok_ILLEGAL) \
    __ENUMERATE_TOKEN_TYPE(tok_EOF) \
    __ENUMERATE_TOKEN_TYPE(tok_IDENT) \
    __ENUMERATE_TOKEN_TYPE(tok_INT) \
    __ENUMERATE_TOKEN_TYPE(tok_ASSIGN) \
    __ENUMERATE_TOKEN_TYPE(tok_PLUS) \
    __ENUMERATE_TOKEN_TYPE(tok_COMMA) \
    __ENUMERATE_TOKEN_TYPE(tok_SEMICOLON) \
    __ENUMERATE_TOKEN_TYPE(tok_LPAREN) \
    __ENUMERATE_TOKEN_TYPE(tok_RPAREN) \
    __ENUMERATE_TOKEN_TYPE(tok_LBRACE) \
    __ENUMERATE_TOKEN_TYPE(tok_RBRACE) \
    __ENUMERATE_TOKEN_TYPE(tok_FUNCTION) \
    __ENUMERATE_TOKEN_TYPE(tok_LET) \
    __ENUMERATE_TOKEN_TYPE(tok_BANG) \
    __ENUMERATE_TOKEN_TYPE(tok_SLASH) \
    __ENUMERATE_TOKEN_TYPE(tok_IF) \
    __ENUMERATE_TOKEN_TYPE(tok_ELSE) \
    __ENUMERATE_TOKEN_TYPE(tok_TRUE) \
    __ENUMERATE_TOKEN_TYPE(tok_FALSE) \
    __ENUMERATE_TOKEN_TYPE(tok_EQ) \
    __ENUMERATE_TOKEN_TYPE(tok_NOT_EQ) \
    __ENUMERATE_TOKEN_TYPE(tok_MINUS) \
    __ENUMERATE_TOKEN_TYPE(tok_ASTERISK) \
    __ENUMERATE_TOKEN_TYPE(tok_LT) \
    __ENUMERATE_TOKEN_TYPE(tok_GT) \
    __ENUMERATE_TOKEN_TYPE(tok_RETURN)

enum tok_Type {
#define __ENUMERATE_TOKEN_TYPE(token) token,
    ENUMERATE_TOKEN_TYPES
#undef __ENUMERATE_TOKEN_TYPE
};

struct tok_Token {
    enum tok_Type type;
    sstring literal;
};

// constructor
struct tok_Token tok_Token_create(enum tok_Type type, char ch) {
    struct tok_Token tok = { .type = type };
    snprintf(tok.literal, SHORT_STRING_MAXLEN, "%c", ch);
    return tok;
}

// gets an enum int and returns .to_string of the enum
const char *tok_Token_int_enum_to_str(int token) {
    switch ((enum tok_Type)token) {
#define __ENUMERATE_TOKEN_TYPE(token) \
    case token: \
        return #token;
        ENUMERATE_TOKEN_TYPES
#undef __ENUMERATE_TOKEN_TYPE
    }
    assert(0 && "should not be reached");
}
