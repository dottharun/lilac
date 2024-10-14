#include "ast.c"
#include "object.c"

obj_Object *eval_expr(struct ast_Expr *expr);

obj_Object *eval_stmt(struct ast_Stmt *stmt);

obj_Object *eval_stmts(struct ast_Stmt **stmts);
