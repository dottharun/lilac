#pragma once
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>

#define SHORT_STRING_MAXLEN 1024

typedef char sstring[SHORT_STRING_MAXLEN];

int str_to_int(const char *str, int *result) {
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
