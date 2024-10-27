#include "eval.c"
#include "lexer.c"
#include "object_env.c"
#include "parser.c"
#include "util.c"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

enum repl_modes {
    repl_mode_LEXER,
    repl_mode_PARSER,
    repl_mode_EVAL,
};

char *repl_lex_str(char *line) {
    gbString out_str = gb_make_string("");
    struct lex_Lexer lexer = lex_Lexer_create(line);

    for (struct tok_Token token = lex_next_token(&lexer); token.type != tok_EOF;
         token = lex_next_token(&lexer)) {

        sstring line;
        sprintf(
            line,
            "token! type: %s lit: %s\n",
            tok_Token_int_enum_to_str(token.type),
            token.literal
        );
        out_str = gb_append_cstring(out_str, line);
    }
    return out_str;
}

const char MONKEY_EEK[] = "\
       __    EEEK!    \n\
      /  \\   ~~|~~   \n\
     (|00|)    |      \n\
      (==)  --/       \n\
    ___||___          \n\
   / _ .. _ \\        \n\
  //  |  |  \\\\      \n\
 //   |  |   \\\\     \n\
 ||  / /\\ \\  ||     \n\
_|| _| || |_ ||_      \n\
\\|||___||___|||/\\   \n\
";

char *repl_print_parser_errors(gbString *errs) {
    gbString err_str = gb_make_string("");
    int n = stbds_arrlen(errs);

    err_str = gb_append_cstring(err_str, MONKEY_EEK);
    err_str = gb_append_cstring(
        err_str,
        "Oops, the monkey sees some errors in parser.\n"
    );

    sstring line1;
    char *plural = n > 1 ? "s" : "";
    sprintf(line1, "Parser has %d Error%s.\n", (int)n, plural);
    err_str = gb_append_cstring(err_str, line1);

    for (int i = 0; i < n; ++i) {
        char *msg = errs[i];
        sstring line;
        sprintf(line, "\t%s.\n", msg);
        err_str = gb_append_cstring(err_str, line);
    }
    return err_str;
}

char *repl_parse_str(char *line) {
    gbString out_str = gb_make_string("");

    struct lex_Lexer lexer = lex_Lexer_create(line);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    if (stbds_arrlen(parser->errors_da) != 0) {
        out_str = gb_append_cstring(
            out_str,
            repl_print_parser_errors(parser->errors_da)
        );
        return out_str;
    }

    gbString prg_str = ast_make_program_str(program);
    out_str = gb_append_cstring(out_str, prg_str != NULL ? prg_str : "");

    gb_free_string(prg_str);
    par_free_parser(parser);
    ast_free_program(program);
    return out_str;
}

obj_Env EVAL_ENV = { .store = NULL, .outer = NULL };

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
char *repl_eval_str(char *line) {
    gbString out_str = gb_make_string("");

    struct lex_Lexer lexer = lex_Lexer_create(line);
    struct par_Parser *parser = par_alloc_parser(&lexer);
    struct ast_Program *program = ast_alloc_program();
    par_parse_program(parser, program);

    if (stbds_arrlen(parser->errors_da) != 0) {
        out_str = gb_append_cstring(
            out_str,
            repl_print_parser_errors(parser->errors_da)
        );
        return out_str;
    }

    obj_Object *evaluated =
        eval_eval((ast_Node){ ast_NODE_PRG, .prg = program }, &EVAL_ENV);

    out_str = gb_append_cstring(
        out_str,
        evaluated != NULL ? obj_object_inspect(evaluated) : ""
    );

    obj_free_object(evaluated);
    par_free_parser(parser);
    ast_free_program(program);
    return out_str;
}

const char *repl_make_prompt(const enum repl_modes mode) {
    switch (mode) {
        case repl_mode_EVAL:
            return "LILAC> ";
        case repl_mode_LEXER:
            return "LILAC-LEXER> ";
        case repl_mode_PARSER:
            return "LILAC-PARSER> ";
    }
}

void repl_start(const enum repl_modes mode) {
    const char *PROMPT = repl_make_prompt(mode);

    for (;;) {
        printf("%s", PROMPT);
        sstring line = "";
        fgets(line, sizeof(line), stdin);

        char *output_str = NULL;
        switch (mode) {
            case repl_mode_EVAL:
                output_str = repl_eval_str(line);
                break;
            case repl_mode_PARSER:
                output_str = repl_parse_str(line);
                break;
            case repl_mode_LEXER:
                output_str = repl_lex_str(line);
                break;
        }

        if (strlen(output_str) != 0) {
            printf("%s\n", output_str);
        }
        gb_free_string(output_str);
    }
}
