#pragma once
#include "object.c"
#include "util.c"

obj_Object *builtin_from_keyword(const char *keyword) {
    obj_Object *obj = obj_alloc_object(obj_BUILTIN);

    if (0 == strcmp(keyword, "len")) {
        obj->m_builtin = BUILTIN_LEN;
    } else if (0 == strcmp(keyword, "first")) {
        obj->m_builtin = BUILTIN_FIRST;
    } else if (0 == strcmp(keyword, "last")) {
        obj->m_builtin = BUILTIN_LAST;
    } else if (0 == strcmp(keyword, "rest")) {
        obj->m_builtin = BUILTIN_REST;
    } else if (0 == strcmp(keyword, "push")) {
        obj->m_builtin = BUILTIN_PUSH;
    } else {
        return NULL;
    }

    return obj;
}

obj_Object *builtin_eval_len(obj_Object **args) {
    if (stbds_arrlen(args) != 1) {
        return obj_alloc_err_object(
            "wrong number of arguments. got=%d, want=1",
            stbds_arrlen(args)
        );
    }
    obj_Object *arg = args[0];

    obj_Object *obj = NULL;
    switch (arg->type) {
        case obj_STRING:
            obj = obj_alloc_object(obj_INTEGER);
            obj->m_int = strlen(arg->m_str);
            break;
        default:
            obj = obj_alloc_err_object(
                "argument to `len` not supported, got %s",
                obj_object_name(arg->type)
            );
    }

    return obj;
}
