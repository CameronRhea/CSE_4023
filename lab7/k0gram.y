%{
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "token.h"

extern int yylex(void);
extern int yyerror(char *s);

struct tree *root;

%}

%union {
    struct tree *treeptr;
    struct token *tokenptr;
}

%token <tokenptr> FUN VAR IF WHILE RETURN
%token <tokenptr> BOOLEANLITERAL NULLLITERAL STRINGLITERAL REALLITERAL INTEGERLITERAL CHARACTERLITERAL
%token <tokenptr> LPAREN RPAREN LBRACE RBRACE COMMA SEMI COLON DOT
%token <tokenptr> EQUAL_TO NOT_EQUAL_TO QUEST ASSIGNMENT PLUS MINUS MULT DIV LESS_THAN GREATER_THAN
%token <tokenptr> IDENTIFIER

%type <treeptr> program statements statement expression expression_opt
%type <treeptr> varDeclaration assignment functionDeclaration
%type <treeptr> parameters parameterList functionBody block
%type <treeptr> loopStatement argumentList anonymousFunction

%left PLUS MINUS
%left MULT DIV
%left LESS_THAN GREATER_THAN EQUAL_TO NOT_EQUAL_TO

%%
program:
    statements
    {
        root = $1;
        $$ = $1;
    }
;

statements:
      /* empty */
      {
          $$ = NULL;
      }
    | statements statement
      {
          $$ = maketree(1, 2, $1, $2);
      }
;

statement:
      varDeclaration SEMI
      {
          $$ = maketree(2, 2, $1, makeleaf($2));
      }
    | assignment SEMI
      {
          $$ = maketree(3, 2, $1, makeleaf($2));
      }
    | functionDeclaration
      {
          $$ = $1;
      }
    | loopStatement
      {
          $$ = $1;
      }
    | RETURN expression_opt SEMI
      {
          $$ = maketree(4, 3, makeleaf($1), $2 ? $2 : NULL, makeleaf($3));
      }
;

expression_opt:
      expression
      {
          $$ = $1;
      }
    | /* empty */
      {
          $$ = NULL;
      }
;

varDeclaration:
      VAR IDENTIFIER ASSIGNMENT expression
      {
          $$ = maketree(5, 4,
                        makeleaf($1),
                        makeleaf($2),
                        makeleaf($3),
                        $4);
      }
    | VAR IDENTIFIER ASSIGNMENT anonymousFunction
      {
          $$ = maketree(5, 4,
                        makeleaf($1),
                        makeleaf($2),
                        makeleaf($3),
                        $4);
      }
    | VAR IDENTIFIER
      {
          $$ = maketree(5, 2,
                        makeleaf($1),
                        makeleaf($2));
      }
;

assignment:
      IDENTIFIER ASSIGNMENT expression
      {
          $$ = maketree(6, 3,
                        makeleaf($1),
                        makeleaf($2),
                        $3);
      }
;

functionDeclaration:
      FUN IDENTIFIER LPAREN parameters RPAREN functionBody
      {
          $$ = maketree(7, 6,
                        makeleaf($1),
                        makeleaf($2),
                        makeleaf($3),
                        $4,
                        makeleaf($5),
                        $6);
      }
;

parameters:
      /* empty */
      {
          $$ = NULL;
      }
    | parameterList
      {
          $$ = $1;
      }
;

parameterList:
      IDENTIFIER
      {
          $$ = makeleaf($1);
      }
    | parameterList COMMA IDENTIFIER
      {
          $$ = maketree(8, 3,
                        $1,
                        makeleaf($2),
                        makeleaf($3));
      }
;

functionBody:
      block
      {
          $$ = $1;
      }
;

anonymousFunction:
      FUN LPAREN parameters RPAREN functionBody
      {
          $$ = maketree(21, 5, makeleaf($1), makeleaf($2), $3, makeleaf($4), $5);
      }
;

block:
      LBRACE statements RBRACE
      {
          $$ = maketree(9, 3,
                        makeleaf($1),
                        $2,
                        makeleaf($3));
      }
;

loopStatement:
      WHILE LPAREN expression RPAREN block
      {
          $$ = maketree(10, 5,
                        makeleaf($1),
                        makeleaf($2),
                        $3,
                        makeleaf($4),
                        $5);
      }
    | WHILE expression block
      {
          $$ = maketree(10, 3,
                        makeleaf($1),
                        $2,
                        $3);
      }
;

expression:
      IDENTIFIER
      {
          $$ = makeleaf($1);
      }
    | IDENTIFIER LPAREN argumentList RPAREN
      {
          $$ = maketree(11, 4,
                        makeleaf($1),
                        makeleaf($2),
                        $3,
                        makeleaf($4));
      }
    | INTEGERLITERAL
      {
          $$ = makeleaf($1);
      }
    | REALLITERAL
      {
          $$ = makeleaf($1);
      }
    | STRINGLITERAL
      {
          $$ = makeleaf($1);
      }
    | BOOLEANLITERAL
      {
          $$ = makeleaf($1);
      }
    | NULLLITERAL
      {
          $$ = makeleaf($1);
      }
    | LPAREN expression RPAREN
      {
          $$ = $2;
      }
    | expression PLUS expression
      {
          $$ = maketree(12, 3, $1, makeleaf($2), $3);
      }
    | expression MINUS expression
      {
          $$ = maketree(13, 3, $1, makeleaf($2), $3);
      }
    | expression MULT expression
      {
          $$ = maketree(14, 3, $1, makeleaf($2), $3);
      }
    | expression DIV expression
      {
          $$ = maketree(15, 3, $1, makeleaf($2), $3);
      }
    | expression LESS_THAN expression
      {
          $$ = maketree(16, 3, $1, makeleaf($2), $3);
      }
    | expression GREATER_THAN expression
      {
          $$ = maketree(17, 3, $1, makeleaf($2), $3);
      }
    | expression EQUAL_TO expression
      {
          $$ = maketree(18, 3, $1, makeleaf($2), $3);
      }
    | expression NOT_EQUAL_TO expression
      {
          $$ = maketree(19, 3, $1, makeleaf($2), $3);
      }
    | anonymousFunction
  	  {
      	  $$ = $1;
  	  }
;

argumentList:
      /* empty */
      {
          $$ = NULL;
      }
    | expression
      {
          $$ = $1;
      }
    | argumentList COMMA expression
      {
          $$ = maketree(20, 3,
                        $1,
                        makeleaf($2),
                        $3);
      }
;
%%