#pragma once
#include "ast.h"
#include "object_env.h"
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

        struct {
            struct ast_Expr **params; // only identifiers
            struct ast_Stmt *body; // only block stmts
            obj_Env *env;
        } m_func;

        sstring m_str;
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
            obj->m_return_obj = NULL;
            break;
        case obj_FUNCTION:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_FUNCTION;
            obj->m_func.params = NULL;
            obj->m_func.body = NULL;
            obj->m_func.env = NULL;
            break;
        case obj_STRING:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_STRING;
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
        case obj_STRING:
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
        case obj_FUNCTION:
            for (int i = 0; i < stbds_arrlen(obj->m_func.params); ++i) {
                ast_free_expr(obj->m_func.params[i]);
            }
            stbds_arrfree(obj->m_func.params);
            ast_free_stmt(obj->m_func.body);
            obj_free_env(obj->m_func.env);
            free(obj);
            break;
        default:
            assert(0 && "unreachable");
    }
}

obj_Object *obj_deepcpy(obj_Object *src) {
    if (src == NULL)
        return NULL;

    obj_Object *dest = malloc(sizeof(obj_Object));
    memcpy(dest, src, sizeof(obj_Object));

    switch (src->type) {
        case obj_INTEGER:
        case obj_BOOLEAN:
        case obj_ERROR:
            // NOTHING - since no deep ptrs
            break;
        case obj_RETURN_VALUE:
            dest->m_return_obj = obj_deepcpy(src->m_return_obj);
            break;
        case obj_FUNCTION:
            dest->m_func.params = ast_deepcpy_fn_params(src->m_func.params);
            dest->m_func.body = ast_deepcopy_stmt(src->m_func.body);
            dest->m_func.env = (src->m_func.env);
            break;
        default:
            assert(0 && "unreachable");
    }
    return dest;
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
    if (obj == NULL)
        return NULL;
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
        case obj_FUNCTION:
            res = gb_append_cstring(res, "fn(");
            for (int i = 0; i < stbds_arrlen(obj->m_func.params); ++i) {
                struct ast_Expr *param = obj->m_func.params[i];
                assert(param->tag == ast_IDENT_EXPR);
                res = gb_append_cstring(res, param->data.ident.value);
                if (i == stbds_arrlen(obj->m_func.params) - 1) {
                    break;
                }
                res = gb_append_cstring(res, ", ");
            }
            res = gb_append_cstring(res, ") {\n");
            res = gb_append_cstring(res, ast_make_stmt_str(obj->m_func.body));
            res = gb_append_cstring(res, "\n}");
            break;
        case obj_STRING:
            res = gb_append_string(res, obj->m_str);
            break;
        default:
            assert(0 && "unreachable");
    }
    return res;
}
