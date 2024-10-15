#pragma once
#include "object.c"

typedef struct {
    char *key;
    obj_Object *value;
} obj_Env_elem;

typedef struct {
    obj_Env_elem *store;
} obj_Env;

obj_Env *obj_alloc_env() {
    obj_Env *env = malloc(sizeof(obj_Env));
    env->store = NULL;
    return env;
}

// FIXME: convert to hashmap for better perf
obj_Object *obj_env_get(obj_Env *env, gbString name) {
    for (int i = 0; i < stbds_arrlen(env->store); ++i) {
        if (strcmp(env->store[i].key, name) == 0) {
            return env->store[i].value;
        }
    }
    return NULL;
}

// Helper function
void printarr(obj_Env_elem *store) {
    printf("\nprinting arr");
    for (int i = 0; i < stbds_arrlen(store); ++i) {
        printf(
            "\n %d elem: key - %s, val - %s",
            i,
            store[i].key,
            obj_object_inspect(store[i].value)
        );
    }
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
