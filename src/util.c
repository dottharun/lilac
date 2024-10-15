#pragma once
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

// deps -------------------------

#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include "stb_ds.h"

#define GB_STRING_IMPLEMENTATION
#include "gb_string.h"

// -------------------------

#define SHORT_STRING_MAXLEN 1024

typedef char sstring[SHORT_STRING_MAXLEN];

int util_str_to_int(const char *str, int *result) {
    char *endptr;
    errno = 0;
    intmax_t value = strtoimax(str, &endptr, 10);

    if (errno == ERANGE) {
        return -1; // Overflow or underflow
    }

    if (endptr == str || *endptr != '\0') {
        return -2; // Invalid input
    }

    if (value < INT_MIN || value > INT_MAX) {
        return -1; // Out of range for int
    }

    *result = (int)value;
    return 0; // Success
}

char *util_str_deepcopy(const char *original) {
    if (original == NULL) {
        return NULL;
    }
    char *copy = (char *)malloc(strlen(original) + 1);
    if (copy == NULL) {
        return NULL;
    }
    strcpy(copy, original);
    return copy;
}

gbString util_int_to_str(int x) {
    gbString str = gb_make_string_length("", 11);
    sprintf(str, "%d", x);
    return str;
}
