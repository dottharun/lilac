#pragma once
#include "object.c"

// forward decl
typedef struct obj_Object obj_Object;

typedef struct {
    char *key;
    obj_Object *value;
} obj_Env_elem;

typedef struct obj_Env {
    obj_Env_elem *store;
    struct obj_Env *outer;
} obj_Env;

obj_Env *obj_alloc_env();
obj_Env *obj_alloc_enclosed_env(obj_Env *outer);

void obj_free_env(obj_Env *obj);

obj_Env *obj_env_deepcpy(obj_Env *obj);

obj_Object *obj_env_get(obj_Env *env, gbString name);
void obj_env_set(obj_Env *env, gbString name, obj_Object *val);

void obj_print_env(obj_Env *env);
