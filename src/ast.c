#include "malloc.h"
#include "token.c"
#include "util.c"

#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include "stb_ds.h"

// ------------------- Interfaces

// NOTE: be careful when passing statement ptrs - statements should never be
// copied, only statement ptrs should be copied, since statements are always
// casted from other structs with more data

#define NODE_INTERFACE \
    char *(*token_literal)(void *); \
    char *(*string)(); \
    void (*free_node)(void *);

#define STATEMENT_INTERFACE \
    NODE_INTERFACE \
    void (*statement_node)(); // FIXME: weirdly not used anywhere

#define EXPRESSION_INTERFACE \
    NODE_INTERFACE \
    void (*expression_node)();

struct ast_Statement {
    STATEMENT_INTERFACE
};

struct ast_Expression {
    EXPRESSION_INTERFACE
};

// ---------------------- Program

struct ast_Program {
    // dynamic array of statement_ptrs
    struct ast_Statement **statement_ptrs_da;
};

// must free after using
struct ast_Program *ast_alloc_program() {
    struct ast_Program *program =
        (struct ast_Program *)malloc(1 * sizeof(struct ast_Program));
    program->statement_ptrs_da = NULL;
    return program;
};

// FIXME: very weird code why only the first statements token literal
char *ast_Program_token_literal(struct ast_Program *program) {
    if (stbds_arrlen(program->statement_ptrs_da) > 0) {
        struct ast_Statement *stmt_ptr = program->statement_ptrs_da[0];
        return (stmt_ptr->token_literal(stmt_ptr));
    } else {
        return "";
    }
}

void ast_free_program(struct ast_Program *program) {
    // travel all statements and free each type of statement
    for (int i = 0; i < stbds_arrlen(program->statement_ptrs_da); ++i) {
        struct ast_Statement *stmt = program->statement_ptrs_da[i];
        stmt->free_node(stmt);
    }

    // free statement_ptrs_da with stb
    stbds_arrfree(program->statement_ptrs_da);

    // free program
    free(program);
}

// ---------------------- Statement

struct ast_Let_statement {
    STATEMENT_INTERFACE
    struct tok_Token token;
    struct ast_Identifier *name;
};

char *ast_Let_statement_token_literal(void *stmt) {
    struct ast_Let_statement *let_stmt = (struct ast_Let_statement *)stmt;
    assert(
        (let_stmt->token.type == tok_LET) &&
        "this func should only receive let stmts"
    );
    return let_stmt->token.literal;
}

void ast_free_let_stmt(void *stmt) {
    struct ast_Let_statement *let_stmt = (struct ast_Let_statement *)stmt;
    assert(
        (let_stmt->token.type == tok_LET) &&
        "this func should only receive let stmts"
    );

    // free the identifier in it
    assert((let_stmt->name != NULL) && "let_stmt should have a identifier");
    free(let_stmt->name);

    // free the stmt
    free(let_stmt);
}

struct ast_Let_statement *ast_alloc_let_stmt() {
    struct ast_Let_statement *let_stmt =
        malloc(1 * sizeof(struct ast_Let_statement));
    // defaults
    let_stmt->token_literal = &ast_Let_statement_token_literal;
    let_stmt->free_node = ast_free_let_stmt;
    let_stmt->name = NULL;
    return let_stmt;
}

// ---------------------- Expression

struct ast_Identifier {
    EXPRESSION_INTERFACE
    struct tok_Token token;
    sstring value;
};

// TODO: must be added to constructor of let_statement
char *ast_Identifier_token_literal(void *exp) {
    struct ast_Identifier *ident = (struct ast_Identifier *)exp;
    assert(
        (ident->token.type == tok_IDENT) &&
        "this func should only receive identifiers"
    );
    return ident->token.literal;
}

struct ast_Identifier *ast_alloc_identifier() {
    struct ast_Identifier *ident = malloc(1 * sizeof(struct ast_Identifier));
    ident->token_literal = &ast_Let_statement_token_literal;
    return ident;
}
