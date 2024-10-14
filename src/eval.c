#include "ast.c"
#include "object.c"

obj_Object *eval_bang_operator_expr(obj_Object *right) {
    if (obj_is_same(right, &TRUE_OBJECT)) {
        return obj_native_bool_object(false);
    } else if (obj_is_same(right, &FALSE_OBJECT)) {
        return obj_native_bool_object(true);
    } else if (obj_is_same(right, &NULL_OBJECT)) {
        return obj_native_bool_object(true);
    } else {
        return obj_native_bool_object(false);
    }
}

obj_Object *eval_prefix_expr(char *operator, obj_Object * right) {
    if (strcmp(operator, "!") == 0) {
        return eval_bang_operator_expr(right);
    } else {
        assert(0 && "unreachable");
        // return NULL; //TODO: implement err handling
    }
}

obj_Object *eval_expr(struct ast_Expr *expr) {
    obj_Object *obj = NULL;
    switch (expr->tag) {
        case ast_INT_LIT_EXPR:
            obj = obj_alloc_object(obj_INTEGER);
            obj->m_int = expr->data.int_lit.value;
            break;
        case ast_BOOL_EXPR:
            obj = obj_native_bool_object(expr->data.boolean.value);
            break;
        case ast_PREFIX_EXPR:
            obj = eval_prefix_expr(
                expr->data.pf.operator,
                eval_expr(expr->data.pf.right)
            );
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_stmt(struct ast_Stmt *stmt) {
    obj_Object *obj = NULL;
    switch (stmt->tag) {
        case ast_EXPR_STMT:
            obj = eval_expr(stmt->data.expr.expr);
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_prg(struct ast_Program *prg) {
    obj_Object *obj = NULL;
    for (int i = 0; i < stbds_arrlen(prg->statement_ptrs_da); ++i) {
        // TODO: this needs to handle each stmts separately
        obj = eval_stmt(prg->statement_ptrs_da[i]);
    }
    return obj;
}

/*
 * Evaluates Program, Statemtents, Expressions as ast_Node
 */
obj_Object *eval_eval(ast_Node node) {
    switch (node.tag) {
        case ast_NODE_PRG:
            return eval_prg(node.prg);
        case ast_NODE_EXPR:
            return eval_expr(node.expr);
        case ast_NODE_STMT:
            return eval_stmt(node.stmt);
        default:
            assert(0 && "unreachable");
    }
}
