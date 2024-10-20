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

    obj_Object *obj = obj_alloc_object(obj_INTEGER);
    switch (arg->type) {
        case obj_STRING:
            obj->m_int = strlen(arg->m_str);
            break;
        case obj_ARRAY:
            obj->m_int = stbds_arrlen(arg->m_arr_da);
            break;
        default:
            obj = obj_alloc_err_object(
                "argument to `len` not supported, got %s",
                obj_object_name(arg->type)
            );
    }

    return obj;
}

obj_Object *builtin_eval_first(obj_Object **args) {
    if (stbds_arrlen(args) != 1) {
        return obj_alloc_err_object(
            "wrong number of arguments. got=%d, want=1",
            stbds_arrlen(args)
        );
    }

    obj_Object *arr = args[0];
    if (arr->type != obj_ARRAY) {
        return obj_alloc_err_object(
            "argument to `first` must be obj_ARRAY, got %s",
            obj_object_name(arr->type)
        );
    }

    if (stbds_arrlen(arr->m_arr_da) == 0) {
        return obj_null();
    }
    assert(stbds_arrlen(arr->m_arr_da) > 0);
    return obj_deepcpy(arr->m_arr_da[0]);
}

obj_Object *builtin_eval_last(obj_Object **args) {
    if (stbds_arrlen(args) != 1) {
        return obj_alloc_err_object(
            "wrong number of arguments. got=%d, want=1",
            stbds_arrlen(args)
        );
    }

    obj_Object *arr = args[0];
    if (arr->type != obj_ARRAY) {
        return obj_alloc_err_object(
            "argument to `last` must be obj_ARRAY, got %s",
            obj_object_name(arr->type)
        );
    }

    int n = stbds_arrlen(arr->m_arr_da);
    if (n == 0) {
        return obj_null();
    }
    assert(n - 1 >= 0);
    return obj_deepcpy(arr->m_arr_da[n - 1]);
}

obj_Object *builtin_eval_rest(obj_Object **args) {
    if (stbds_arrlen(args) != 1) {
        return obj_alloc_err_object(
            "wrong number of arguments. got=%d, want=1",
            stbds_arrlen(args)
        );
    }

    obj_Object *arr = args[0];
    if (arr->type != obj_ARRAY) {
        return obj_alloc_err_object(
            "argument to `rest` must be obj_ARRAY, got %s",
            obj_object_name(arr->type)
        );
    }

    int n = stbds_arrlen(arr->m_arr_da);
    if (n == 0) {
        return obj_null();
    }

    obj_Object *obj = obj_alloc_object(obj_ARRAY);
    for (int i = 1; i < n; ++i) {
        obj_Object *elem = obj_deepcpy(arr->m_arr_da[i]);
        stbds_arrput(obj->m_arr_da, elem);
    }
    return obj;
}

obj_Object *builtin_eval_push(obj_Object **args) {
    if (stbds_arrlen(args) != 2) {
        return obj_alloc_err_object(
            "wrong number of arguments. got=%d, want=2",
            stbds_arrlen(args)
        );
    }

    obj_Object *arr = args[0];
    if (arr->type != obj_ARRAY) {
        return obj_alloc_err_object(
            "argument to `push` must be obj_ARRAY, got %s",
            obj_object_name(arr->type)
        );
    }

    int n = stbds_arrlen(arr->m_arr_da);

    obj_Object *new_arr = obj_deepcpy(arr);
    obj_Object *new_elem = obj_deepcpy(args[1]);
    stbds_arrput(new_arr->m_arr_da, new_elem);

    assert(stbds_arrlen(new_arr->m_arr_da) == n + 1);
    return new_arr;
}
