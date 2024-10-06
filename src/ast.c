#include "malloc.h"
#include "token.c"
#include "util.c"

#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include "stb_ds.h"

#define GB_STRING_IMPLEMENTATION
#include "gb_string.h"
struct ast_Expr {
    struct tok_Token token;

    enum ast_expr_tag {
        ast_IDENT_EXPR,
    } tag;

    union {
        struct ast_Ident {
            sstring value;
        } ident;
    } data;
};

// TODO: implement with expressions in ret statement
void ast_free_expr(struct ast_Expr *expr) {
    free(expr);
};

struct ast_Expr *ast_alloc_expr(enum ast_expr_tag tag) {
    struct ast_Expr *expr = malloc(sizeof(struct ast_Expr));
    expr->tag = tag;

    // TODO: implement for all exprs
    switch (tag) {
        case ast_IDENT_EXPR:
            // NOTHING???
            break;
        default:
            assert(0 && "unreachable");
    }

    return expr;
}

struct ast_Stmt {
    struct tok_Token token;

    enum ast_stmt_tag {
        ast_LET_STMT,
        ast_RET_STMT
    } tag;

    union {
        struct ast_Let_stmt {
            struct ast_Expr *name; // should always be identifier ast_IDENT_EXPR
        } let;

        struct ast_Ret_stmt {
            struct ast_Expr *ret_val;
        } ret;
    } data;
};

void ast_free_stmt(struct ast_Stmt *stmt) {
    // TODO: free every stmt correctly

    // free all inside resources according to tag
    switch (stmt->tag) {
        case ast_LET_STMT:
            // assert might not be needed
            assert(
                (stmt->data.let.name != NULL) &&
                (stmt->data.let.name->tag == ast_IDENT_EXPR) &&
                "let_stmt should have a identifier"
            );
            free(stmt->data.let.name);
            break;
        case ast_RET_STMT:
            // TODO: remove NULL check after implementing expression in return
            // stmt
            if (stmt->data.ret.ret_val != NULL) {
                ast_free_expr(stmt->data.ret.ret_val);
            }
            break;
        default:
            assert(0 && "unreachable");
    }

    free(stmt);
}

struct ast_Stmt *ast_alloc_stmt(enum ast_stmt_tag tag) {
    struct ast_Stmt *stmt = malloc(sizeof(struct ast_Stmt));
    stmt->tag = tag;

    // TODO: implement for all stmts
    switch (tag) {
        case ast_LET_STMT:
            stmt->data.let.name = ast_alloc_expr(ast_IDENT_EXPR);
            break;
        case ast_RET_STMT:
            stmt->data.ret.ret_val = NULL;
            break;
        default:
            assert(0 && "unreachable");
    }

    return stmt;
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
