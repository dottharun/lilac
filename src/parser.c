#include "parser.h"

#include "parser_tracing.c"

/*
# Parser: Two types - Top down & Bottom up

## Recursive Decent - going to be used in this interpreter is a Top down
parser

*/

/*
# Pratt Parsing
*/

/*
 * Precendence level of different tokens
 */
static const enum par_precedence precedences[] = {
    [tok_EQ] = prec_EQUALS,      [tok_NOT_EQ] = prec_EQUALS,
    [tok_LT] = prec_LESSGREATER, [tok_GT] = prec_LESSGREATER,
    [tok_PLUS] = prec_SUM,       [tok_MINUS] = prec_SUM,
    [tok_SLASH] = prec_PRODUCT,  [tok_ASTERISK] = prec_PRODUCT,
    [tok_LPAREN] = prec_CALL,    [tok_LBRACKET] = prec_INDEX,
};

bool par_is_token_in_precendences(enum tok_Type token) {
    size_t n = sizeof(precedences) / sizeof(precedences[0]);
    return ((token >= 0 && token < n) && precedences[token] != 0);
}

enum par_precedence par_peek_precedence(struct par_Parser *parser) {
    if (par_is_token_in_precendences(parser->peek_token.type)) {
        return precedences[parser->peek_token.type];
    }
    return prec_LOWEST;
}

enum par_precedence par_curr_precedence(struct par_Parser *parser) {
    if (par_is_token_in_precendences(parser->curr_token.type)) {
        return precedences[parser->curr_token.type];
    }
    return prec_LOWEST;
}

bool par_is_prefix_expr_parsable(enum tok_Type type) {
    static const bool valid_types[] = {
        [tok_IDENT] = true,    [tok_INT] = true,      [tok_STRING] = true,
        [tok_BANG] = true,     [tok_MINUS] = true,    [tok_TRUE] = true,
        [tok_FALSE] = true,    [tok_LPAREN] = true,   [tok_IF] = true,
        [tok_FUNCTION] = true, [tok_LBRACKET] = true, [tok_LBRACE] = true,
    };
    const int n = sizeof(valid_types) / sizeof(valid_types[0]);

    return (type >= 0 && type < n) ? valid_types[type] : false;
}

bool par_is_infix_expr_parsable(enum tok_Type type) {
    static const bool valid_infix_types[] = {
        [tok_PLUS] = true,     [tok_MINUS] = true, [tok_SLASH] = true,
        [tok_ASTERISK] = true, [tok_EQ] = true,    [tok_NOT_EQ] = true,
        [tok_LT] = true,       [tok_GT] = true,    [tok_LPAREN] = true,
        [tok_LBRACKET] = true,
    };
    const int n = sizeof(valid_infix_types) / sizeof(valid_infix_types[0]);

    return (type >= 0 && type < n) ? valid_infix_types[type] : false;
}

struct ast_Expr *
par_parse_prefix_expr(enum tok_Type type, struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Expr *left_expr = NULL;

    switch (type) {
        case tok_IDENT:
            left_expr = ast_alloc_expr(ast_IDENT_EXPR);
            left_expr->token = parser->curr_token;
            strcpy(left_expr->data.ident.value, parser->curr_token.literal);
            break;
        case tok_INT:
            left_expr = ast_alloc_expr(ast_INT_LIT_EXPR);
            left_expr->token = parser->curr_token;

            int err = str_to_int(
                left_expr->token.literal,
                &left_expr->data.int_lit.value
            );
            if (err != 0) {
                gbString err_str =
                    gb_make_string("String-to-Integer Conversion Error");
                stbds_arrput(parser->errors_da, err_str);
            }
            break;
        // prefix expression
        case tok_BANG:
        case tok_MINUS:
            left_expr = ast_alloc_expr(ast_PREFIX_EXPR);
            left_expr->token = parser->curr_token;
            strcpy(left_expr->data.pf.operator, parser->curr_token.literal);

            par_next_token(parser);
            left_expr->data.pf.right =
                par_parse_expression(parser, prec_PREFIX);
            break;
        case tok_TRUE:
        case tok_FALSE:
            left_expr = ast_alloc_expr(ast_BOOL_EXPR);
            left_expr->token = parser->curr_token;
            left_expr->data.boolean.value = par_curr_token_is(parser, tok_TRUE);
            break;
        // grouped expression
        case tok_LPAREN:
            par_next_token(parser);
            left_expr = par_parse_expression(parser, prec_LOWEST);

            if (!par_expect_peek(parser, tok_RPAREN)) {
                ast_free_expr(left_expr);
                left_expr = NULL;
            }
            break;
        case tok_IF:
            if (!par_expect_peek(parser, tok_LPAREN)) {
                ast_free_expr(left_expr);
                left_expr = NULL;
                break;
            }
            par_next_token(parser);

            left_expr = ast_alloc_expr(ast_IF_EXPR);
            left_expr->token = parser->curr_token;
            left_expr->data.ife.cond =
                par_parse_expression(parser, prec_LOWEST);

            if (!par_expect_peek(parser, tok_RPAREN) ||
                !par_expect_peek(parser, tok_LBRACE)) {
                ast_free_expr(left_expr);
                left_expr = NULL;
                break;
            }

            left_expr->data.ife.conseq = par_parse_block_stmt(parser);
            assert(left_expr->data.ife.conseq->tag == ast_BLOCK_STMT);

            if (par_peek_token_is(parser, tok_ELSE)) {
                par_next_token(parser);

                if (!par_expect_peek(parser, tok_LBRACE)) {
                    ast_free_expr(left_expr);
                    left_expr = NULL;
                    break;
                }

                left_expr->data.ife.alt = par_parse_block_stmt(parser);
                assert(left_expr->data.ife.alt->tag == ast_BLOCK_STMT);
            }
            break;
        case tok_FUNCTION:
            /*
             * # Function Parsing:
             * ## function structure: fn <parameters> <block statement>
             * ### parameters structure:
             *     (<parameter one>, <parameter two>, <parameter three>, ...)
             */
            left_expr = ast_alloc_expr(ast_FN_LIT_EXPR);
            left_expr->token = parser->curr_token;

            if (!par_expect_peek(parser, tok_LPAREN)) {
                ast_free_expr(left_expr);
                left_expr = NULL;
                break;
            }

            left_expr->data.fn_lit.params_da = par_parse_fn_params(parser);

            if (!par_expect_peek(parser, tok_LBRACE)) {
                ast_free_expr(left_expr);
                left_expr = NULL;
                break;
            }

            left_expr->data.fn_lit.body = par_parse_block_stmt(parser);

            assert(left_expr->data.fn_lit.body->tag == ast_BLOCK_STMT);
            break;
        default:
            par_no_prefix_parsing_err(parser, type);
            // assert(0 && "unreachable");
    }

    return left_expr;
}

struct ast_Expr *par_parse_infix_expr(
    enum tok_Type type,
    struct par_Parser *parser,
    struct ast_Expr *left_expr
) {
    TRACE_PARSER_FUNC;
    struct ast_Expr *res_left_expr = malloc(sizeof(struct ast_Expr));

    switch (type) {
        // infix expression
        case tok_PLUS:
        case tok_MINUS:
        case tok_SLASH:
        case tok_ASTERISK:
        case tok_EQ:
        case tok_NOT_EQ:
        case tok_LT:
        case tok_GT:
            res_left_expr->tag = ast_INFIX_EXPR;
            res_left_expr->token = parser->curr_token;
            strcpy(
                res_left_expr->data.inf.operator,
                parser->curr_token.literal
            );
            res_left_expr->data.inf.left = left_expr;

            enum par_precedence precedence = par_curr_precedence(parser);
            par_next_token(parser);
            res_left_expr->data.inf.right =
                par_parse_expression(parser, precedence);

            break;
        case tok_LPAREN:
            break;
        case tok_LBRACKET:
            break;
        default:
            assert(0 && "unreachable");
            break;
    }

    return res_left_expr;
}

// -----------------------------

gbString *par_parser_errors(struct par_Parser *parser) {
    return parser->errors_da;
}

// to add an error - when peek_token doesn't match any expectation
void par_peek_error(struct par_Parser *parser, enum tok_Type token_type) {
    char msg[256];
    int n = sprintf(
        msg,
        // SHORT_STRING_MAXLEN,
        "Expected next token to be %s, got %s instead",
        tok_Token_int_enum_to_str((int)token_type),
        tok_Token_int_enum_to_str((int)parser->peek_token.type)
    );
    assert(n < 256 && "string capacity will be exceeded");

    gbString err_str = gb_make_string(msg);
    stbds_arrput(parser->errors_da, err_str);
}

void par_next_token(struct par_Parser *parser) {
    parser->curr_token = parser->peek_token;
    parser->peek_token = lex_next_token(parser->lexer);
}

// constructor
struct par_Parser *par_alloc_parser(struct lex_Lexer *lexer) {
    struct par_Parser *parser = malloc(sizeof(struct par_Parser));
    parser->lexer = lexer;
    parser->errors_da = NULL;
    par_next_token(parser);
    par_next_token(parser);
    return parser;
}

void par_free_parser(struct par_Parser *parser) {
    if (parser == NULL)
        return;
    for (int i = 0; i < stbds_arrlen(parser->errors_da); ++i) {
        gb_free_string(parser->errors_da[i]);
    }
    free(parser);
}

bool par_curr_token_is(struct par_Parser *parser, enum tok_Type token_type) {
    return parser->curr_token.type == token_type;
}

bool par_peek_token_is(struct par_Parser *parser, enum tok_Type token_type) {
    return parser->peek_token.type == token_type;
}

// checks and moves if token is found
bool par_expect_peek(struct par_Parser *parser, enum tok_Type token_type) {
    if (par_peek_token_is(parser, token_type)) {
        par_next_token(parser);
        return true;
    } else {
        par_peek_error(parser, token_type);
        return false;
    }
}

struct ast_Stmt *par_parse_let_statement(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Stmt *stmt = ast_alloc_stmt(ast_LET_STMT);
    stmt->token = parser->curr_token;

    // First there should be a identifier
    if (!par_expect_peek(parser, tok_IDENT)) {
        ast_free_stmt(stmt);
        return NULL;
    }

    struct ast_Expr *let_ident = stmt->data.let.name;
    let_ident->token = parser->curr_token;
    strcpy(let_ident->data.ident.value, parser->curr_token.literal);

    // Then there should be a assign
    if (!par_expect_peek(parser, tok_ASSIGN)) {
        ast_free_stmt(stmt);
        return NULL;
    }

    // TODO: We're skipping the expressions until we encounter a semicolon
    while (!par_curr_token_is(parser, tok_SEMICOLON)) {
        par_next_token(parser);
    }

    return stmt;
}

struct ast_Stmt *par_parse_ret_statement(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Stmt *ret_stmt = ast_alloc_stmt(ast_RET_STMT);
    ret_stmt->token = parser->curr_token;

    par_next_token(parser);

    // TODO: We're skipping the expressions until we encounter a semicolon
    while (!par_curr_token_is(parser, tok_SEMICOLON)) {
        par_next_token(parser);
    }
    return ret_stmt;
}

struct ast_Expr **par_parse_fn_params(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;

    if (par_peek_token_is(parser, tok_RPAREN)) {
        par_next_token(parser);
        return NULL;
    }
    par_next_token(parser);
    // not curr_token is ident x

    struct ast_Expr **param_identifiers = NULL;

    while (!par_curr_token_is(parser, tok_RPAREN)) {
        struct ast_Expr *ident = ast_alloc_expr(ast_IDENT_EXPR);
        ident->token = parser->curr_token;
        strcpy(ident->data.ident.value, parser->curr_token.literal);

        stbds_arrput(param_identifiers, ident);
        par_next_token(parser);

        if (par_curr_token_is(parser, tok_COMMA)) {
            par_next_token(parser);
        } else if (par_curr_token_is(parser, tok_RPAREN)) {
            break;
        } else {
            for (int i = 0; i < stbds_arrlen(param_identifiers); ++i) {
                ast_free_expr(param_identifiers[i]);
            }
            stbds_arrfree(param_identifiers);
            return NULL;
        }
    }

    return param_identifiers;
}

struct ast_Stmt *par_parse_block_stmt(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Stmt *block = ast_alloc_stmt(ast_BLOCK_STMT);
    block->token = parser->curr_token;

    par_next_token(parser);

    while (!par_curr_token_is(parser, tok_RBRACE) &&
           !par_curr_token_is(parser, tok_EOF)) {
        struct ast_Stmt *stmt = par_parse_statement(parser);
        if (stmt != NULL) {
            stbds_arrput(block->data.block.stmts_da, stmt);
        }
        par_next_token(parser);
    }
    return block;
}

void par_no_prefix_parsing_err(struct par_Parser *parser, enum tok_Type token) {
    gbString msg = gb_make_string("No prefix parse function for ");
    msg = gb_append_cstring(msg, tok_Token_int_enum_to_str(token));
    msg = gb_append_cstring(msg, " found");
    stbds_arrput(parser->errors_da, msg);
}

struct ast_Expr *par_parse_expression(
    struct par_Parser *parser,
    enum par_precedence precedence
) {
    TRACE_PARSER_FUNC;
    if (!par_is_prefix_expr_parsable(parser->curr_token.type)) {
        par_no_prefix_parsing_err(parser, parser->curr_token.type);
        return NULL;
    }
    struct ast_Expr *left_expr =
        par_parse_prefix_expr(parser->curr_token.type, parser);

    while (!par_peek_token_is(parser, tok_SEMICOLON) &&
           (precedence < par_peek_precedence(parser))) {
        if (!par_is_infix_expr_parsable(parser->peek_token.type)) {
            return left_expr;
        }

        par_next_token(parser);
        left_expr =
            par_parse_infix_expr(parser->curr_token.type, parser, left_expr);
    }

    return left_expr;
}

struct ast_Stmt *par_parse_expr_statement(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Stmt *expr_stmt = ast_alloc_stmt(ast_EXPR_STMT);
    expr_stmt->token = parser->curr_token;

    expr_stmt->data.expr.expr = par_parse_expression(parser, prec_LOWEST);

    if (par_peek_token_is(parser, tok_SEMICOLON)) {
        par_next_token(parser);
    }

    return expr_stmt;
}

struct ast_Stmt *par_parse_statement(struct par_Parser *parser) {
    TRACE_PARSER_FUNC;
    struct ast_Stmt *statement = NULL;
    switch (parser->curr_token.type) {
        case tok_LET:
            statement = par_parse_let_statement(parser);
            break;
        case tok_RETURN:
            statement = par_parse_ret_statement(parser);
            break;
        default:
            statement = par_parse_expr_statement(parser);
    }

    return statement;
}

void par_parse_program(struct par_Parser *parser, struct ast_Program *program) {
    TRACE_PARSER_FUNC;
    while (parser->curr_token.type != tok_EOF) {
        struct ast_Stmt *statement = par_parse_statement(parser);
        if (statement != NULL) {
            stbds_arrput(program->statement_ptrs_da, statement);
        }
        par_next_token(parser);
    }
}
