#include "greatest.h"

#include "../src/parser.c"

SUITE(parser_suite);

enum test_expected_type {
    TEST_INT,
    TEST_STRING,
    TEST_BOOL
};

typedef struct {
    enum test_expected_type type;

    union {
        sstring str;
        int num;
        bool flag;
    };
} expec_u;

bool test_identifier(struct ast_Expr *ident, const char *val) {
    assert(ident != NULL);
    assert(ident->tag == ast_IDENT_EXPR);
    assert(strcmp(ident->data.ident.value, val) == 0);
    assert(strcmp(ident->token.literal, val) == 0);
    return true;
}

bool test_int_literal(struct ast_Expr *il_expr, int val) {
    assert(il_expr->tag == ast_INT_LIT_EXPR);
    if (il_expr->data.int_lit.value != val) {
        return false;
    }

    char str[32]; // for max_int
    sprintf(str, "%d", val);
    if (strcmp(str, il_expr->token.literal) != 0) {
        return false;
    }
    return true;
}

bool test_bool_literal(struct ast_Expr *bool_expr, bool val) {
    assert(bool_expr != NULL);
    assert(bool_expr->tag == ast_BOOL_EXPR);
    assert(bool_expr->data.boolean.value == val);
    assert(strcmp(bool_expr->token.literal, (val ? "true" : "false")) == 0);
    return true;
}

bool test_lit_expr(struct ast_Expr *expr, expec_u val) {
    switch (val.type) {
        case TEST_INT:
            return test_int_literal(expr, val.num);
        case TEST_STRING:
            return test_identifier(expr, val.str);
        case TEST_BOOL:
            return test_bool_literal(expr, val.flag);
        default:
            assert(0 && "unreachable");
    }
    return true;
}

bool test_infix_expr(
    struct ast_Expr *inf_expr,
    expec_u left,
    const char *operator,
    expec_u right
) {
    assert(inf_expr->tag == ast_INFIX_EXPR);
    assert(test_lit_expr(inf_expr->data.inf.left, left));
    assert(strcmp(inf_expr->data.inf.operator, operator) == 0);
    assert(test_lit_expr(inf_expr->data.inf.right, right));
    return true;
}

bool test_let_statement(struct ast_Stmt *let_stmt, const char *name) {
    assert(let_stmt != NULL);
    assert(let_stmt->tag == ast_LET_STMT);

    assert(0 == strcmp(let_stmt->token.literal, "let"));
    assert(let_stmt->token.type == tok_LET);

    struct ast_Expr *let_ident = let_stmt->data.let.name;
    assert(let_ident->tag == ast_IDENT_EXPR);

    assert(0 == strcmp(let_ident->data.ident.value, name));

    assert(0 == strcmp(let_ident->token.literal, name));
    assert(let_ident->token.type == tok_IDENT);

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
        struct ast_Stmt *stmt = program->statement_ptrs_da[i];
        ASSERT(stmt != NULL);
        ASSERT(stmt->tag == ast_RET_STMT);
        ASSERT_STR_EQ("return", stmt->token.literal);
    }

    par_free_parser(parser);
    ast_free_program(program);

    PASS();
}

TEST parser_test_identifier_expression(void) {
    char input[] = "foobar;";

    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    ASSERT(check_parser_errors(parser) == false);
    ASSERT(program != NULL);
    ASSERT_EQ_FMT(
        1,
        (int)stbds_arrlen(program->statement_ptrs_da),
        "No of stmts - %d"
    );

    struct ast_Stmt *stmt = program->statement_ptrs_da[0];
    ASSERT(stmt != NULL);

    ASSERT(stmt->tag == ast_EXPR_STMT);
    struct ast_Expr *ident_expr = stmt->data.expr.expr;
    ASSERT(ident_expr != NULL);
    ASSERT(ident_expr->tag == ast_IDENT_EXPR);
    ASSERT_STR_EQ("foobar", ident_expr->data.ident.value);
    ASSERT_STR_EQ("foobar", ident_expr->token.literal);

    par_free_parser(parser);
    ast_free_program(program);

    PASS();
}

TEST parser_test_int_literal_expression(void) {
    char input[] = "5;";

    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    ASSERT(check_parser_errors(parser) == false);
    ASSERT(program != NULL);
    ASSERT_EQ_FMT(1, (int)stbds_arrlen(program->statement_ptrs_da), "%d");

    struct ast_Stmt *stmt = program->statement_ptrs_da[0];
    ASSERT(stmt != NULL);

    ASSERT(stmt->tag == ast_EXPR_STMT);
    struct ast_Expr *int_lit_expr = stmt->data.expr.expr;
    ASSERT(int_lit_expr != NULL);
    ASSERT(int_lit_expr->tag == ast_INT_LIT_EXPR);
    ASSERT_EQ_FMT(5, int_lit_expr->data.int_lit.value, "%d");
    ASSERT_STR_EQ("5", int_lit_expr->token.literal);

    PASS();
}

TEST parser_test_prefix_expressions(void) {
    struct {
        char *input;
        char *operator;
        expec_u val;
    } prefix_tests[] = {
        { "!5;", "!", { TEST_INT, .num = 5 } },
        { "-15;", "-", { TEST_INT, .num = 15 } },
        { "!foobar;", "!", { TEST_STRING, .str = "foobar" } },
        { "-foobar;", "-", { TEST_STRING, .str = "foobar" } },
        { "!true;", "!", { TEST_BOOL, .flag = true } },
        { "!false;", "!", { TEST_BOOL, .flag = false } },
    };

    int n = sizeof(prefix_tests) / sizeof(prefix_tests[0]);

    for (int i = 0; i < n; ++i) {
        struct lex_Lexer lexer = lex_Lexer_create(prefix_tests[i].input);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();
        par_parse_program(parser, program);

        ASSERT(check_parser_errors(parser) == false);
        ASSERT(program != NULL);
        ASSERT_EQ_FMT(1, (int)stbds_arrlen(program->statement_ptrs_da), "%d");

        struct ast_Stmt *stmt = program->statement_ptrs_da[0];
        ASSERT(stmt != NULL);

        ASSERT(stmt->tag == ast_EXPR_STMT);
        struct ast_Expr *prefix_expr = stmt->data.expr.expr;
        ASSERT(prefix_expr != NULL);
        ASSERT(prefix_expr->tag == ast_PREFIX_EXPR);
        ASSERT_STR_EQ(prefix_tests[i].operator, prefix_expr->data.pf.operator);
        ASSERT(test_lit_expr(prefix_expr->data.pf.right, prefix_tests[i].val));

        par_free_parser(parser);
        ast_free_program(program);
    }

    PASS();
}

#define INFIX_INT_CASE(inp, x, op, y) \
    { inp, { TEST_INT, { .num = x } }, op, { TEST_INT, { .num = y } } },

#define INFIX_STR_CASE(inp, x, op, y) \
    { inp, { TEST_STRING, { .str = x } }, op, { TEST_STRING, { .str = y } } },

#define INFIX_BOOL_CASE(inp, x, op, y) \
    { inp, { TEST_BOOL, { .flag = x } }, op, { TEST_BOOL, { .flag = y } } },

TEST parser_test_infix_expressions(void) {
    struct {
        char *input;
        expec_u left_val;
        char *operator;
        expec_u right_val;
    } prefix_tests[] = {
        // clang-format off
        INFIX_INT_CASE("5 + 5;", 5, "+", 5)
        INFIX_INT_CASE("5 - 5;", 5, "-", 5)
        INFIX_INT_CASE("5 * 5;", 5, "*", 5)
        INFIX_INT_CASE("5 / 5;", 5, "/", 5)
        INFIX_INT_CASE("5 > 5;", 5, ">", 5)
        INFIX_INT_CASE("5 < 5;", 5, "<", 5)
        INFIX_INT_CASE("5 == 5;", 5, "==", 5)
        INFIX_INT_CASE("5 != 5;", 5, "!=", 5)
        INFIX_STR_CASE("foobar + barfoo;", "foobar", "+", "barfoo")
        INFIX_STR_CASE("foobar - barfoo;", "foobar", "-", "barfoo")
        INFIX_STR_CASE("foobar * barfoo;", "foobar", "*", "barfoo")
        INFIX_STR_CASE("foobar / barfoo;", "foobar", "/", "barfoo")
        INFIX_STR_CASE("foobar > barfoo;", "foobar", ">", "barfoo")
        INFIX_STR_CASE("foobar < barfoo;", "foobar", "<", "barfoo")
        INFIX_STR_CASE("foobar == barfoo;", "foobar", "==", "barfoo")
        INFIX_STR_CASE("foobar != barfoo;", "foobar", "!=", "barfoo")
        INFIX_BOOL_CASE("true == true", true, "==", true)
        INFIX_BOOL_CASE("true != false", true, "!=", false)
        INFIX_BOOL_CASE("false == false", false, "==", false)
        // clang-format on
    };

    int n = sizeof(prefix_tests) / sizeof(prefix_tests[0]);

    for (int i = 0; i < n; ++i) {
        struct lex_Lexer lexer = lex_Lexer_create(prefix_tests[i].input);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();
        par_parse_program(parser, program);

        ASSERT(check_parser_errors(parser) == false);
        ASSERT(program != NULL);
        ASSERT_EQ_FMT(1, (int)stbds_arrlen(program->statement_ptrs_da), "%d");

        struct ast_Stmt *stmt = program->statement_ptrs_da[0];
        ASSERT(stmt != NULL);

        ASSERT(stmt->tag == ast_EXPR_STMT);
        struct ast_Expr *infix_expr = stmt->data.expr.expr;

        ASSERT(test_infix_expr(
            infix_expr,
            prefix_tests[i].left_val,
            prefix_tests[i].operator,
            prefix_tests[i].right_val
        ));

        par_free_parser(parser);
        ast_free_program(program);
    }
    PASS();
}

TEST parser_test_operator_precedence_parsing(void) {
    struct {
        char *input;
        char *expected;
    } tests[] = {
        { "-a * b", "((-a) * b)" },
        { "!-a", "(!(-a))" },
        { "a + b + c", "((a + b) + c)" },
        { "a + b - c", "((a + b) - c)" },
        { "a * b * c", "((a * b) * c)" },
        { "a * b / c", "((a * b) / c)" },
        { "a + b / c", "(a + (b / c))" },
        { "a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)" },
        { "3 + 4; -5 * 5", "(3 + 4)((-5) * 5)" },
        { "5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))" },
        { "5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))" },
        { "3 + 4 * 5 == 3 * 1 + 4 * 5",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))" },
        { "3 + 4 * 5 == 3 * 1 + 4 * 5",
          "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))" },
        { "true", "true" },
        { "false", "false" },
        { "3 > 5 == false", "((3 > 5) == false)" },
        { "3 < 5 == true", "((3 < 5) == true)" },
        { "1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)" },
        { "(5 + 5) * 2", "((5 + 5) * 2)" },
        { "2 / (5 + 5)", "(2 / (5 + 5))" },
        { "(5 + 5) * 2 * (5 + 5)", "(((5 + 5) * 2) * (5 + 5))" },
        { "-(5 + 5)", "(-(5 + 5))" },
        { "!(true == true)", "(!(true == true))" },
    };

    int n = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < n; ++i) {
        struct lex_Lexer lexer = lex_Lexer_create(tests[i].input);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();
        par_parse_program(parser, program);

        ASSERT(check_parser_errors(parser) == false);
        ASSERT(program != NULL);

        char *prg_str = ast_make_program_str(program);
        ASSERT_STR_EQ(tests[i].expected, prg_str);

        struct ast_Stmt *stmt = program->statement_ptrs_da[0];
        ASSERT(stmt != NULL);
        ASSERT(stmt->tag == ast_EXPR_STMT);

        par_free_parser(parser);
        ast_free_program(program);
    }
    PASS();
}

TEST parser_test_boolean_expr(void) {
    struct {
        char *input;
        expec_u expected;
    } tests[] = {
        { "true;", { TEST_BOOL, .flag = true } },
        { "false;", { TEST_BOOL, .flag = false } },
    };

    int n = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < n; ++i) {
        struct lex_Lexer lexer = lex_Lexer_create(tests[i].input);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();

        par_parse_program(parser, program);
        ASSERT(check_parser_errors(parser) == false);
        ASSERT(program != NULL);
        ASSERT_EQ_FMT(1, (int)stbds_arrlen(program->statement_ptrs_da), "%d");

        struct ast_Stmt *stmt = program->statement_ptrs_da[0];
        ASSERT(stmt != NULL);
        ASSERT(stmt->tag == ast_EXPR_STMT);

        struct ast_Expr *bool_expr = stmt->data.expr.expr;
        ASSERT(test_lit_expr(bool_expr, tests[i].expected));

        par_free_parser(parser);
        ast_free_program(program);
    }
    PASS();
}

TEST parser_test_if_expr(void) {
    char input[] = "if (x < y) { x }";

    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);

    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);
    ASSERT(check_parser_errors(parser) == false);

    ASSERT(program != NULL);
    ASSERT_EQ_FMT((size_t)1, stbds_arrlen(program->statement_ptrs_da), "%lu");

    // test cases
    struct ast_Stmt *stmt = program->statement_ptrs_da[0];
    ASSERT(stmt != NULL);
    // since we consider `if` to be an expression
    ASSERT(stmt->tag == ast_EXPR_STMT);

    struct ast_Expr *expr = stmt->data.expr.expr;
    ASSERT(expr != NULL);
    ASSERT(expr->tag == ast_IF_EXPR);

    ASSERT(test_infix_expr(
        expr->data.ife.cond,
        (expec_u){ TEST_STRING, .str = "x" },
        "<",
        (expec_u){ TEST_STRING, .str = "y" }
    ));

    ASSERT_EQ_FMT(
        (size_t)1,
        stbds_arrlen(expr->data.ife.conseq->data.block.stmts_da),
        "%lu"
    );

    struct ast_Stmt *conseq = expr->data.ife.conseq->data.block.stmts_da[0];
    ASSERT(conseq != NULL);
    ASSERT(conseq->tag == ast_EXPR_STMT);

    ASSERT(test_identifier(conseq->data.expr.expr, "x"));
    ASSERT(expr->data.ife.alt == NULL);

    par_free_parser(parser);
    ast_free_program(program);
    PASS();
}

SUITE(parser_suite) {
    RUN_TEST(parser_test_let_statement);
    RUN_TEST(parser_test_ret_statement);
    RUN_TEST(parser_test_identifier_expression);
    RUN_TEST(parser_test_int_literal_expression);
    RUN_TEST(parser_test_prefix_expressions);
    RUN_TEST(parser_test_infix_expressions);
    RUN_TEST(parser_test_operator_precedence_parsing);
    RUN_TEST(parser_test_boolean_expr);
    RUN_TEST(parser_test_if_expr);
}
