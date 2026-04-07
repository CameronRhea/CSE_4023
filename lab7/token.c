#include <stdio.h>
#include "token.h"

struct token yytoken;

int yyerror(char *s) {
    fprintf(stderr, "%s:%d: syntax error: %s at '%s'\n",
            current_filename ? current_filename : "unknown",
            lineno,
            s,
            yytoken.text ? yytoken.text : "unknown");
    return 2;
}