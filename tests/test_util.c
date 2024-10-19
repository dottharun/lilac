#pragma once
#include <stdbool.h>

enum test_expected_type {
    TEST_INT,
    TEST_STRING,
    TEST_BOOL
};

typedef struct {
    enum test_expected_type type;

    union {
        char *str;
        int num;
        bool flag;
    };
} expec_u;
