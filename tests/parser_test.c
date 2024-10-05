#include "greatest.h"

#include "../src/parser.c"

SUITE(parser_suite);

bool test_let_statement(struct ast_Stmt *statement, const char *name) {
    ASSERT_STR_EQ(statement->token.literal, "let");

    const struct ast_Stmt *let_stmt = (struct ast_Stmt *)statement;
    ASSERT_NEQ(let_stmt, NULL);
    ASSERT_EQ(let_stmt->tag, ast_LET_STMT);
    ASSERT_EQ(let_stmt->token.type, tok_LET);

    struct ast_Expr *ident = let_stmt->data.let.name;
    ASSERT_EQ(ident->tag, ast_IDENT_EXPR);
    ASSERT_EQ(ident->data.ident.value, name);
    ASSERT_EQ(ident->token.literal, name);

    return true;
}

bool check_parser_errors(struct par_Parser *parser) {
    char **errors_da = par_parser_errors(parser);
    int n = stbds_arrlen(errors_da);

    if (n == 0) {
        return false;
    }

    printf("Parser has %d Errors.\n", (int)n);
    for (int i = 0; i < n; ++i) {
        char *msg = errors_da[i];
        printf("%s.\n", msg);
    }
    return true;
}

TEST parser_test_let_statement(void) {
    char input[] = "\
let x = 5;\
let y = 10;\
let foobar = 838383;";

    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);

    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    ASSERT(check_parser_errors(parser) == false);
    ASSERT(program != NULL);
    ASSERT_EQ_FMT((size_t)3, stbds_arrlen(program->statement_ptrs_da), "%lu");

    char *expected_identifiers[] = { "x", "y", "foobar" };
    const size_t n =
        sizeof(expected_identifiers) / sizeof(expected_identifiers[0]);

    for (size_t i = 0; i < n; i++) {
        const char *expected_identifier = expected_identifiers[i];
        struct ast_Stmt *statement = program->statement_ptrs_da[i];
        ASSERT(statement->tag == ast_LET_STMT);
        ASSERT_EQ(test_let_statement(statement, expected_identifier), true);
    }

    par_free_parser(parser);
    ast_free_program(program);

    PASS();
}

TEST parser_test_ret_statement(void) {
    char input[] = "\
return 5;\
return 10;\
return 993322;\
";
    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);

    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    ASSERT(check_parser_errors(parser) == false);
    ASSERT(program != NULL);

    int n = stbds_arrlen(program->statement_ptrs_da);
    ASSERT_EQ_FMT(3, n, "Received no of statement_ptrs: %d");

    for (int i = 0; i < 3; i++) {
        struct ast_Stmt *stmt =
            (struct ast_Stmt *)program->statement_ptrs_da[i];
        ASSERT(stmt != NULL);
        ASSERT(stmt->tag == ast_RET_STMT);
        ASSERT_STR_EQ("return", stmt->token.literal);
    }

    par_free_parser(parser);
    ast_free_program(program);

    PASS();
}

SUITE(parser_suite) {
    RUN_TEST(parser_test_let_statement);
    RUN_TEST(parser_test_ret_statement);
}
