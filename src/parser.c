#include "ast.c"
#include "lexer.c"
#include "token.c"

/*
# Parser: Two types - Top down & Bottom up

## Recursive Decent - going to be used in this interpreter is a Top down
parser

*/

struct par_Parser {
    struct lex_Lexer *lexer;
    char **errors_da; // dynamic arr of err_strings
    struct tok_Token curr_token;
    struct tok_Token peek_token;
};

char **par_parser_errors(struct par_Parser *parser) {
    return parser->errors_da;
}

// to add an error - when peek_token doesn't match any expectation
void par_peek_error(struct par_Parser *parser, enum tok_Type token_type) {
    char *msg = malloc(256 * sizeof(char));
    int n = sprintf(
        msg,
        // SHORT_STRING_MAXLEN,
        "Expected next token to be %s, got %s instead",
        tok_Token_int_enum_to_str((int)token_type),
        tok_Token_int_enum_to_str((int)parser->peek_token.type)
    );
    assert(n < 256 && "string capacity will be exceeded");
    stbds_arrput(parser->errors_da, msg);
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
    for (int i = 0; i < stbds_arrlen(parser->errors_da); ++i) {
        free(parser->errors_da[i]);
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
    struct ast_Stmt *ret_stmt = ast_alloc_stmt(ast_RET_STMT);
    ret_stmt->token = parser->curr_token;

    par_next_token(parser);

    // TODO: We're skipping the expressions until we encounter a semicolon
    while (!par_curr_token_is(parser, tok_SEMICOLON)) {
        par_next_token(parser);
    }
    return ret_stmt;
}

struct ast_Stmt *par_parse_statement(struct par_Parser *parser) {
    struct ast_Stmt *statement = NULL;
    switch (parser->curr_token.type) {
        case tok_LET:
            statement = (struct ast_Stmt *)par_parse_let_statement(parser);
            break;
        case tok_RETURN:
            statement = (struct ast_Stmt *)par_parse_ret_statement(parser);
            break;
        default:
            // assert(0 && "should not be reached");
            // statement = NULL;
            break;
    }

    return statement;
}

void par_parse_program(struct par_Parser *parser, struct ast_Program *program) {
    while (parser->curr_token.type != tok_EOF) {
        struct ast_Stmt *statement = par_parse_statement(parser);
        if (statement != NULL) {
            stbds_arrput(program->statement_ptrs_da, statement);
        }
        par_next_token(parser);
    }
}
