#pragma once
#include "malloc.h"
#include "token.c"
#include "util.c"

#include <stdbool.h>

// ---------------------- Expression

// TODO: convert union to anonymous
struct ast_Expr {
    struct tok_Token token;

    enum ast_expr_tag {
        ast_IDENT_EXPR,
        ast_INT_LIT_EXPR,
        ast_PREFIX_EXPR,
        ast_INFIX_EXPR,
        ast_BOOL_EXPR,
        ast_IF_EXPR,
        ast_FN_LIT_EXPR,
        ast_CALL_EXPR,
    } tag;

    union {
        struct ast_Ident {
            sstring value;
        } ident;

        struct ast_Int_lit {
            int value;
        } int_lit;

        struct ast_Prefix {
            sstring operator;
            struct ast_Expr *right;
        } pf;

        struct ast_Infix {
            struct ast_Expr *left;
            sstring operator;
            struct ast_Expr *right;
        } inf;

        struct ast_Bool {
            bool value;
        } boolean;

        struct ast_If {
            struct ast_Expr *cond; // Condition
            struct ast_Stmt *conseq; // Consequence - always block stmt
            struct ast_Stmt *alt; // Alternative - always block stmt
        } ife;

        struct ast_Fn_lit {
            // dyn arr of identifier_ptrs -- always identifier expressions
            struct ast_Expr **params_da;
            struct ast_Stmt *body; // always block stmts
        } fn_lit;

        struct ast_Call {
            struct ast_Expr *func; // identifier or function expr
            // expr_ptrs da --not sure if its only identifiers or func exprs too
            struct ast_Expr **args_da;
        } call;
    } data;
};

// ---------------------- Statement

// TODO: convert union to anonymous
struct ast_Stmt {
    struct tok_Token token;

    enum ast_stmt_tag {
        ast_LET_STMT,
        ast_RET_STMT,
        ast_EXPR_STMT,
        ast_BLOCK_STMT,
    } tag;

    union {
        struct ast_Let_stmt {
            struct ast_Expr *name; // should always be identifier ast_IDENT_EXPR

            struct ast_Expr *value; // could be any expression
        } let;

        struct ast_Ret_stmt {
            struct ast_Expr *ret_val;
        } ret;

        struct ast_Expr_stmt {
            struct ast_Expr *expr;
        } expr;

        struct ast_Block_stmt {
            struct ast_Stmt **stmts_da; // dynamic array of stmt_ptrs
        } block;
    } data;
};

gbString ast_make_stmt_str(struct ast_Stmt *stmt);

struct ast_Stmt *ast_alloc_stmt(enum ast_stmt_tag tag);
void ast_free_stmt(struct ast_Stmt *stmt);
void ast_free_expr(struct ast_Expr *expr);

struct ast_Expr *deepcopy_expr(const struct ast_Expr *expr);
struct ast_Stmt *deepcopy_stmt(const struct ast_Stmt *stmt);
struct ast_Expr **ast_deepcpy_fn_params(struct ast_Expr **params);

struct ast_Program {
    // dynamic array of statement_ptrs
    struct ast_Stmt **statement_ptrs_da;
};

typedef struct {
    enum ast_Node_type {
        ast_NODE_EXPR,
        ast_NODE_STMT,
        ast_NODE_PRG,
    } tag;

    union {
        struct ast_Stmt *stmt;
        struct ast_Expr *expr;
        struct ast_Program *prg;
    };
} ast_Node;
