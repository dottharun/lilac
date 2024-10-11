#include "greatest.h"

#include "../src/ast.c"

SUITE(ast_suite);

TEST ast_test_program_string(void) {

    struct ast_Stmt *let_stmt = ast_alloc_stmt(ast_LET_STMT);
    let_stmt->token.type = tok_LET;
    strcpy(let_stmt->token.literal, "let");

    let_stmt->data.let.name = ast_alloc_expr(ast_IDENT_EXPR);
    let_stmt->data.let.name->token.type = tok_IDENT;
    strcpy(let_stmt->data.let.name->token.literal, "myVar");
    strcpy(let_stmt->data.let.name->data.ident.value, "myVar");

    // FIXME: maybe needed to be included in the constructor  itself
    let_stmt->data.let.value = ast_alloc_expr(ast_IDENT_EXPR);
    strcpy(let_stmt->data.let.value->token.literal, "anotherVar");
    strcpy(let_stmt->data.let.value->data.ident.value, "anotherVar");

    struct ast_Stmt **stmts = NULL;
    stbds_arrput(stmts, let_stmt);

    struct ast_Program prg = { .statement_ptrs_da = stmts };

    gbString prg_str = ast_make_program_str(&prg);
    ASSERT_STR_EQ("let myVar = anotherVar;", prg_str);

    ast_free_stmt(let_stmt);
    gb_free_string(prg_str);
    PASS();
}

SUITE(ast_suite) {
    RUN_TEST(ast_test_program_string);
}
