#pragma once
#include "util.c"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define ENUMERATE_OBJECTS \
    __ENUMERATE_OBJECT(obj_INTEGER) \
    __ENUMERATE_OBJECT(obj_BOOLEAN) \
    __ENUMERATE_OBJECT(obj_NULL) \
    __ENUMERATE_OBJECT(obj_ERROR) \
    __ENUMERATE_OBJECT(obj_STRING) \
    __ENUMERATE_OBJECT(obj_RETURN_VALUE) \
    __ENUMERATE_OBJECT(obj_FUNCTION) \
    __ENUMERATE_OBJECT(obj_BUILTIN) \
    __ENUMERATE_OBJECT(obj_ARRAY) \
    __ENUMERATE_OBJECT(obj_HASH)

enum obj_Type {
#define __ENUMERATE_OBJECT(obj) obj,
    ENUMERATE_OBJECTS
#undef __ENUMERATE_OBJECT
};

const char *obj_object_name(enum obj_Type type) {
    switch (type) {
#define __ENUMERATE_OBJECT(obj) \
    case obj: \
        return #obj;
        ENUMERATE_OBJECTS
#undef __ENUMERATE_OBJECT
    }
    assert(0 && "unreachable");
}

typedef struct obj_Object {
    enum obj_Type type;

    union {
        int m_int;
        bool m_bool;
        struct obj_Object *m_return_obj;
        gbString m_err_msg;
    };
} obj_Object;

// Native objects to be referenced by ptrs
static obj_Object NULL_OBJECT = { .type = obj_NULL };
static obj_Object TRUE_OBJECT = { .type = obj_BOOLEAN, .m_bool = true };
static obj_Object FALSE_OBJECT = { .type = obj_BOOLEAN, .m_bool = false };

obj_Object *obj_null() {
    return &NULL_OBJECT;
}

obj_Object *obj_native_bool_object(bool val) {
    if (val) {
        return &TRUE_OBJECT;
    } else {
        return &FALSE_OBJECT;
    }
}

#define MAX_ERR_STRING_LEN 1024

obj_Object *obj_alloc_err_object(const char *format, ...) {
    obj_Object *err_obj = malloc(sizeof(obj_Object));
    err_obj->type = obj_ERROR;
    err_obj->m_err_msg = gb_make_string_length("", MAX_ERR_STRING_LEN);

    va_list args;
    va_start(args, format);
    vsnprintf(err_obj->m_err_msg, MAX_ERR_STRING_LEN, format, args);
    va_end(args);

    return err_obj;
}

// FIXME: use arena for correct cleanup and easy allocation
obj_Object *obj_alloc_object(enum obj_Type type) {
    obj_Object *obj = NULL;
    switch (type) {
        case obj_INTEGER:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_INTEGER;
            break;
        case obj_RETURN_VALUE:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_RETURN_VALUE;
            break;
        case obj_BOOLEAN: // should use the native objects
        case obj_ERROR: // use its own func
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

void obj_free_object(obj_Object *obj) {
    if (obj == NULL)
        return;
    switch (obj->type) {
        case obj_INTEGER:
            free(obj);
            break;
        case obj_BOOLEAN:
        case obj_NULL:
            // NOTHING since we use native obj
            break;
        case obj_RETURN_VALUE:
            obj_free_object(obj->m_return_obj);
            free(obj);
            break;
        case obj_ERROR:
            gb_free_string(obj->m_err_msg);
            free(obj);
            break;
        default:
            assert(0 && "unreachable");
    }
}

bool obj_is_err(obj_Object *obj) {
    return (obj == NULL ? false : (obj->type == obj_ERROR));
}

bool obj_is_same(obj_Object *x, obj_Object *y) {
    return (memcmp(x, y, sizeof(obj_Object)) == 0);
}

bool obj_is_truthy(obj_Object *obj) {
    if (obj_is_same(obj, &NULL_OBJECT)) {
        return false;
    } else if (obj_is_same(obj, &TRUE_OBJECT)) {
        return true;
    } else if (obj_is_same(obj, &FALSE_OBJECT)) {
        return false;
    } else {
        return true;
    }
}

gbString obj_object_inspect(obj_Object *obj) {
    gbString res = gb_make_string("");
    switch (obj->type) {
        case obj_INTEGER:
            res = gb_append_string(res, util_int_to_str(obj->m_int));
            break;
        case obj_BOOLEAN:
            res = gb_append_cstring(res, obj->m_bool ? "true" : "false");
            break;
        case obj_NULL:
            res = gb_append_cstring(res, "null");
            break;
        case obj_RETURN_VALUE:
            res = obj_object_inspect(obj->m_return_obj);
            break;
        case obj_ERROR:
            res = gb_append_cstring(res, "ERROR: ");
            res = gb_append_string(res, obj->m_err_msg);
            break;
        default:
            assert(0 && "unreachable");
    }
    return res;
}
