#include "greatest.h"
#include "test_util.c"

#include "../src/eval.c"
#include "../src/object_env.c"
#include "../src/parser.c"

SUITE(eval_suite);

obj_Object *test_eval(char *input) {
    struct lex_Lexer lexer = lex_Lexer_create(input);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);
    obj_Env *env = obj_alloc_env();

    obj_Object *res =
        eval_eval((ast_Node){ ast_NODE_PRG, .prg = program }, env);
    par_free_parser(parser);
    ast_free_program(program);
    obj_free_env(env);
    return res;
}

bool test_int_obj(obj_Object *obj, int expected) {
    assert(obj != NULL);
    assert(obj->type == obj_INTEGER);
    assert(obj->m_int == expected);
    return true;
}

bool test_bool_obj(obj_Object *obj, bool expected) {
    assert(obj->type == obj_BOOLEAN);
    assert(obj->m_bool == expected);
    return true;
}

bool test_str_obj(obj_Object *obj, const char *expected) {
    assert(obj->type == obj_STRING);
    assert(0 == strcmp(obj->m_str, expected));
    return true;
}

bool test_arr_obj(obj_Object *obj, int n, int *expected) {
    assert(obj->type == obj_ARRAY);
    assert(stbds_arrlen(obj->m_arr_da) == n);
    for (int i = 0; i < n; ++i) {
        assert(obj->m_arr_da[i]->m_int == expected[i]);
    }
    return true;
}

bool test_null_obj(obj_Object *obj) {
    assert(obj->type == obj_NULL);
    return obj_is_same(obj, obj_null());
}

bool test_err_obj(obj_Object *obj, char *err_str) {
    assert(obj->type == obj_ERROR);
    return strcmp(obj->m_err_msg, err_str) == 0;
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
        {
            "   \
if (10 > 1) {   \
  if (10 > 1) { \
    return 10;  \
  }             \
  return 1;     \
}",
            10,
        },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_err_handling(void) {
    struct {
        char *input;
        char *expected;
    } tests[] = {
        {
            "5 + true;",
            "type mismatch: obj_INTEGER + obj_BOOLEAN",
        },
        {
            "5 + true; 5;",
            "type mismatch: obj_INTEGER + obj_BOOLEAN",
        },
        {
            "-true",
            "unknown operator: -obj_BOOLEAN",
        },
        {
            "true + false;",
            "unknown operator: obj_BOOLEAN + obj_BOOLEAN",
        },
        {
            "5; true + false; 5",
            "unknown operator: obj_BOOLEAN + obj_BOOLEAN",
        },
        {
            "if (10 > 1) { true + false; }",
            "unknown operator: obj_BOOLEAN + obj_BOOLEAN",
        },
        {
            "            \
if (10 > 1) {            \
  if (10 > 1) {          \
    return true + false; \
  }                      \
  return 1;              \
}",
            "unknown operator: obj_BOOLEAN + obj_BOOLEAN",
        },
        {
            "foobar",
            "identifier not found: foobar",
        },
        {
            "\"Hello\" - \"World\"",
            "unknown operator: obj_STRING - obj_STRING",
        },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_err_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_let_stmt(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "let a = 5; a;", 5 },
        { "let a = 5 * 5; a;", 25 },
        { "let a = 5; let b = a; b;", 5 },
        { "let a = 5; let b = a; let c = a + b + 5; c;", 15 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_func_obj(void) {
    char *input = "fn(x, y) { x + 2; };";

    obj_Object *evaluated = test_eval(input);
    ASSERT_EQ(2, stbds_arrlen(evaluated->m_func.params));
    ASSERT_STR_EQ("x", ast_make_expr_str(evaluated->m_func.params[0]));
    ASSERT_STR_EQ("(x + 2)", ast_make_stmt_str(evaluated->m_func.body));

    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_fn_appln(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "let identity = fn(x) { x; }; identity(5);", 5 },
        { "let identity = fn(x) { return x; }; identity(5);", 5 },
        { "let double = fn(x) { x * 2; }; double(5);", 10 },
        { "let add = fn(x, y) { x + y; }; add(5, 5);", 10 },
        { "let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20 },
        { "fn(x) { x; }(5)", 5 },
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);
        ASSERT(test_int_obj(evaluated, tests[i].expected));
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_closures(void) {
    char *input = "        \
let newAdder = fn(x) {     \
    fn(y) { x + y };       \
};                         \
let addTwo = newAdder(66); \
addTwo(22);                \
";

    obj_Object *evaluated = test_eval(input);
    ASSERT(test_int_obj(evaluated, 88));

    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_recursive_fn(void) {
    char *input = "        \
let counter = fn(x) {      \
    if (x > 3) {           \
        return 999;        \
    } else {               \
        let foobar = 9999; \
        counter(x + 1);    \
    }                      \
};                         \
counter(0);                \
";

    obj_Object *evaluated = test_eval(input);
    ASSERT(test_int_obj(evaluated, 999));
    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_str_lit(void) {
    char input[] = "\"Hello World!\";";

    obj_Object *evaluated = test_eval(input);
    ASSERT(test_str_obj(evaluated, "Hello World!"));
    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_str_concat(void) {
    char input[] = "\"Hello\" + \" \" + \"World!\"";

    obj_Object *evaluated = test_eval(input);
    ASSERT(test_str_obj(evaluated, "Hello World!"));
    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_builtin_fn(void) {
    struct {
        char *input;
        expec_u expected;
    } tests[] = {
        { "len(\"\")", { TEST_INT, .num = 0 } },
        { "len(\"four\")", { TEST_INT, .num = 4 } },
        { "len(\"hello world\")", { TEST_INT, .num = 11 } },
        { "len(1)",
          { TEST_STRING,
            .str = "argument to `len` not supported, got obj_INTEGER" } },
        { "len(\"one\", \"two\")",
          { TEST_STRING, .str = "wrong number of arguments. got=2, want=1" } },
        { "len([1, 2, 3])", { TEST_INT, .num = 3 } },
        { "len([])", { TEST_INT, .num = 0 } },

        // first function tests
        { "first([1, 2, 3])", { TEST_INT, .num = 1 } },
        { "first([])", { TEST_INT, .num = nil } },
        { "first(1)",
          { TEST_STRING,
            .str = "argument to `first` must be obj_ARRAY, got obj_INTEGER" } },

        // last function tests
        { "last([1, 2, 3])", { TEST_INT, .num = 3 } },
        { "last([])", { TEST_INT, .num = nil } },
        { "last(1)",
          { TEST_STRING,
            .str = "argument to `last` must be obj_ARRAY, got obj_INTEGER" } },

        // rest function tests
        {
            "rest([1, 2, 3])",
            { TEST_ARRAY, .arr = { .elems = (int[]){ 2, 3 }, .n = 2 } },
        },
        { "rest([])", { TEST_INT, .num = nil } },

        // push function tests
        { "push([], 1)",
          { TEST_ARRAY, .arr = { .elems = (int[]){ 1 }, .n = 1 } } },
        { "push(1, 1)",
          { TEST_STRING,
            .str = "argument to `push` must be obj_ARRAY, got obj_INTEGER" } },

    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; ++i) {
        obj_Object *evaluated = test_eval(tests[i].input);

        if (tests[i].expected.type == TEST_INT &&
            tests[i].expected.num == nil) {
            ASSERT(test_null_obj(evaluated));
        } else if (tests[i].expected.type == TEST_INT) {
            ASSERT(test_int_obj(evaluated, tests[i].expected.num));
        } else if (tests[i].expected.type == TEST_STRING) {
            ASSERT(test_err_obj(evaluated, tests[i].expected.str));
        } else if (tests[i].expected.type == TEST_ARRAY) {
            ASSERT(test_arr_obj(
                evaluated,
                tests[i].expected.arr.n,
                tests[i].expected.arr.elems
            ));
        } else {
            FAIL();
        }
        obj_free_object(evaluated);
    }
    PASS();
}

TEST eval_test_arr_lit(void) {
    char input[] = "[1, 2 * 2, 3 + 3]";

    obj_Object *evaluated = test_eval(input);
    ASSERT(test_arr_obj(evaluated, 3, (int[]){ 1, 4, 6 }));
    obj_free_object(evaluated);
    PASS();
}

TEST eval_test_arr_idx_expr(void) {
    struct {
        char *input;
        int expected;
    } tests[] = {
        { "[1, 2, 3][0]", 1 },
        { "[1, 2, 3][1]", 2 },
        { "[1, 2, 3][2]", 3 },
        { "let i = 0; [1][i];", 1 },
        { "[1, 2, 3][1 + 1];", 3 },
        { "let myArray = [1, 2, 3]; myArray[2];", 3 },
        { "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6 },
        { "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2 },
        { "[1, 2, 3][3]", nil },
        { "[1, 2, 3][-1]", nil },
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

SUITE(eval_suite) {
    RUN_TEST(eval_test_int_expr);
    RUN_TEST(eval_test_bool_expr);
    RUN_TEST(eval_test_bang_operator);
    RUN_TEST(eval_test_infix_expr);
    RUN_TEST(eval_test_if_else_expr);
    RUN_TEST(eval_test_ret_stmt);
    RUN_TEST(eval_test_err_handling);
    RUN_TEST(eval_test_let_stmt);
    RUN_TEST(eval_test_func_obj);
    RUN_TEST(eval_test_fn_appln);
    RUN_TEST(eval_test_closures);
    RUN_TEST(eval_test_recursive_fn);
    RUN_TEST(eval_test_str_lit);
    RUN_TEST(eval_test_str_concat);
    RUN_TEST(eval_test_builtin_fn);
    RUN_TEST(eval_test_arr_lit);
    RUN_TEST(eval_test_arr_idx_expr);
}
