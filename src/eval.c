#include "eval.h"

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

obj_Object *eval_minus_operator_prefix_expr(obj_Object *right) {
    if (right->type != obj_INTEGER) {
        obj_Object *res = obj_alloc_err_object(
            "unknown operator: -%s",
            obj_object_name(right->type)
        );
        obj_free_object(right);
        return res;
    }

    right->m_int = -right->m_int;
    return right;
}

obj_Object *eval_prefix_expr(char *operator, obj_Object * right) {
    if (strcmp(operator, "!") == 0) {
        return eval_bang_operator_expr(right);
    } else if (strcmp(operator, "-") == 0) {
        return eval_minus_operator_prefix_expr(right);
    } else {
        obj_Object *res = obj_alloc_err_object(
            "unknown operator: %s%s",
            operator,
            obj_object_name(right->type)
        );
        obj_free_object(right);
        return res;
    }
}

obj_Object *
eval_int_infix_expr(char *operator, obj_Object * left, obj_Object *right) {
    if (strcmp(operator, "+") == 0) {
        left->m_int += right->m_int;
        obj_free_object(right);
        return left;
    } else if (strcmp(operator, "-") == 0) {
        left->m_int -= right->m_int;
        obj_free_object(right);
        return left;
    } else if (strcmp(operator, "*") == 0) {
        left->m_int *= right->m_int;
        obj_free_object(right);
        return left;
    } else if (strcmp(operator, "/") == 0) {
        left->m_int /= right->m_int;
        obj_free_object(right);
        return left;
    } else if (strcmp(operator, "<") == 0) {
        obj_Object *cmp = obj_native_bool_object(left->m_int < right->m_int);
        obj_free_object(left);
        obj_free_object(right);
        return cmp;
    } else if (strcmp(operator, ">") == 0) {
        obj_Object *cmp = obj_native_bool_object(left->m_int > right->m_int);
        obj_free_object(left);
        obj_free_object(right);
        return cmp;
    } else if (strcmp(operator, "==") == 0) {
        obj_Object *cmp = obj_native_bool_object(left->m_int == right->m_int);
        obj_free_object(left);
        obj_free_object(right);
        return cmp;
    } else if (strcmp(operator, "!=") == 0) {
        obj_Object *cmp = obj_native_bool_object(left->m_int != right->m_int);
        obj_free_object(left);
        obj_free_object(right);
        return cmp;
    } else {
        obj_Object *res = obj_alloc_err_object(
            "unknown operator: %s %s %s",
            obj_object_name(left->type),
            operator,
            obj_object_name(right->type)
        );
        obj_free_object(right);
        obj_free_object(left);
        return res;
    }
}

obj_Object *
eval_infix_expr(char *operator, obj_Object * left, obj_Object *right) {
    if (left->type == obj_INTEGER && right->type == obj_INTEGER) {
        return eval_int_infix_expr(operator, left, right);
    } else if (strcmp(operator, "==") == 0) {
        obj_Object *cmp = obj_native_bool_object(obj_is_same(left, right));
        obj_free_object(right);
        obj_free_object(left);
        return cmp;
    } else if (strcmp(operator, "!=") == 0) {
        obj_Object *cmp = obj_native_bool_object(!obj_is_same(left, right));
        obj_free_object(right);
        obj_free_object(left);
        return cmp;
    } else if (left->type != right->type) {
        obj_Object *res = obj_alloc_err_object(
            "type mismatch: %s %s %s",
            obj_object_name(left->type),
            operator,
            obj_object_name(right->type)
        );
        obj_free_object(right);
        obj_free_object(left);
        return res;
    } else {
        obj_Object *res = obj_alloc_err_object(
            "unknown operator: %s %s %s",
            obj_object_name(left->type),
            operator,
            obj_object_name(right->type)
        );
        obj_free_object(right);
        obj_free_object(left);
        return res;
    }
}

obj_Object *eval_if_expr(struct ast_Expr *if_expr) {
    obj_Object *cond = eval_expr(if_expr->data.ife.cond);
    if (obj_is_err(cond)) {
        return cond;
    }

    if (obj_is_truthy(cond)) {
        return eval_stmt(if_expr->data.ife.conseq);
    } else if (if_expr->data.ife.alt != NULL) {
        return eval_stmt(if_expr->data.ife.alt);
    } else {
        return obj_null();
    }
}

obj_Object *eval_expr(struct ast_Expr *expr) {
    obj_Object *obj = NULL;

    obj_Object *left = NULL;
    obj_Object *right = NULL;
    switch (expr->tag) {
        case ast_INT_LIT_EXPR:
            obj = obj_alloc_object(obj_INTEGER);
            obj->m_int = expr->data.int_lit.value;
            break;
        case ast_BOOL_EXPR:
            obj = obj_native_bool_object(expr->data.boolean.value);
            break;
        case ast_PREFIX_EXPR:
            right = eval_expr(expr->data.pf.right);
            if (obj_is_err(right)) {
                return right;
            }
            obj = eval_prefix_expr(expr->data.pf.operator, right);
            break;
        case ast_INFIX_EXPR:
            left = eval_expr(expr->data.inf.left);
            if (obj_is_err(left)) {
                return left;
            }
            right = eval_expr(expr->data.inf.right);
            if (obj_is_err(right)) {
                return right;
            }
            obj = eval_infix_expr(expr->data.inf.operator, left, right);
            break;
        case ast_IF_EXPR:
            obj = eval_if_expr(expr);
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_block_stmts(struct ast_Stmt **stmts) {
    obj_Object *obj = NULL;
    for (int i = 0; i < stbds_arrlen(stmts); ++i) {
        obj = eval_stmt(stmts[i]);

        // Here we are not unwrapping return val here and sending it to caller
        // as return val
        if (obj != NULL &&
            (obj->type == obj_RETURN_VALUE || obj->type == obj_ERROR)) {
            return obj;
        }
    }
    return obj;
}

obj_Object *eval_stmt(struct ast_Stmt *stmt) {
    obj_Object *obj = NULL;
    obj_Object *val = NULL;
    switch (stmt->tag) {
        case ast_EXPR_STMT:
            obj = eval_expr(stmt->data.expr.expr);
            break;
        case ast_BLOCK_STMT:
            obj = eval_block_stmts(stmt->data.block.stmts_da);
            break;
        case ast_RET_STMT:
            val = eval_expr(stmt->data.ret.ret_val);
            if (obj_is_err(val)) {
                return val;
            }
            obj = obj_alloc_object(obj_RETURN_VALUE);
            obj->m_return_obj = val;
            break;
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_prg(struct ast_Stmt **stmts) {
    obj_Object *obj = NULL;
    for (int i = 0; i < stbds_arrlen(stmts); ++i) {
        obj = eval_stmt(stmts[i]);

        obj_Object *ret = NULL;
        switch (obj->type) {
            case obj_RETURN_VALUE:
                // Here we are unwrapping the return val and convert it as
                // normal object to the caller - since programs are not
                // recursive and need to evaluate calculatable value
                ret = obj->m_return_obj;
                free(obj);
                return ret;
            case obj_ERROR:
                return obj;
            default:
                break;
        }
    }
    return obj;
}

/*
 * Evaluates Program, Statemtents, Expressions as ast_Node
 */
obj_Object *eval_eval(ast_Node node) {
    switch (node.tag) {
        case ast_NODE_PRG:
            return eval_prg(node.prg->statement_ptrs_da);
        case ast_NODE_EXPR:
            return eval_expr(node.expr);
        case ast_NODE_STMT:
            return eval_stmt(node.stmt);
        default:
            assert(0 && "unreachable");
    }
}
