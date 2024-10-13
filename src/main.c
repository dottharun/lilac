#include "repl.c"

#include <stdio.h>

int main(int argc, char *argv[]) {

    printf("                  __\n");
    printf("     w  c(..)o   (\n");
    printf("      \\__(-)    __)\n");
    printf("          /\\   (\n");
    printf("         /(_)___)\n");
    printf("         w /|\n");
    printf("          | \\\n");
    printf("         m  m\n");
    printf(
        "Hello, This is the Monkey programming language - With Lilac Interpreter!.\n"
    );
    printf("Feel free to type in the commands.\n");

    // repl mode
    enum repl_modes mode = repl_mode_EVAL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--lexer") == 0) {
            mode = repl_mode_LEXER;
        } else if (strcmp(argv[i], "--parser") == 0) {
            mode = repl_mode_PARSER;
        }
    }

    repl_start(mode);
    return 0;
}
