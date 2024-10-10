#include "malloc.h"
#include "token.c"
#include "util.c"

#include <stdbool.h>

// ---------------------- Expression

struct ast_Expr {
    struct tok_Token token;

    enum ast_expr_tag {
        ast_IDENT_EXPR,
        ast_INT_LIT_EXPR,
        ast_PREFIX_EXPR,
        ast_INFIX_EXPR,
        ast_BOOL_EXPR,
        ast_IF_EXPR
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
    } data;
};

// ---------------------- Statement

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

void ast_free_stmt(struct ast_Stmt *stmt);
