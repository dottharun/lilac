#include "repl.c"

#include <stdio.h>

int main() {
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
    repl_start();
    return 0;
}
