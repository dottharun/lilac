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

struct ast_Stmt *par_parse_statement(struct par_Parser *parser);

struct ast_Stmt *par_parse_block_stmt(struct par_Parser *parser);

struct ast_Expr **par_parse_fn_params(struct par_Parser *parser);

struct ast_Expr **
par_parse_expression_list(struct par_Parser *parser, enum tok_Type type);

bool par_curr_token_is(struct par_Parser *parser, enum tok_Type token_type);

void par_no_prefix_parsing_err(struct par_Parser *parser, enum tok_Type token);

bool par_expect_peek(struct par_Parser *parser, enum tok_Type token_type);

bool par_peek_token_is(struct par_Parser *parser, enum tok_Type token_type);
