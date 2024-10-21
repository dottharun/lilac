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

        enum {
            BUILTIN_LEN,
            BUILTIN_FIRST,
            BUILTIN_LAST,
            BUILTIN_REST,
            BUILTIN_PUSH,
            BUILTIN_PUTS,
        } m_builtin;

        obj_Object **m_arr_da;

        struct obj_Hash {
            struct obj_Hash_elem {
                obj_Object *key;
                obj_Object *val;
            } **hash_da;
        } m_hash;
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
        case obj_BUILTIN:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_BUILTIN;
            break;
        case obj_ARRAY:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_ARRAY;
            obj->m_arr_da = NULL;
            break;
        case obj_HASH:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_HASH;
            obj->m_hash.hash_da = NULL;
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
        case obj_BUILTIN:
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
        case obj_ARRAY:
            for (int i = 0; i < stbds_arrlen(obj->m_arr_da); ++i) {
                obj_free_object(obj->m_arr_da[i]);
            }
            stbds_arrfree(obj->m_arr_da);
            free(obj);
            break;
        case obj_HASH:
            for (int i = 0; i < stbds_arrlen(obj->m_hash.hash_da); ++i) {
                obj_free_object(obj->m_hash.hash_da[i]->key);
                obj_free_object(obj->m_hash.hash_da[i]->val);
            }
            stbds_arrfree(obj->m_hash.hash_da);
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
        case obj_STRING:
        case obj_BUILTIN:
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
        case obj_ARRAY:
            dest->m_arr_da = NULL;
            for (int i = 0; i < stbds_arrlen(src->m_arr_da); ++i) {
                stbds_arrput(dest->m_arr_da, obj_deepcpy(src->m_arr_da[i]));
            }
            break;
        case obj_HASH:
            dest->m_hash.hash_da = NULL;
            for (int i = 0; i < stbds_arrlen(src->m_hash.hash_da); ++i) {
                struct obj_Hash_elem *elem =
                    malloc(sizeof(struct ast_Hash_elem));
                elem->key = obj_deepcpy(src->m_hash.hash_da[i]->key);
                elem->val = obj_deepcpy(src->m_hash.hash_da[i]->val);
                stbds_arrput(dest->m_hash.hash_da, elem);
            }
            break;
        default:
            assert(0 && "unreachable");
    }
    return dest;
}

bool obj_is_err(obj_Object *obj) {
    return (obj == NULL ? false : (obj->type == obj_ERROR));
}

bool obj_is_same(obj_Object *a, obj_Object *b) {
    if (a == b)
        return true; // Same pointer or both NULL
    if (!a || !b)
        return false; // One is NULL
    if (a->type != b->type)
        return false; // Different types

    switch (a->type) {
        case obj_INTEGER:
            return a->m_int == b->m_int;

        case obj_BOOLEAN:
            return a->m_bool == b->m_bool;

        case obj_RETURN_VALUE:
            return obj_is_same(a->m_return_obj, b->m_return_obj);

        case obj_ERROR:
            return strcmp(a->m_err_msg, b->m_err_msg) == 0;

        case obj_FUNCTION: {
            // Functions are only equal if they're the same instance
            // since they might have different closures/environments
            return a == b;
        }

        case obj_STRING:
            return strcmp(a->m_str, b->m_str) == 0;

        case obj_BUILTIN:
            return a->m_builtin == b->m_builtin;

        case obj_ARRAY: {
            int len_a = stbds_arrlen(a->m_arr_da);
            int len_b = stbds_arrlen(b->m_arr_da);
            if (len_a != len_b)
                return false;

            for (int i = 0; i < len_a; i++) {
                if (!obj_is_same(a->m_arr_da[i], b->m_arr_da[i])) {
                    return false;
                }
            }
            return true;
        }

        case obj_HASH: {
            int len_a = stbds_arrlen(a->m_hash.hash_da);
            int len_b = stbds_arrlen(b->m_hash.hash_da);
            if (len_a != len_b)
                return false;

            // For each key-value pair in a, find matching pair in b
            for (int i = 0; i < len_a; i++) {
                bool found = false;
                for (int j = 0; j < len_b; j++) {
                    if (obj_is_same(
                            a->m_hash.hash_da[i]->key,
                            b->m_hash.hash_da[j]->key
                        ) &&
                        obj_is_same(
                            a->m_hash.hash_da[i]->val,
                            b->m_hash.hash_da[j]->val
                        )) {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
        }

        default:
            return false;
    }
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
            res = gb_append_cstring(res, util_int_to_str(obj->m_int));
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
            res = gb_append_cstring(res, obj->m_str);
            break;
        case obj_BUILTIN:
            res = gb_append_cstring(res, "builtin function");
            break;
        case obj_ARRAY:
            res = gb_append_cstring(res, "[");
            for (int i = 0; i < stbds_arrlen(obj->m_arr_da); ++i) {
                obj_Object *elem = obj->m_arr_da[i];
                res = gb_append_cstring(res, obj_object_inspect(elem));
                if (i == stbds_arrlen(obj->m_arr_da) - 1) {
                    break;
                }
                res = gb_append_cstring(res, ", ");
            }
            res = gb_append_cstring(res, "]");
            break;
        case obj_HASH:
            res = gb_append_cstring(res, "{");
            for (int i = 0; i < stbds_arrlen(obj->m_hash.hash_da); ++i) {
                gbString key = obj_object_inspect(obj->m_hash.hash_da[i]->key);
                gbString val = obj_object_inspect(obj->m_hash.hash_da[i]->val);
                res = gb_append_string(res, key);
                res = gb_append_cstring(res, ": ");
                res = gb_append_string(res, val);
                if (i == stbds_arrlen(obj->m_hash.hash_da) - 1) {
                    break;
                }
                res = gb_append_cstring(res, ", ");
            }
            res = gb_append_cstring(res, "}");
            break;
        default:
            assert(0 && "unreachable");
    }
    return res;
}

void obj_hash_put(obj_Object *obj, obj_Object *key, obj_Object *val) {
    int found_times = 0;
    int found_idx = -1;

    int n = stbds_arrlen(obj->m_hash.hash_da);
    for (int i = 0; i < n; ++i) {
        if (obj_is_same(obj->m_hash.hash_da[i]->key, key)) {
            found_times++;
            found_idx = i;
        }
    }
    assert(found_times == 0 || found_times == 1);

    if (found_times == 1) {
        obj->m_hash.hash_da[found_idx]->val = val;
    } else {
        struct obj_Hash_elem *elem = malloc(sizeof(struct ast_Hash_elem));
        elem->key = key;
        elem->val = val;
        stbds_arrput(obj->m_hash.hash_da, elem);
    }
}
