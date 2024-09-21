#include "token.c"
#include "util.c"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

struct lex_Lexer {
    const char *input;
    int position; // current position in input (points to current char)
    int read_position; // current reading position in input (after current ch
    char ch; // current input char under examination
};

void lex_read_char(struct lex_Lexer *lexer) {
    if (lexer->read_position >= (int)strlen(lexer->input)) {
        lexer->ch = 0; // NUL ascii char
    } else {
        lexer->ch = lexer->input[lexer->read_position];
    }
    lexer->position = lexer->read_position;
    lexer->read_position++;
}

// constructor
struct lex_Lexer lex_Lexer_create(const char *input) {
    struct lex_Lexer lex = {
        .input = input,
        .position = 0,
        .read_position = 0,
        .ch = input[0],
    };
    lex_read_char(&lex);
    return lex;
}

bool lex_is_letter(char ch) {
    return isalpha(ch) || ch == '_';
}

bool lex_is_alnum(char ch) {
    return isalnum(ch);
}

void lex_read_identifier(struct lex_Lexer *lexer, sstring str) {
    int position = lexer->position;
    while (lex_is_letter(lexer->ch)) {
        lex_read_char(lexer);
    }
    int str_len = lexer->position - position;
    strncpy(str, lexer->input + position, str_len);
    str[str_len] = '\0';
}

void lex_read_number(struct lex_Lexer *lexer, sstring str) {
    int position = lexer->position;
    while (lex_is_alnum(lexer->ch)) {
        lex_read_char(lexer);
    }
    int str_len = lexer->position - position;
    strncpy(str, lexer->input + position, str_len);
    str[str_len] = '\0';
}

void lex_skip_whitespace(struct lex_Lexer *lexer) {
    while (lexer->ch == ' ' || lexer->ch == '\n' || lexer->ch == '\r' ||
           lexer->ch == '\t') {
        lex_read_char(lexer);
    }
}

struct tok_Token lex_next_token(struct lex_Lexer *lexer) {
    struct tok_Token token;

    lex_skip_whitespace(lexer);

    switch (lexer->ch) {
        case '=':
            token = tok_Token_create(tok_ASSIGN, lexer->ch);
            break;
        case ';':
            token = tok_Token_create(tok_SEMICOLON, lexer->ch);
            break;
        case '(':
            token = tok_Token_create(tok_LPAREN, lexer->ch);
            break;
        case ')':
            token = tok_Token_create(tok_RPAREN, lexer->ch);
            break;
        case ',':
            token = tok_Token_create(tok_COMMA, lexer->ch);
            break;
        case '+':
            token = tok_Token_create(tok_PLUS, lexer->ch);
            break;
        case '{':
            token = tok_Token_create(tok_LBRACE, lexer->ch);
            break;
        case '}':
            token = tok_Token_create(tok_RBRACE, lexer->ch);
            break;
        case 0:
            token.type = tok_EOF;
            strcpy(token.literal, "");
            break;
        default:
            if (lex_is_letter(lexer->ch)) {
                lex_read_identifier(lexer, token.literal);
                token.type = tok_lookup_identifier(token.literal);
                return token;
            } else if (lex_is_alnum(lexer->ch)) {
                lex_read_number(lexer, token.literal);
                token.type = tok_INT;
                return token;
            } else {
                token = tok_Token_create(tok_ILLEGAL, lexer->ch);
            }

    }

    lex_read_char(lexer);
    return token;
}
