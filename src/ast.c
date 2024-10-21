#pragma once
#include "ast.h"

struct ast_Expr **ast_deepcpy_fn_params(struct ast_Expr **params) {
    struct ast_Expr **res_da = NULL;

    for (int i = 0; i < stbds_arrlen(params); ++i) {
        struct ast_Expr *elem = ast_deepcopy_expr(params[i]);
        stbds_arrput(res_da, elem);
    }
    return res_da;
}

struct ast_Expr *ast_deepcopy_expr(const struct ast_Expr *expr) {
    if (expr == NULL)
        return NULL;
    struct ast_Expr *new_expr = malloc(sizeof(struct ast_Expr));
    memcpy(new_expr, expr, sizeof(struct ast_Expr));

    switch (expr->tag) {
        case ast_IDENT_EXPR:
        case ast_INT_LIT_EXPR:
        case ast_BOOL_EXPR:
        case ast_STR_LIT_EXPR:
            break;

        case ast_PREFIX_EXPR:
            new_expr->data.pf.right = ast_deepcopy_expr(expr->data.pf.right);
            break;

        case ast_INFIX_EXPR:
            new_expr->data.inf.left = ast_deepcopy_expr(expr->data.inf.left);
            new_expr->data.inf.right = ast_deepcopy_expr(expr->data.inf.right);
            break;

        case ast_IF_EXPR:
            new_expr->data.ife.cond = ast_deepcopy_expr(expr->data.ife.cond);
            new_expr->data.ife.conseq =
                ast_deepcopy_stmt(expr->data.ife.conseq);
            new_expr->data.ife.alt = ast_deepcopy_stmt(expr->data.ife.alt);
            break;

        case ast_FN_LIT_EXPR: {
            new_expr->data.fn_lit.params_da = NULL;
            for (int i = 0; i < stbds_arrlen(expr->data.fn_lit.params_da);
                 i++) {
                struct ast_Expr *elem =
                    ast_deepcopy_expr(expr->data.fn_lit.params_da[i]);
                stbds_arrput(new_expr->data.fn_lit.params_da, elem);
            }
            new_expr->data.fn_lit.body =
                ast_deepcopy_stmt(expr->data.fn_lit.body);
            break;
        }

        case ast_CALL_EXPR: {
            new_expr->data.call.func = ast_deepcopy_expr(expr->data.call.func);
            new_expr->data.call.args_da = NULL;
            for (int i = 0; i < stbds_arrlen(expr->data.call.args_da); i++) {
                struct ast_Expr *elem =
                    ast_deepcopy_expr(expr->data.call.args_da[i]);
                stbds_arrput(new_expr->data.call.args_da, elem);
            }
            break;
        }
        case ast_ARR_LIT_EXPR:
            new_expr->data.arr.elems_da = NULL;
            for (int i = 0; i < stbds_arrlen(expr->data.arr.elems_da); i++) {
                struct ast_Expr *elem =
                    ast_deepcopy_expr(expr->data.arr.elems_da[i]);
                stbds_arrput(new_expr->data.arr.elems_da, elem);
            }
            break;
        case ast_IDX_EXPR:
            new_expr->data.idx.left = ast_deepcopy_expr(expr->data.idx.left);
            new_expr->data.idx.index = ast_deepcopy_expr(expr->data.idx.index);
            break;
        case ast_HASH_LIT_EXPR:
            new_expr->data.hash.hash_da = NULL;
            for (int i = 0; i < stbds_arrlen(expr->data.hash.hash_da); i++) {
                struct ast_Hash_elem *elem =
                    malloc(sizeof(struct ast_Hash_elem));
                elem->key = ast_deepcopy_expr(expr->data.hash.hash_da[i]->key);
                elem->val = ast_deepcopy_expr(expr->data.hash.hash_da[i]->val);
                stbds_arrput(new_expr->data.hash.hash_da, elem);
            }
            break;
    }

    return new_expr;
}

struct ast_Stmt *ast_deepcopy_stmt(const struct ast_Stmt *stmt) {
    if (stmt == NULL)
        return NULL;

    struct ast_Stmt *new_stmt = malloc(sizeof(struct ast_Stmt));
    memcpy(new_stmt, stmt, sizeof(struct ast_Stmt));

    switch (stmt->tag) {
        case ast_LET_STMT:
            new_stmt->data.let.name = ast_deepcopy_expr(stmt->data.let.name);
            new_stmt->data.let.value = ast_deepcopy_expr(stmt->data.let.value);
            break;
        case ast_RET_STMT:
            new_stmt->data.ret.ret_val =
                ast_deepcopy_expr(stmt->data.ret.ret_val);
            break;
        case ast_EXPR_STMT:
            new_stmt->data.expr.expr = ast_deepcopy_expr(stmt->data.expr.expr);
            break;
        case ast_BLOCK_STMT: {

            new_stmt->data.block.stmts_da = NULL;
            for (int i = 0; i < stbds_arrlen(stmt->data.block.stmts_da); i++) {
                struct ast_Stmt *elem =
                    ast_deepcopy_stmt(stmt->data.block.stmts_da[i]);
                stbds_arrput(new_stmt->data.block.stmts_da, elem);
            }
            break;
        }
    }

    return new_stmt;
}

void ast_free_expr(struct ast_Expr *expr) {
    if (expr == NULL)
        return;
    switch (expr->tag) {
        case ast_IDENT_EXPR:
        case ast_INT_LIT_EXPR:
        case ast_BOOL_EXPR:
        case ast_STR_LIT_EXPR:
            // NOTHING
            break;
        case ast_PREFIX_EXPR:
            ast_free_expr(expr->data.pf.right);
            break;
        case ast_INFIX_EXPR:
            ast_free_expr(expr->data.inf.left);
            ast_free_expr(expr->data.inf.right);
            break;
        case ast_IF_EXPR:
            ast_free_expr(expr->data.ife.cond);
            ast_free_stmt(expr->data.ife.conseq);
            ast_free_stmt(expr->data.ife.alt);
            break;
        case ast_FN_LIT_EXPR:
            for (int i = 0; i < stbds_arrlen(expr->data.fn_lit.params_da);
                 ++i) {
                struct ast_Expr *param = expr->data.fn_lit.params_da[i];
                ast_free_expr(param);
            }
            stbds_arrfree(expr->data.fn_lit.params_da);

            ast_free_stmt(expr->data.fn_lit.body);
            break;
        case ast_CALL_EXPR:
            for (int i = 0; i < stbds_arrlen(expr->data.call.args_da); ++i) {
                struct ast_Expr *arg = expr->data.call.args_da[i];
                ast_free_expr(arg);
            }
            stbds_arrfree(expr->data.call.args_da);

            ast_free_expr(expr->data.call.func);
            break;
        case ast_ARR_LIT_EXPR:
            for (int i = 0; i < stbds_arrlen(expr->data.arr.elems_da); ++i) {
                struct ast_Expr *elem = expr->data.arr.elems_da[i];
                ast_free_expr(elem);
            }
            stbds_arrfree(expr->data.arr.elems_da);
            break;
        case ast_IDX_EXPR:
            ast_free_expr(expr->data.idx.left);
            ast_free_expr(expr->data.idx.index);
            break;
        case ast_HASH_LIT_EXPR:
            for (int i = 0; i < stbds_arrlen(expr->data.hash.hash_da); ++i) {
                ast_free_expr(expr->data.hash.hash_da[i]->key);
                ast_free_expr(expr->data.hash.hash_da[i]->val);
            }

            stbds_arrfree(expr->data.hash.hash_da);
            break;
        default:
            assert(0 && "unreachable");
    }
    free(expr);
};

struct ast_Expr *ast_alloc_expr(enum ast_expr_tag tag) {
    struct ast_Expr *expr = malloc(sizeof(struct ast_Expr));
    expr->tag = tag;

    switch (tag) {
        case ast_IDENT_EXPR:
        case ast_INT_LIT_EXPR:
        case ast_BOOL_EXPR:
        case ast_STR_LIT_EXPR:
            // NOTHING
            break;
        case ast_PREFIX_EXPR:
            expr->data.pf.right = NULL;
            break;
        case ast_INFIX_EXPR:
            expr->data.inf.left = NULL;
            expr->data.inf.right = NULL;
            break;
        case ast_IF_EXPR:
            expr->data.ife.cond = NULL;
            expr->data.ife.conseq = NULL;
            expr->data.ife.alt = NULL;
            break;
        case ast_FN_LIT_EXPR:
            expr->data.fn_lit.params_da = NULL;
            expr->data.fn_lit.body = NULL;
            break;
        case ast_ARR_LIT_EXPR:
            expr->data.arr.elems_da = NULL;
            break;
        case ast_HASH_LIT_EXPR:
            expr->data.hash.hash_da = NULL;
            break;
        default:
            assert(0 && "unreachable");
    }

    return expr;
}

gbString ast_make_expr_str(struct ast_Expr *expr) {
    gbString str = gb_make_string("");

    switch (expr->tag) {
        case ast_IDENT_EXPR:
            // FIXME: could be made simpler with gbString?
            str = gb_append_cstring(str, expr->data.ident.value);
            break;
        case ast_INT_LIT_EXPR:
        case ast_BOOL_EXPR:
        case ast_STR_LIT_EXPR:
            str = gb_append_cstring(str, expr->token.literal);
            break;
        case ast_PREFIX_EXPR:
            str = gb_append_cstring(str, "(");
            str = gb_append_cstring(str, expr->data.pf.operator);
            str = gb_append_string(str, ast_make_expr_str(expr->data.pf.right));
            str = gb_append_cstring(str, ")");
            break;
        case ast_INFIX_EXPR:
            str = gb_append_cstring(str, "(");
            str = gb_append_string(str, ast_make_expr_str(expr->data.inf.left));
            str = gb_append_cstring(str, " ");
            str = gb_append_cstring(str, expr->data.inf.operator);
            str = gb_append_cstring(str, " ");
            str =
                gb_append_string(str, ast_make_expr_str(expr->data.inf.right));
            str = gb_append_cstring(str, ")");
            break;
        case ast_IF_EXPR:
            str = gb_append_cstring(str, "if");
            str = gb_append_string(str, ast_make_expr_str(expr->data.ife.cond));
            str = gb_append_cstring(str, " ");
            str =
                gb_append_string(str, ast_make_stmt_str(expr->data.ife.conseq));

            if (expr->data.ife.alt != NULL) {
                str = gb_append_cstring(str, "else ");
                str = gb_append_string(
                    str,
                    ast_make_stmt_str(expr->data.ife.alt)
                );
            }
            break;
        case ast_FN_LIT_EXPR:
            str = gb_append_cstring(str, expr->token.literal);

            str = gb_append_cstring(str, "(");
            struct ast_Expr **params = expr->data.fn_lit.params_da;
            for (int i = 0; i < stbds_arrlen(params); ++i) {
                gbString param = ast_make_expr_str(params[i]);
                str = gb_append_string(str, param);
                gb_free_string(param);
                if (i == stbds_arrlen(params) - 1) {
                    break;
                }
                str = gb_append_cstring(str, ", ");
            }
            str = gb_append_cstring(str, ") ");

            gbString body = ast_make_stmt_str(expr->data.fn_lit.body);
            str = gb_append_string(str, body);
            break;
        case ast_CALL_EXPR:
            str = gb_append_cstring(str, expr->data.call.func->token.literal);
            str = gb_append_cstring(str, "(");

            struct ast_Expr **args = expr->data.call.args_da;
            for (int i = 0; i < stbds_arrlen(args); ++i) {
                gbString arg = ast_make_expr_str(args[i]);
                str = gb_append_string(str, arg);
                gb_free_string(arg);
                if (i == stbds_arrlen(args) - 1) {
                    break;
                }
                str = gb_append_cstring(str, ", ");
            }

            str = gb_append_cstring(str, ")");
            break;
        case ast_ARR_LIT_EXPR:
            str = gb_append_cstring(str, "[");
            struct ast_Expr **elems = expr->data.arr.elems_da;
            for (int i = 0; i < stbds_arrlen(elems); ++i) {
                gbString arg = ast_make_expr_str(elems[i]);
                str = gb_append_string(str, arg);
                gb_free_string(arg);
                if (i == stbds_arrlen(elems) - 1) {
                    break;
                }
                str = gb_append_cstring(str, ", ");
            }

            str = gb_append_cstring(str, "]");
            break;
        case ast_IDX_EXPR:
            str = gb_append_cstring(str, "(");
            str = gb_append_string(str, ast_make_expr_str(expr->data.idx.left));
            str = gb_append_cstring(str, "[");
            str =
                gb_append_string(str, ast_make_expr_str(expr->data.idx.index));
            str = gb_append_cstring(str, "]");
            str = gb_append_cstring(str, ")");
            break;
        case ast_HASH_LIT_EXPR:
            str = gb_append_cstring(str, "{");
            struct ast_Hash_elem **hashes = expr->data.hash.hash_da;
            for (int i = 0; i < stbds_arrlen(hashes); ++i) {
                gbString key = ast_make_expr_str(hashes[i]->key);
                gbString val = ast_make_expr_str(hashes[i]->val);
                str = gb_append_string(str, key);
                str = gb_append_cstring(str, ": ");
                str = gb_append_string(str, val);
                if (i == stbds_arrlen(hashes) - 1) {
                    break;
                }
                str = gb_append_cstring(str, ", ");
            }
            str = gb_append_cstring(str, "}");
            break;
        default:
            assert(0 && "unreachable");
    }

    return str;
}

void ast_free_stmt(struct ast_Stmt *stmt) {
    if (stmt == NULL)
        return;

    switch (stmt->tag) {
        case ast_LET_STMT:
            // FIXME: assert might not be needed
            assert(
                (stmt->data.let.name != NULL) &&
                (stmt->data.let.name->tag == ast_IDENT_EXPR) &&
                "let_stmt should have a identifier"
            );
            ast_free_expr(stmt->data.let.name);
            ast_free_expr(stmt->data.let.value);
            break;
        case ast_RET_STMT:
            ast_free_expr(stmt->data.ret.ret_val);
            break;
        case ast_EXPR_STMT:
            ast_free_expr(stmt->data.expr.expr);
            break;
        case ast_BLOCK_STMT:
            for (int i = 0; i < stbds_arrlen(stmt->data.block.stmts_da); ++i) {
                struct ast_Stmt *stmt_ptr = stmt->data.block.stmts_da[i];
                ast_free_stmt(stmt_ptr);
            }
            stbds_arrfree(stmt->data.block.stmts_da);
            break;
        default:
            assert(0 && "unreachable");
    }

    free(stmt);
}

struct ast_Stmt *ast_alloc_stmt(enum ast_stmt_tag tag) {
    struct ast_Stmt *stmt = malloc(sizeof(struct ast_Stmt));
    stmt->tag = tag;

    switch (tag) {
        case ast_LET_STMT:
            stmt->data.let.name = NULL;
            stmt->data.let.value = NULL;
            break;
        case ast_RET_STMT:
            stmt->data.ret.ret_val = NULL;
            break;
        case ast_EXPR_STMT:
            stmt->data.expr.expr = NULL;
            break;
        case ast_BLOCK_STMT:
            stmt->data.block.stmts_da = NULL;
            break;
        default:
            assert(0 && "unreachable");
    }

    return stmt;
}

// free after using
gbString ast_make_stmt_str(struct ast_Stmt *stmt) {
    gbString str = gb_make_string("");

    switch (stmt->tag) {
        case ast_LET_STMT:
            str = gb_append_cstring(str, stmt->token.literal);
            str = gb_append_cstring(str, " ");

            gbString expr = ast_make_expr_str(stmt->data.let.name);
            str = gb_append_string(str, expr);
            gb_free_string(expr);

            str = gb_append_cstring(str, " = ");

            if (stmt->data.let.value != NULL) {
                gbString expr = ast_make_expr_str(stmt->data.let.value);
                str = gb_append_string(str, expr);
                gb_free_string(expr);
            }

            str = gb_append_cstring(str, ";");
            break;
        case ast_RET_STMT:
            str = gb_append_cstring(str, stmt->token.literal);
            str = gb_append_cstring(str, " ");

            if (stmt->data.ret.ret_val != NULL) {
                gbString expr = ast_make_expr_str(stmt->data.ret.ret_val);
                str = gb_append_string(str, expr);
                gb_free_string(expr);
            }

            str = gb_append_cstring(str, ";");
            break;
        case ast_EXPR_STMT:
            if (stmt->data.expr.expr != NULL) {
                gbString expr = ast_make_expr_str(stmt->data.expr.expr);
                str = gb_append_string(str, expr);
                gb_free_string(expr);
            }
            break;
        case ast_BLOCK_STMT:
            for (int i = 0; i < stbds_arrlen(stmt->data.block.stmts_da); ++i) {
                gbString stmt_str =
                    ast_make_stmt_str(stmt->data.block.stmts_da[i]);
                str = gb_append_string(str, stmt_str);
                gb_free_string(stmt_str);
            }
            break;
        default:
            assert(0 && "unreachable");
    }

    return str;
}

// must free after using
struct ast_Program *ast_alloc_program() {
    struct ast_Program *program = malloc(sizeof(struct ast_Program));
    program->statement_ptrs_da = NULL;
    return program;
};

// FIXME: very weird code why only the first statements token literal
char *ast_Program_token_literal(struct ast_Program *program) {
    if (stbds_arrlen(program->statement_ptrs_da) > 0) {
        struct ast_Stmt *stmt_ptr = program->statement_ptrs_da[0];
        return (stmt_ptr->token.literal);
    } else {
        return "";
    }
}

void ast_free_program(struct ast_Program *program) {
    if (program == NULL)
        return;
    // travel all statements and free each type of statement
    for (int i = 0; i < stbds_arrlen(program->statement_ptrs_da); ++i) {
        struct ast_Stmt *stmt = program->statement_ptrs_da[i];
        ast_free_stmt(stmt);
    }

    // free statement_ptrs_da with stb
    stbds_arrfree(program->statement_ptrs_da);

    // free program
    free(program);
}

// free after using
gbString ast_make_program_str(struct ast_Program *prg) {
    gbString str = gb_make_string("");

    int n = stbds_arrlen(prg->statement_ptrs_da);
    for (int i = 0; i < n; ++i) {
        gbString stmt_str = ast_make_stmt_str(prg->statement_ptrs_da[i]);
        str = gb_append_string(str, stmt_str);

        gb_free_string(stmt_str);
    }

    return str;
}

bool ast_is_expr_same(struct ast_Expr *a, struct ast_Expr *b) {
    if (a == NULL && b == NULL)
        return true;
    if (a == NULL || b == NULL)
        return false;

    // First check tag
    if (a->tag != b->tag)
        return false;

    // Then do deep comparison based on tag
    switch (a->tag) {
        case ast_IDENT_EXPR:
            return strcmp(a->data.ident.value, b->data.ident.value) == 0;

        case ast_INT_LIT_EXPR:
            return a->data.int_lit.value == b->data.int_lit.value;

        case ast_PREFIX_EXPR:
            return strcmp(a->data.pf.operator, b->data.pf.operator) == 0 &&
                   ast_is_expr_same(a->data.pf.right, b->data.pf.right);

        case ast_INFIX_EXPR:
            return strcmp(a->data.inf.operator, b->data.inf.operator) == 0 &&
                   ast_is_expr_same(a->data.inf.left, b->data.inf.left) &&
                   ast_is_expr_same(a->data.inf.right, b->data.inf.right);

        case ast_BOOL_EXPR:
            return a->data.boolean.value == b->data.boolean.value;

        case ast_IF_EXPR:
            return ast_is_expr_same(a->data.ife.cond, b->data.ife.cond) &&
                   ast_is_stmt_same(a->data.ife.conseq, b->data.ife.conseq) &&
                   ast_is_stmt_same(a->data.ife.alt, b->data.ife.alt);

        case ast_FN_LIT_EXPR: {
            if (stbds_arrlen(a->data.fn_lit.params_da) !=
                stbds_arrlen(b->data.fn_lit.params_da))
                return false;

            for (int i = 0; i < stbds_arrlen(a->data.fn_lit.params_da); i++) {
                if (!ast_is_expr_same(
                        a->data.fn_lit.params_da[i],
                        b->data.fn_lit.params_da[i]
                    ))
                    return false;
            }
            return ast_is_stmt_same(a->data.fn_lit.body, b->data.fn_lit.body);
        }

        case ast_CALL_EXPR: {
            if (!ast_is_expr_same(a->data.call.func, b->data.call.func))
                return false;

            if (stbds_arrlen(a->data.call.args_da) !=
                stbds_arrlen(b->data.call.args_da))
                return false;

            for (int i = 0; i < stbds_arrlen(a->data.call.args_da); i++) {
                if (!ast_is_expr_same(
                        a->data.call.args_da[i],
                        b->data.call.args_da[i]
                    ))
                    return false;
            }
            return true;
        }

        case ast_STR_LIT_EXPR:
            return strcmp(a->data.str.value, b->data.str.value) == 0;

        case ast_ARR_LIT_EXPR: {
            if (stbds_arrlen(a->data.arr.elems_da) !=
                stbds_arrlen(b->data.arr.elems_da))
                return false;

            for (int i = 0; i < stbds_arrlen(a->data.arr.elems_da); i++) {
                if (!ast_is_expr_same(
                        a->data.arr.elems_da[i],
                        b->data.arr.elems_da[i]
                    ))
                    return false;
            }
            return true;
        }

        case ast_IDX_EXPR:
            return ast_is_expr_same(a->data.idx.left, b->data.idx.left) &&
                   ast_is_expr_same(a->data.idx.index, b->data.idx.index);

        case ast_HASH_LIT_EXPR: {
            if (stbds_arrlen(a->data.hash.hash_da) !=
                stbds_arrlen(b->data.hash.hash_da))
                return false;

            // FIXME: hashes may not be aligned
            for (int i = 0; i < stbds_arrlen(a->data.hash.hash_da); i++) {
                if (!ast_is_expr_same(
                        a->data.hash.hash_da[i]->key,
                        b->data.hash.hash_da[i]->key
                    ) ||
                    !ast_is_expr_same(
                        a->data.hash.hash_da[i]->val,
                        b->data.hash.hash_da[i]->val
                    ))
                    return false;
            }
            return true;
        }
    }
    return false; // Unknown tag
}

bool ast_is_stmt_same(struct ast_Stmt *a, struct ast_Stmt *b) {
    if (a == NULL && b == NULL)
        return true;
    if (a == NULL || b == NULL)
        return false;

    if (a->tag != b->tag)
        return false;

    switch (a->tag) {
        case ast_LET_STMT:
            return ast_is_expr_same(a->data.let.name, b->data.let.name) &&
                   ast_is_expr_same(a->data.let.value, b->data.let.value);

        case ast_RET_STMT:
            return ast_is_expr_same(a->data.ret.ret_val, b->data.ret.ret_val);

        case ast_EXPR_STMT:
            return ast_is_expr_same(a->data.expr.expr, b->data.expr.expr);

        case ast_BLOCK_STMT: {
            if (stbds_arrlen(a->data.block.stmts_da) !=
                stbds_arrlen(b->data.block.stmts_da))
                return false;

            for (int i = 0; i < stbds_arrlen(a->data.block.stmts_da); i++) {
                if (!ast_is_stmt_same(
                        a->data.block.stmts_da[i],
                        b->data.block.stmts_da[i]
                    ))
                    return false;
            }
            return true;
        }
    }
    return false; // Unknown tag
}
