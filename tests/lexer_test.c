#include "greatest.h"

#include "../src/lexer.c"

SUITE(lexer_suite);

TEST lexer_test_next_token(void) {
    char input[] = "let five = 5;\
let ten = 10;\
\
let add = fn(x, y) {\
  x + y;\
};\
\
let result = add(five, ten);\
!-/*5;\
5 < 10 > 5;\
\
if (5 < 10) {\
	return true;\
} else {\
	return false;\
}\
\
10 == 10;\
10 != 9;\
";

    struct tok_Token expected_tokens[] = {
        { tok_LET, "let" },     { tok_IDENT, "five" },
        { tok_ASSIGN, "=" },    { tok_INT, "5" },
        { tok_SEMICOLON, ";" }, { tok_LET, "let" },
        { tok_IDENT, "ten" },   { tok_ASSIGN, "=" },
        { tok_INT, "10" },      { tok_SEMICOLON, ";" },
        { tok_LET, "let" },     { tok_IDENT, "add" },
        { tok_ASSIGN, "=" },    { tok_FUNCTION, "fn" },
        { tok_LPAREN, "(" },    { tok_IDENT, "x" },
        { tok_COMMA, "," },     { tok_IDENT, "y" },
        { tok_RPAREN, ")" },    { tok_LBRACE, "{" },
        { tok_IDENT, "x" },     { tok_PLUS, "+" },
        { tok_IDENT, "y" },     { tok_SEMICOLON, ";" },
        { tok_RBRACE, "}" },    { tok_SEMICOLON, ";" },
        { tok_LET, "let" },     { tok_IDENT, "result" },
        { tok_ASSIGN, "=" },    { tok_IDENT, "add" },
        { tok_LPAREN, "(" },    { tok_IDENT, "five" },
        { tok_COMMA, "," },     { tok_IDENT, "ten" },
        { tok_RPAREN, ")" },    { tok_SEMICOLON, ";" },
        { tok_BANG, "!" },      { tok_MINUS, "-" },
        { tok_SLASH, "/" },     { tok_ASTERISK, "*" },
        { tok_INT, "5" },       { tok_SEMICOLON, ";" },
        { tok_INT, "5" },       { tok_LT, "<" },
        { tok_INT, "10" },      { tok_GT, ">" },
        { tok_INT, "5" },       { tok_SEMICOLON, ";" },
        { tok_IF, "if" },       { tok_LPAREN, "(" },
        { tok_INT, "5" },       { tok_LT, "<" },
        { tok_INT, "10" },      { tok_RPAREN, ")" },
        { tok_LBRACE, "{" },    { tok_RETURN, "return" },
        { tok_TRUE, "true" },   { tok_SEMICOLON, ";" },
        { tok_RBRACE, "}" },    { tok_ELSE, "else" },
        { tok_LBRACE, "{" },    { tok_RETURN, "return" },
        { tok_FALSE, "false" }, { tok_SEMICOLON, ";" },
        { tok_RBRACE, "}" },    { tok_INT, "10" },
        { tok_EQ, "==" },       { tok_INT, "10" },
        { tok_SEMICOLON, ";" }, { tok_INT, "10" },
        { tok_NOT_EQ, "!=" },   { tok_INT, "9" },
        { tok_SEMICOLON, ";" }, { tok_EOF, "" },
    };
    int expected_tokens_len =
        sizeof(expected_tokens) / sizeof(expected_tokens[0]);

    struct lex_Lexer lexer = lex_Lexer_create(input);

    for (int i = 0; i < expected_tokens_len; i++) {
        struct tok_Token expected_token = expected_tokens[i];
        struct tok_Token received_token = lex_next_token(&lexer);

        ASSERT_ENUM_EQ(
            expected_token.type,
            received_token.type,
            tok_Token_int_enum_to_str
        );
        ASSERT_STR_EQ(expected_token.literal, received_token.literal);
    }

    PASS();
}

SUITE(lexer_suite) {
    RUN_TEST(lexer_test_next_token);
}
