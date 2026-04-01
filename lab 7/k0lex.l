%{
#include "k0gram.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

extern char *current_filename;
extern YYSTYPE yylval;
int lineno = 1;
int comment_depth = 0;

#define SETTOK(tok) \
    yytoken.category = tok; \
    yytoken.text = strdup(yytext); \
    yytoken.lineno = lineno; \
    yytoken.filename = current_filename; \
    yytoken.ival = 0; \
    yytoken.dval = 0.0; \
    yytoken.sval = NULL;
%}

%option noyywrap
%x COMMENT

%%

[\t\f\v\r ]+        { }
\n                  { lineno++; }
"//"[^\n]*          { }
^[ \t]*#.*\n        { }

"/*"                { comment_depth = 1; BEGIN(COMMENT); }
<COMMENT>"/*"       { comment_depth++; }
<COMMENT>"*/"       { comment_depth--; if(comment_depth==0) BEGIN(INITIAL); }
<COMMENT>\n         { lineno++; }
<COMMENT>.          { }
<COMMENT><<EOF>>    { printf("Unterminated comment at line %d\n", lineno); exit(1); }

"fun" {
    SETTOK(FUN);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return FUN;
}

"var" {
    SETTOK(VAR);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return VAR;
}

"if" {
    SETTOK(IF);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return IF;
}

"while" {
    SETTOK(WHILE);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return WHILE;
}

"return" {
    SETTOK(RETURN);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return RETURN;
}

"true"|"false" {
    SETTOK(BOOLEANLITERAL);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return BOOLEANLITERAL;
}

"null" {
    SETTOK(NULLLITERAL);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return NULLLITERAL;
}

\"([^\"\n]|\\.)*\" {
    SETTOK(STRINGLITERAL);

    int len = strlen(yytext);
    yytoken.sval = malloc(len-1);
    strncpy(yytoken.sval, yytext+1, len-2);
    yytoken.sval[len-2]='\0';

    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return STRINGLITERAL;
}

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)? {
    SETTOK(REALLITERAL);
    yytoken.dval = atof(yytext);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return REALLITERAL;
}

[0-9]+[eE][+-]?[0-9]+ {
    SETTOK(REALLITERAL);
    yytoken.dval = atof(yytext);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return REALLITERAL;
}

[0-9]+ {
    SETTOK(INTEGERLITERAL);
    yytoken.ival = atoi(yytext);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return INTEGERLITERAL;
}

\'([^\\\n]|\\.)\' {
    SETTOK(CHARACTERLITERAL);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return CHARACTERLITERAL;
}

"("  { SETTOK(LPAREN); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return LPAREN; }
")"  { SETTOK(RPAREN); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return RPAREN; }
"{"  { SETTOK(LBRACE); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return LBRACE; }
"}"  { SETTOK(RBRACE); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return RBRACE; }
","  { SETTOK(COMMA); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return COMMA; }
";"  { SETTOK(SEMI); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return SEMI; }
":"  { SETTOK(COLON); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return COLON; }
"."  { SETTOK(DOT); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return DOT; }

"==" { SETTOK(EQUAL_TO); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return EQUAL_TO; }
"!=" { SETTOK(NOT_EQUAL_TO); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return NOT_EQUAL_TO; }
"?"  { SETTOK(QUEST); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return QUEST; }
"="  { SETTOK(ASSIGNMENT); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return ASSIGNMENT; }
"+"  { SETTOK(PLUS); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return PLUS; }
"-"  { SETTOK(MINUS); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return MINUS; }
"*"  { SETTOK(MULT); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return MULT; }
"/"  { SETTOK(DIV); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return DIV; }
"<"  { SETTOK(LESS_THAN); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return LESS_THAN; }
">"  { SETTOK(GREATER_THAN); yylval.tokenptr=malloc(sizeof(struct token)); *yylval.tokenptr=yytoken; return GREATER_THAN; }

[A-Za-z_][A-Za-z0-9_]* {
    SETTOK(IDENTIFIER);
    yylval.tokenptr = malloc(sizeof(struct token));
    *yylval.tokenptr = yytoken;
    return IDENTIFIER;
}

. {
    printf("Illegal character '%s' at line %d\n", yytext, lineno);
    exit(1);
}

%%