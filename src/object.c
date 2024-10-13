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

gbString obj_object_inspect(obj_Object *obj) {
    gbString res = gb_make_string("");
    switch (obj->type) {
        case obj_INTEGER:
            gb_append_string(res, util_int_to_str(obj->m_int));
            break;
        case obj_BOOLEAN:
            gb_append_cstring(res, obj->m_bool ? "true" : "false");
            break;
        case obj_NULL:
            gb_append_cstring(res, "null");
            break;
        default:
            assert(0 && "unreachable");
    }
    return res;
}
