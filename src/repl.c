#include "eval.c"
#include "lexer.c"
#include "parser.c"
#include "util.c"

#include <stdio.h>

enum repl_modes {
    repl_mode_LEXER,
    repl_mode_PARSER,
    repl_mode_EVAL,
};

void repl_start_lexer() {
    const char PROMPT[] = "LILAC-LEXER> ";
    for (;;) {
        printf(PROMPT);

        sstring line = "";
        fgets(line, sizeof(line), stdin);

        struct lex_Lexer lexer = lex_Lexer_create(line);

        for (struct tok_Token token = lex_next_token(&lexer);
             token.type != tok_EOF;
             token = lex_next_token(&lexer)) {

            printf(
                "token! type: %s lit: %s\n",
                tok_Token_int_enum_to_str(token.type),
                token.literal
            );
        }
    }
}

void repl_print_parser_errors(gbString *errs) {
    int n = stbds_arrlen(errs);

    char *plural = n > 1 ? "s" : "";
    printf("Parser has %d Error%s.\n", (int)n, plural);

    for (int i = 0; i < n; ++i) {
        char *msg = errs[i];
        printf("\t%s.\n", msg);
    }
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

void repl_start_parser() {
    const char PROMPT[] = "LILAC-PARSER> ";

    for (;;) {
        printf(PROMPT);

        sstring line = "";
        fgets(line, sizeof(line), stdin);

        struct lex_Lexer lexer = lex_Lexer_create(line);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();
        par_parse_program(parser, program);

        if (stbds_arrlen(parser->errors_da) != 0) {
            printf("%s", MONKEY_EEK);
            printf("Oops, the monkey sees some errors.\n");
            repl_print_parser_errors(parser->errors_da);
            continue;
        }

        gbString prg_str = ast_make_program_str(program);
        printf("%s\n", prg_str);

        gb_free_string(prg_str);
        par_free_parser(parser);
        ast_free_program(program);
    }
}

void repl_start_eval() {
    const char PROMPT[] = "LILAC> ";

    for (;;) {
        printf(PROMPT);

        sstring line = "";
        fgets(line, sizeof(line), stdin);

        struct lex_Lexer lexer = lex_Lexer_create(line);
        struct par_Parser *parser = par_alloc_parser(&lexer);
        struct ast_Program *program = ast_alloc_program();
        par_parse_program(parser, program);

        if (stbds_arrlen(parser->errors_da) != 0) {
            printf("%s", MONKEY_EEK);
            printf("Oops, the monkey sees some errors in parser.\n");
            repl_print_parser_errors(parser->errors_da);
            continue;
        }

        // TODO: Add error handling
        obj_Object *evaluated =
            eval_eval((ast_Node){ ast_NODE_PRG, .prg = program });

        if (evaluated != NULL) {
            printf("%s\n", obj_object_inspect(evaluated));
        } else {
            printf("%s", MONKEY_EEK);
            printf("Oops, the monkey sees some errors in evaluator.\n");
        }

        obj_free_object(evaluated);
        par_free_parser(parser);
        ast_free_program(program);
    }
}

void repl_start(enum repl_modes mode) {
    switch (mode) {
        case repl_mode_EVAL:
            repl_start_eval();
            break;
        case repl_mode_LEXER:
            repl_start_lexer();
            break;
        case repl_mode_PARSER:
            repl_start_parser();
            break;
        default:
            assert(0 && "unreachable");
    }
}
