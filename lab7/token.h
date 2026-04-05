#ifndef TOKEN_H
#define TOKEN_H

struct token {
   int category;
   char *text;
   int lineno;
   char *filename;
   int ival;
   double dval;
   char *sval;
};

extern struct token yytoken;
extern int lineno;
extern char *current_filename;

int yyerror(char *s); 

#endif