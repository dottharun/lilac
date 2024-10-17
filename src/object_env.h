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
} obj_Env;

void obj_free_env(obj_Env *obj);

obj_Env *obj_env_deepcpy(obj_Env *obj);
