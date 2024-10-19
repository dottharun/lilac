#include "ast.h"
#include "object.c"
#include "object_env.h"

obj_Object *eval_expr(struct ast_Expr *expr, obj_Env *env);

obj_Object *eval_stmt(struct ast_Stmt *stmt, obj_Env *env);
