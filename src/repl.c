#include "lexer.c"
#include "util.c"

#include <stdio.h>

#define PROMPT "MONKE> "

void repl_start() {
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
