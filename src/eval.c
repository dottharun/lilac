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

obj_Object *eval_if_expr(struct ast_Expr *if_expr, obj_Env *env) {
    obj_Object *cond = eval_expr(if_expr->data.ife.cond, env);
    if (obj_is_err(cond)) {
        return cond;
    }

    if (obj_is_truthy(cond)) {
        return eval_stmt(if_expr->data.ife.conseq, env);
    } else if (if_expr->data.ife.alt != NULL) {
        return eval_stmt(if_expr->data.ife.alt, env);
    } else {
        return obj_null();
    }
}

obj_Object *eval_identifier(struct ast_Expr *ident_expr, obj_Env *env) {
    obj_Object *exists = obj_env_get(env, ident_expr->data.ident.value);
    if (exists == NULL) {
        return obj_alloc_err_object(
            "identifier not found: %s",
            ident_expr->data.ident.value
        );
    }

    obj_Object *val = obj_deepcpy(exists);
    return val;
}

obj_Object **eval_expressions(struct ast_Expr **expr_da, obj_Env *env) {
    obj_Object **obj_da = NULL;

    for (int i = 0; i < stbds_arrlen(expr_da); ++i) {
        struct ast_Expr *expr = expr_da[i];
        obj_Object *evaluated = eval_expr(expr, env);
        if (obj_is_err(evaluated)) {
            // FIXME: free correctly or use arena
            stbds_arrfree(obj_da);

            obj_da = NULL;
            stbds_arrput(obj_da, evaluated);
            return obj_da;
        }
        stbds_arrput(obj_da, evaluated);
    }

    return obj_da;
}

obj_Env *eval_extend_func_env(obj_Object *func, obj_Object **args) {
    obj_Env *env = obj_alloc_enclosed_env(func->m_func.env);

    for (int i = 0; i < stbds_arrlen(func->m_func.params); ++i) {
        obj_env_set(env, func->m_func.params[i]->data.ident.value, args[i]);
    }
    return env;
}

obj_Object *eval_unwrap_return_val(obj_Object *obj) {
    if (obj->type == obj_RETURN_VALUE) {
        return obj->m_return_obj;
    }
    return obj;
}

obj_Object *eval_apply_func(obj_Object *func, obj_Object **args) {
    if (func->type != obj_FUNCTION) {
        return obj_alloc_err_object("not a function: xoxo");
    }

    obj_Env *extended_env = eval_extend_func_env(func, args);
    obj_Object *evaluated = eval_stmt(func->m_func.body, extended_env);
    return eval_unwrap_return_val(evaluated);
}

obj_Object *eval_expr(struct ast_Expr *expr, obj_Env *env) {
    obj_Object *obj = NULL;

    obj_Object *left = NULL;
    obj_Object *right = NULL;

    obj_Object *func = NULL;
    obj_Object **args = NULL;
    switch (expr->tag) {
        case ast_INT_LIT_EXPR:
            obj = obj_alloc_object(obj_INTEGER);
            obj->m_int = expr->data.int_lit.value;
            break;
        case ast_BOOL_EXPR:
            obj = obj_native_bool_object(expr->data.boolean.value);
            break;
        case ast_PREFIX_EXPR:
            right = eval_expr(expr->data.pf.right, env);
            if (obj_is_err(right)) {
                return right;
            }
            obj = eval_prefix_expr(expr->data.pf.operator, right);
            break;
        case ast_INFIX_EXPR:
            left = eval_expr(expr->data.inf.left, env);
            if (obj_is_err(left)) {
                return left;
            }
            right = eval_expr(expr->data.inf.right, env);
            if (obj_is_err(right)) {
                return right;
            }
            obj = eval_infix_expr(expr->data.inf.operator, left, right);
            break;
        case ast_IF_EXPR:
            obj = eval_if_expr(expr, env);
            break;
        case ast_IDENT_EXPR:
            obj = eval_identifier(expr, env);
            break;
        case ast_FN_LIT_EXPR:
            obj = obj_alloc_object(obj_FUNCTION);
            obj->m_func.params =
                ast_deepcpy_fn_params(expr->data.fn_lit.params_da);
            obj->m_func.body = ast_deepcopy_stmt(expr->data.fn_lit.body);
            obj->m_func.env = obj_env_deepcpy(env);
            break;
        case ast_CALL_EXPR:
            func = eval_expr(expr->data.call.func, env);
            if (obj_is_err(func)) {
                return func;
            }

            args = eval_expressions(expr->data.call.args_da, env);
            if (stbds_arrlen(args) == 1 && obj_is_err(args[0])) {
                return args[0];
            }
            obj = eval_apply_func(func, args);
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_block_stmts(struct ast_Stmt **stmts, obj_Env *env) {
    obj_Object *obj = NULL;
    for (int i = 0; i < stbds_arrlen(stmts); ++i) {
        obj = eval_stmt(stmts[i], env);

        // Here we are not unwrapping return val here and sending it to caller
        // as return val
        if (obj != NULL &&
            (obj->type == obj_RETURN_VALUE || obj->type == obj_ERROR)) {
            return obj;
        }
    }
    return obj;
}

obj_Object *eval_stmt(struct ast_Stmt *stmt, obj_Env *env) {
    obj_Object *obj = NULL;
    obj_Object *val = NULL;
    switch (stmt->tag) {
        case ast_EXPR_STMT:
            obj = eval_expr(stmt->data.expr.expr, env);
            break;
        case ast_BLOCK_STMT:
            obj = eval_block_stmts(stmt->data.block.stmts_da, env);
            break;
        case ast_RET_STMT:
            val = eval_expr(stmt->data.ret.ret_val, env);
            if (obj_is_err(val)) {
                return val;
            }
            obj = obj_alloc_object(obj_RETURN_VALUE);
            obj->m_return_obj = val;
            break;
        case ast_LET_STMT:
            val = eval_expr(stmt->data.let.value, env);
            if (obj_is_err(val)) {
                return val;
            }
            obj_env_set(env, stmt->data.let.name->data.ident.value, val);
            break;
        default:
            assert(0 && "unreachable");
    }
    return obj;
}

obj_Object *eval_prg(struct ast_Stmt **stmts, obj_Env *env) {
    obj_Object *obj = NULL;
    for (int i = 0; i < stbds_arrlen(stmts); ++i) {
        obj = eval_stmt(stmts[i], env);
        if (obj == NULL) {
            continue;
        }

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
obj_Object *eval_eval(ast_Node node, obj_Env *env) {
    switch (node.tag) {
        case ast_NODE_PRG:
            return eval_prg(node.prg->statement_ptrs_da, env);
        case ast_NODE_EXPR:
            return eval_expr(node.expr, env);
        case ast_NODE_STMT:
            return eval_stmt(node.stmt, env);
        default:
            assert(0 && "unreachable");
    }
}
