#include "greatest.h"
#include "lexer_test.c"
#include "parser_test.c"

/* Add all the definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN(); /* command-line arguments, initialization. */

    RUN_SUITE(lexer_suite);
    RUN_SUITE(parser_suite);

    GREATEST_MAIN_END(); /* display results */
}
