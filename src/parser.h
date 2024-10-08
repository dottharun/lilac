#include "ast.c"
#include "lexer.c"
#include "token.c"

struct par_Parser {
    struct lex_Lexer *lexer;
    gbString *errors_da; // dynamic arr of err_strings
    struct tok_Token curr_token;
    struct tok_Token peek_token;
};

void par_next_token(struct par_Parser *);

enum par_precedence {
    prec_LOWEST = 1,
    prec_EQUALS, // ==
    prec_LESSGREATER, // > or <
    prec_SUM, // +
    prec_PRODUCT, // *
    prec_PREFIX, // -X or !X
    prec_CALL, // myFunction(X)
    prec_INDEX,
};

struct ast_Expr *
par_parse_expression(struct par_Parser *parser, enum par_precedence precedence);
