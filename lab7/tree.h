#ifndef TREE_H
#define TREE_H
#include "tac.h"

struct tree {
   int prodrule;
   char *symbolname;
   int nkids;
   struct tree *kids[9];
   struct token *leaf;
   int id;
   
   struct addr first;
   struct addr follow;
   struct addr onTrue;
   struct addr onFalse;
   int hasFirst, hasFollow, hasTrue, hasFalse;
};

struct tree *maketree(int rule, int nkids, ...);
struct tree *makeleaf(struct token *tok);
void treeprint(struct tree *t, int depth);
void print_graph(struct tree *t, char *filename);
void printsyms(struct tree *t);
void buildSymtab(struct tree *t);
void assign_first(struct tree *t);
void assign_follow(struct tree *t);
void assign_bool(struct tree *t);
#endif