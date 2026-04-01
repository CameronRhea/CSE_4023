#ifndef TREE_H
#define TREE_H

struct tree {
   int prodrule;
   char *symbolname;
   int nkids;
   struct tree *kids[9];
   struct token *leaf;
   int id;
};

struct tree *maketree(int rule, int nkids, ...);
struct tree *makeleaf(struct token *tok);
void treeprint(struct tree *t, int depth);
void print_graph(struct tree *t, char *filename);
void printsyms(struct tree *t);
void buildSymtab(struct tree *t);

#endif