#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "tree.h"
#include "token.h"
#include "symtab.h"
#include "k0gram.tab.h"

int serial = 0;
SymbolTable globalTable = NULL;
int semantic_error = 0;

struct tree *maketree(int rule, int nkids, ...) {
    struct tree *t = malloc(sizeof(struct tree));
    if (!t) {
        perror("malloc");
        exit(1);
    }

    t->prodrule = rule;
    t->symbolname = NULL;
    t->nkids = nkids;
    t->leaf = NULL;
    t->id = serial++;

    for (int i = 0; i < 9; i++) {
        t->kids[i] = NULL;
    }

    va_list args;
    va_start(args, nkids);

    for (int i = 0; i < nkids; i++) {
        t->kids[i] = va_arg(args, struct tree *);
    }

    va_end(args);

    return t;
}

struct tree *makeleaf(struct token *tok) {
    struct tree *t = malloc(sizeof(struct tree));
    t->prodrule = 0;
    t->symbolname = NULL;
    t->nkids = 0;
    t->leaf = tok;
    t->id = serial++;
    return t;
}

static char *humanreadable(struct tree *t) {
    if (t->leaf) {
        return t->leaf->text;
    }

    switch(t->prodrule) {
        case 1: return "statements";
        case 2: return "varDeclarationStmt";
        case 3: return "assignmentStmt";
        case 4: return "returnStmt";
        case 5: return "varDeclaration";
        case 6: return "assignment";
        case 7: return "functionDeclaration";
        case 8: return "parameterList";
        case 9: return "block";
        case 10: return "whileLoop";
        case 11: return "functionCall";
        case 12: return "plusExpr";
        case 13: return "minusExpr";
        case 14: return "multExpr";
        case 15: return "divExpr";
        case 16: return "lessExpr";
        case 17: return "greaterExpr";
        case 18: return "equalExpr";
        case 19: return "notEqualExpr";
        case 20: return "argumentList";
        case 21: return "anonymousFunction";
        default: return "node";
    }
}

void treeprint(struct tree *t, int depth)
{
    if (!t) return;

    if (t->leaf) {
        printf("%*sTOKEN(%d): %s",
               depth * 2, "",
               t->leaf->category,
               t->leaf->text);

        if (t->leaf->category == IDENTIFIER && globalTable) {
            SymbolTableEntry e = globalTable->next;
            while (e) {
                if (strcmp(e->s, t->leaf->text) == 0) {
                    printf(" : %s", typename(e->type));
                    break;
                }
                e = e->next;
            }
        }

        printf("\n");
    }
    else {
        printf("%*s%s\n", depth * 2, "", humanreadable(t));

        for (int i = 0; i < t->nkids; i++) {
            treeprint(t->kids[i], depth + 1);
        }
    }
}

void print_graph2(struct tree *t, FILE *f) {

    if (!t) return;

    if (t->leaf) {
        fprintf(f,"N%d [shape=box label=\"%s\"];\n",
                t->id,
                t->leaf->text);
        return;
    }

    fprintf(f,"N%d [label=\"%s\"];\n",
            t->id,
            humanreadable(t));

    for (int i = 0; i < t->nkids; i++) {

        if (t->kids[i]) {
            fprintf(f,"N%d -> N%d;\n", t->id, t->kids[i]->id);
            print_graph2(t->kids[i], f);
        }

    }
}

void print_graph(struct tree *t, char *filename) {

    FILE *f = fopen(filename,"w");

    fprintf(f,"digraph {\n");

    print_graph2(t,f);

    fprintf(f,"}\n");

    fclose(f);
}

void printsymbol(char *s)
{
    printf("%s\n", s);
    fflush(stdout);
}

void printsyms(struct tree *t)
{
    if (!t) return;

    if (t->leaf && t->leaf->category == IDENTIFIER) {
        printsymbol(t->leaf->text);
    }

    for (int i = 0; i < t->nkids; i++) {
        printsyms(t->kids[i]);
    }
}

typeptr getType(struct tree *t)
{
    if (!t) return null_typeptr;

    if (t->leaf) {
        switch (t->leaf->category) {
            case INTEGERLITERAL:
                return integer_typeptr;

            case REALLITERAL:
                return double_typeptr;

            case BOOLEANLITERAL:
                return boolean_typeptr;

            case CHARACTERLITERAL:
                return char_typeptr;

            case NULLLITERAL:
                return null_typeptr;

            default:
                return NULL;
        }
    }

    // basic arithmetic expressions
    if (t->prodrule >= 12 && t->prodrule <= 15) {
        return integer_typeptr;
    }

    return NULL;
}

void buildSymtab(struct tree *t)
{
    if (!t) return;

    if (t->prodrule == 5) {   // varDeclaration
        if (t->nkids >= 2 && t->kids[1] && t->kids[1]->leaf) {
            char *name = t->kids[1]->leaf->text;

            typeptr varType = NULL;

            // if declaration has assignment
            if (t->nkids == 4) {
                varType = getType(t->kids[3]);
            }

            if (lookup(globalTable, name)) {
    			fprintf(stderr, "Semantic error: redeclared variable %s\n", name);
    			semantic_error = 1;
			}
			else {
    			insert(globalTable, name, varType);
			}
        }
    }

    for (int i = 0; i < t->nkids; i++) {
        buildSymtab(t->kids[i]);
    }
}