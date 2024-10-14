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

SUITE(eval_suite) {
    RUN_TEST(eval_test_int_expr);
    RUN_TEST(eval_test_bool_expr);
    RUN_TEST(eval_test_bang_operator);
}
