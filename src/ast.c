#pragma once
#include "ast.h"

// TODO: implement with expressions in ret statement
void ast_free_expr(struct ast_Expr *expr) {
    if (expr == NULL)
        return;
    switch (expr->tag) {
        case ast_IDENT_EXPR:
        case ast_INT_LIT_EXPR:
        case ast_BOOL_EXPR:
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
                str = gb_append_cstring(str, ", ");
                gb_free_string(param);
            }
            str = gb_append_cstring(str, ") ");

            gbString body = ast_make_stmt_str(expr->data.fn_lit.body);
            str = gb_append_string(str, body);
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
            // assert might not be needed
            assert(
                (stmt->data.let.name != NULL) &&
                (stmt->data.let.name->tag == ast_IDENT_EXPR) &&
                "let_stmt should have a identifier"
            );
            free(stmt->data.let.name);
            free(stmt->data.let.value);
            break;
        case ast_RET_STMT:
            // TODO: remove NULL check after implementing expression in return
            // stmt
            if (stmt->data.ret.ret_val != NULL) {
                ast_free_expr(stmt->data.ret.ret_val);
            }
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
            stmt->data.let.name = ast_alloc_expr(ast_IDENT_EXPR);
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

// ---------------------- Program

struct ast_Program {
    // dynamic array of statement_ptrs
    struct ast_Stmt **statement_ptrs_da;
};

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
