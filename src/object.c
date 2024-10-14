#include "util.c"

#include <stdbool.h>
#include <stdio.h>

enum obj_Type {
    obj_INTEGER,
    obj_BOOLEAN,
    obj_NULL,
    // obj_ERROR,
    // obj_STRING,
    // obj_RETURN_VALUE,
    // obj_FUNCTION,
    // obj_BUILTIN,
    // obj_ARRAY,
    // obj_HASH,
};

typedef struct {
    enum obj_Type type;

    union {
        int m_int;
        bool m_bool;
    };
} obj_Object;

// Native objects to be referenced by ptrs
static obj_Object TRUE_OBJECT = { .type = obj_BOOLEAN, .m_bool = true };
static obj_Object FALSE_OBJECT = { .type = obj_BOOLEAN, .m_bool = false };

obj_Object *obj_native_bool_object(bool val) {
    if (val) {
        return &TRUE_OBJECT;
    } else {
        return &FALSE_OBJECT;
    }
}

obj_Object *obj_alloc_object(enum obj_Type type) {
    obj_Object *obj = NULL;
    switch (type) {
        case obj_INTEGER:
            obj = malloc(sizeof(obj_Object));
            obj->type = obj_INTEGER;
            break;
        case obj_BOOLEAN: // should use the native objects
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

void obj_free_object(obj_Object *obj) {
    switch (obj->type) {
        case obj_INTEGER:
            free(obj);
            break;
        case obj_BOOLEAN:
            // NOTHING since we use native obj
            break;
        default:
            assert(0 && "unreachable");
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
        default:
            assert(0 && "unreachable");
    }
    return res;
}
