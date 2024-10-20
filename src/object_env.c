#pragma once
#include "object_env.h"

obj_Env *obj_alloc_env() {
    obj_Env *env = malloc(sizeof(obj_Env));
    env->store = NULL;
    env->outer = NULL;
    return env;
}

obj_Env *obj_alloc_enclosed_env(obj_Env *outer) {
    obj_Env *env = obj_alloc_env();
    env->outer = obj_env_deepcpy(outer);
    return env;
}

void obj_free_env(obj_Env *obj) {
    if (obj == NULL)
        return;
    stbds_arrfree(obj->store);
    obj_free_env(obj->outer);
    free(obj);
}

obj_Env *obj_env_deepcpy(obj_Env *obj) {
    if (obj == NULL)
        return NULL;
    obj_Env *res = obj_alloc_env();

    for (int i = 0; i < stbds_arrlen(obj->store); ++i) {
        obj_Env_elem elem = {
            .key = util_str_deepcopy(obj->store[i].key),
            .value = obj_deepcpy(obj->store[i].value),
        };
        stbds_arrput(res->store, elem);
    }

    // copy outer
    res->outer = obj_env_deepcpy(obj->outer);

    return res;
}

// FIXME: convert to hashmap for better perf
obj_Object *obj_env_get(obj_Env *env, char *name) {
    if (env == NULL)
        return NULL;

    for (int i = 0; i < stbds_arrlen(env->store); ++i) {
        if (strcmp(env->store[i].key, name) == 0) {
            return env->store[i].value;
        }
    }

    // check outer if its not NULL
    if (env->outer != NULL)
        return obj_env_get(env->outer, name);

    return NULL;
}

// Helper function
void obj_print_env(obj_Env *env) {
    if (env == NULL)
        return;

    printf(env->store ? "\ninner env:" : "");
    for (int i = 0; i < stbds_arrlen(env->store); ++i) {
        printf(
            "\n %d elem: key - %s, val - %s",
            i,
            env->store[i].key,
            obj_object_inspect(env->store[i].value)
        );
    }
    obj_print_env(env->outer);
}

void obj_env_set(obj_Env *env, gbString name, obj_Object *val) {
    // search if not found add it to end of array
    int found_idx = -1;
    for (int i = 0; i < stbds_arrlen(env->store); ++i) {
        if (strcmp(env->store[i].key, name) == 0) {
            assert(found_idx == -1);
            found_idx = i;
        }
    }

    if (found_idx != -1) {
        // FIXME: free previous or use arena
        env->store[found_idx].value = obj_deepcpy(val);
    } else {
        obj_Env_elem elem = {
            .key = util_str_deepcopy(name),
            .value = obj_deepcpy(val),
        };
        stbds_arrput(env->store, elem);
    }
    return;
}
