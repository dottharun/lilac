#include "greatest.h"

#include "../src/eval.c"
#include "../src/parser.c"

SUITE(eval_suite);

obj_Object *test_eval(char *input) {
    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    return eval_eval((ast_Node){ ast_NODE_PRG, .prg = program });
}

bool test_int_obj(obj_Object *obj, int expected) {
    assert(obj->type == obj_INTEGER);
    assert(obj->m_int == expected);
    return true;
}

bool test_bool_obj(obj_Object *obj, bool expected) {
    assert(obj->type == obj_BOOLEAN);
    assert(obj->m_bool == expected);
    return true;
}

bool test_null_obj(obj_Object *obj) {
    assert(obj->type == obj_NULL);
    return obj_is_same(obj, obj_null());
}

TEST eval_test_int_expr(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "5", 5 },
        { "10", 10 },
        { "-5", -5 },
        { "-10", -10 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));

        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_bool_expr(void) {
    struct {
        char *input;
        bool expected;
    } tests[] = {
        { "true", true },
        { "false", false },
        { "1 < 2", true },
        { "1 > 2", false },
        { "1 < 1", false },
        { "1 > 1", false },
        { "1 == 1", true },
        { "1 != 1", false },
        { "1 == 2", false },
        { "1 != 2", true },
        { "true == true", true },
        { "false == false", true },
        { "true == false", false },
        { "true != false", true },
        { "false != true", true },
        { "(1 < 2) == true", true },
        { "(1 < 2) == false", false },
        { "(1 > 2) == true", false },
        { "(1 > 2) == false", true },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_bool_obj(evaluated, tests[i].expected));

        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_bang_operator(void) {
    struct {
        char *input;
        bool expected;
    } tests[] = {
        { "!true", false }, { "!false", true },   { "!5", false },
        { "!!true", true }, { "!!false", false }, { "!!5", true },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_bool_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_infix_expr(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "5", 5 },
        { "10", 10 },
        { "-5", -5 },
        { "-10", -10 },
        { "5 + 5 + 5 + 5 - 10", 10 },
        { "2 * 2 * 2 * 2 * 2", 32 },
        { "-50 + 100 + -50", 0 },
        { "5 * 2 + 10", 20 },
        { "5 + 2 * 10", 25 },
        { "20 + 2 * -10", 0 },
        { "50 / 2 * 2 + 10", 60 },
        { "2 * (5 + 10)", 30 },
        { "3 * 3 * 3 + 10", 37 },
        { "3 * (3 * 3) + 10", 37 },
        { "(5 + 10 * 2 + 15 / 3) * 2 + -10", 50 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

#define nil INT32_MIN

TEST eval_test_if_else_expr(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "if (true) { 10 }", 10 },
        { "if (false) { 10 }", nil },
        { "if (1) { 10 }", 10 },
        { "if (1 < 2) { 10 }", 10 },
        { "if (1 > 2) { 10 }", nil },
        { "if (1 > 2) { 10 } else { 20 }", 20 },
        { "if (1 < 2) { 10 } else { 20 }", 10 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        if (tests[i].expected != nil) {
            ASSERT(test_int_obj(evaluated, tests[i].expected));
        } else {
            ASSERT(test_null_obj(evaluated));
        }
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_ret_stmt(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "return 10;", 10 },
        { "return 10; 9;", 10 },
        { "return 2 * 5; 9;", 10 },
        { "9; return 2 * 5; 9;", 10 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

SUITE(eval_suite) {
    RUN_TEST(eval_test_int_expr);
    RUN_TEST(eval_test_bool_expr);
    RUN_TEST(eval_test_bang_operator);
    RUN_TEST(eval_test_infix_expr);
    RUN_TEST(eval_test_if_else_expr);
    RUN_TEST(eval_test_ret_stmt);
}
