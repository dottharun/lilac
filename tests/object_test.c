#include "greatest.h"

#include "../src/object.c"

SUITE(obj_suite);

TEST test_string_is_same(void) {
    // Create test strings
    obj_Object hello1 = { .type = obj_STRING, .m_str = "Hello World" };
    obj_Object hello2 = { .type = obj_STRING, .m_str = "Hello World" };
    obj_Object diff1 = { .type = obj_STRING, .m_str = "My name is johnny" };
    obj_Object diff2 = { .type = obj_STRING, .m_str = "My name is johnny" };

    // Same content strings should be equal
    ASSERT(obj_is_same(&hello1, &hello2));
    ASSERT(obj_is_same(&diff1, &diff2));

    // Different content strings should not be equal
    ASSERT_FALSE(obj_is_same(&hello1, &diff1));

    PASS();
}

TEST test_boolean_is_same(void) {
    // Create test booleans
    obj_Object true1 = { .type = obj_BOOLEAN, .m_bool = true };
    obj_Object true2 = { .type = obj_BOOLEAN, .m_bool = true };
    obj_Object false1 = { .type = obj_BOOLEAN, .m_bool = false };
    obj_Object false2 = { .type = obj_BOOLEAN, .m_bool = false };

    // Same boolean values should be equal
    ASSERT(obj_is_same(&true1, &true2));
    ASSERT(obj_is_same(&false1, &false2));

    // Different boolean values should not be equal
    ASSERT_FALSE(obj_is_same(&true1, &false1));

    PASS();
}

TEST test_integer_is_same(void) {
    // Create test integers
    obj_Object one1 = { .type = obj_INTEGER, .m_int = 1 };
    obj_Object one2 = { .type = obj_INTEGER, .m_int = 1 };
    obj_Object two1 = { .type = obj_INTEGER, .m_int = 2 };
    obj_Object two2 = { .type = obj_INTEGER, .m_int = 2 };

    // Same integer values should be equal
    ASSERT(obj_is_same(&one1, &one2));
    ASSERT(obj_is_same(&two1, &two2));

    // Different integer values should not be equal
    ASSERT_FALSE(obj_is_same(&one1, &two1));

    PASS();
}

TEST test_array_is_same(void) {
    // Initialize arrays as NULL
    obj_Object **arr1_elements = NULL;
    obj_Object **arr2_elements = NULL;
    obj_Object **arr3_elements = NULL;

    // Create test values
    obj_Object *val1 = &(obj_Object){ .type = obj_INTEGER, .m_int = 1 };
    obj_Object *val2 = &(obj_Object){ .type = obj_INTEGER, .m_int = 2 };
    obj_Object *val3 = &(obj_Object){ .type = obj_INTEGER, .m_int = 3 };
    obj_Object *val4 = &(obj_Object){ .type = obj_INTEGER, .m_int = 4 };

    // Build first two identical arrays
    stbds_arrput(arr1_elements, val1);
    stbds_arrput(arr1_elements, val2);

    stbds_arrput(arr2_elements, val1);
    stbds_arrput(arr2_elements, val2);

    // Build third array with different values
    stbds_arrput(arr3_elements, val3);
    stbds_arrput(arr3_elements, val4);

    // Create array objects
    obj_Object arr1 = { .type = obj_ARRAY, .m_arr_da = arr1_elements };
    obj_Object arr2 = { .type = obj_ARRAY, .m_arr_da = arr2_elements };
    obj_Object arr3 = { .type = obj_ARRAY, .m_arr_da = arr3_elements };

    // Same content arrays should be equal
    ASSERT(obj_is_same(&arr1, &arr2));

    // Different content arrays should not be equal
    ASSERT_FALSE(obj_is_same(&arr1, &arr3));

    // Free the arrays
    stbds_arrfree(arr1_elements);
    stbds_arrfree(arr2_elements);
    stbds_arrfree(arr3_elements);

    PASS();
}

TEST test_null_and_type_comparison(void) {
    obj_Object int_obj = { .type = obj_INTEGER, .m_int = 1 };
    obj_Object bool_obj = { .type = obj_BOOLEAN, .m_bool = true };

    // NULL comparisons
    ASSERT_FALSE(obj_is_same(NULL, &int_obj));
    ASSERT_FALSE(obj_is_same(&int_obj, NULL));
    ASSERT(obj_is_same(NULL, NULL));

    // Different types should not be equal
    ASSERT_FALSE(obj_is_same(&int_obj, &bool_obj));

    PASS();
}

SUITE(obj_suite) {
    RUN_TEST(test_string_is_same);
    RUN_TEST(test_boolean_is_same);
    RUN_TEST(test_integer_is_same);
    RUN_TEST(test_array_is_same);
    RUN_TEST(test_null_and_type_comparison);
}
