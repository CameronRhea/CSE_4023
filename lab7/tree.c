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
SymbolTableEntry currentFunction = NULL;
int semantic_error = 0;

SymbolTableEntry lookupSymbol(SymbolTable st, char *name)
{
    return lookupEntry(st, name);
}

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

    // =========================
    // LEAF
    // =========================
    if (t->leaf) {
        switch (t->leaf->category) {

            case INTEGERLITERAL: return integer_typeptr;
            case REALLITERAL: return double_typeptr;
            case BOOLEANLITERAL: return boolean_typeptr;
            case CHARACTERLITERAL: return char_typeptr;
            case STRINGLITERAL: return string_typeptr;
            case NULLLITERAL: return null_typeptr;

            case IDENTIFIER: {
                SymbolTableEntry e = lookupEntry(globalTable, t->leaf->text);

                if (!e) {
                    semantic_error = 1;
                    fprintf(stderr, "Undefined variable %s\n", t->leaf->text);
                    return NULL;
                }

                return e->type;
            }
        }
    }

    typeptr left, right;

    switch (t->prodrule) {

        // =========================
        // ARITHMETIC
        // =========================
        case 12:
        case 13:
        case 14:
        case 15:

            left = getType(t->kids[0]);
            right = getType(t->kids[2]);

            if (!left || !right) return NULL;

            if (left == null_typeptr || right == null_typeptr) {
                semantic_error = 1;
                fprintf(stderr, "Null used in arithmetic expression\n");
                return NULL;
            }

            if (left == double_typeptr || right == double_typeptr)
                return double_typeptr;

            return integer_typeptr;

        // =========================
        // RELATIONAL
        // =========================
        case 16:
        case 17:
        case 18:
        case 19:
            return boolean_typeptr;

        // =========================
        // FUNCTION CALL (FULL FIX)
        // =========================
        case 11: {

    SymbolTableEntry f =
        lookupEntry(globalTable, t->kids[0]->leaf->text);

    if (!f) {
        semantic_error = 1;
        fprintf(stderr, "Undefined function %s\n",
                t->kids[0]->leaf->text);
        return NULL;
    }

    if (!f->params && !f->returnType) {
        semantic_error = 1;
        fprintf(stderr, "%s is not a function\n", f->s);
        return NULL;
    }

    // =========================
    // ARGUMENT CHECKING (NO NULLABLE LOGIC)
    // =========================
    struct tree *arg = t->kids[2];
    struct param *param = f->params;

    while (arg && param) {

        typeptr argType = getType(arg);
        typeptr paramType = param->type;

        if (argType == null_typeptr) {
            semantic_error = 1;
            fprintf(stderr,
                "Null passed to function %s\n",
                f->s);
        }

        if (argType && paramType && argType != paramType) {
            semantic_error = 1;
            fprintf(stderr,
                "Type mismatch in call to %s\n",
                f->s);
        }

        // advance argument list
        if (arg->prodrule == 20)
            arg = arg->kids[0];
        else
            arg = NULL;

        param = param->next;
    }

    if (arg || param) {
        semantic_error = 1;
        fprintf(stderr,
            "Argument count mismatch in call to %s\n",
            f->s);
    }

    return f->returnType ? f->returnType : integer_typeptr;
}
    }

    return NULL;
}

void buildSymtab(struct tree *t)
{
    if (!t) return;

    // =========================
    // VAR DECL
    // =========================
    if (t->prodrule == 5) {

        char *name = t->kids[1]->leaf->text;
        typeptr varType = integer_typeptr;
        int isNullable = 0;

        if (t->nkids == 4) {
            typeptr rhs = getType(t->kids[3]);

            if (rhs == null_typeptr) {
                isNullable = 1;
            } else {
                varType = rhs;
            }
        }

        if (lookup(globalTable, name)) {
            semantic_error = 1;
            fprintf(stderr, "Redeclared variable %s\n", name);
        } else {
            insert(globalTable, name, varType);

            SymbolTableEntry e = lookupEntry(globalTable, name);
            e->mutable = 1;
            e->nullable = isNullable;
        }
    }

    // =========================
    // ASSIGNMENT
    // =========================
    else if (t->prodrule == 6) {

        char *name = t->kids[0]->leaf->text;
        SymbolTableEntry e = lookupEntry(globalTable, name);

        if (!e) {
            semantic_error = 1;
            fprintf(stderr, "Undefined variable %s\n", name);
        } else {

            if (!e->mutable) {
                semantic_error = 1;
                fprintf(stderr, "Immutable assignment %s\n", name);
            }

            typeptr rhs = getType(t->kids[2]);

            if (rhs == null_typeptr && !e->nullable) {
                semantic_error = 1;
                fprintf(stderr, "Null assigned to non-nullable %s\n", name);
            }

            if (rhs && e->type && rhs != e->type &&
                !(rhs == null_typeptr && e->nullable)) {
                semantic_error = 1;
                fprintf(stderr, "Type mismatch assignment %s\n", name);
            }
        }
    }

    // =========================
    // FUNCTION DECL
    // =========================
    else if (t->prodrule == 7) {

        char *fname = t->kids[1]->leaf->text;

        if (lookup(globalTable, fname)) {
            semantic_error = 1;
            fprintf(stderr, "Redeclared function %s\n", fname);
            return;
        }

        SymbolTableEntry f = malloc(sizeof(*f));

        f->s = strdup(fname);
        f->type = alctype(FUNC_TYPE);
        f->mutable = 0;
        f->nullable = 0;
        f->next = globalTable->next;

        f->params = NULL;
        f->returnType = integer_typeptr;

        currentFunction = f;

        // =========================
        // PARAMETERS
        // =========================
        struct tree *p = t->kids[3];

        struct param *head = NULL;
        struct param *tail = NULL;

        if (p && p->prodrule == 8) {

            struct tree *curr = p;

            while (curr) {

                struct param *np = malloc(sizeof(struct param));

                if (curr->nkids == 3) {
                    np->name = strdup(curr->kids[2]->leaf->text);
                    curr = curr->kids[0];
                } else {
                    np->name = strdup(curr->kids[0]->leaf->text);
                    curr = NULL;
                }

                np->type = integer_typeptr;
                np->next = NULL;

                if (!head) head = tail = np;
                else {
                    tail->next = np;
                    tail = np;
                }
            }
        }

        f->params = head;

        insert(globalTable, fname, f->type);

        SymbolTableEntry e = lookupEntry(globalTable, fname);
        e->params = head;
        e->returnType = f->returnType;
    }

    // =========================
    // RETURN STATEMENT (FIXED)
    // =========================
    else if (t->prodrule == 4) {

    if (!currentFunction) return;

    typeptr retType = getType(t->kids[1]);

    if (retType == null_typeptr) {
        semantic_error = 1;
        fprintf(stderr,
            "Null returned from function %s\n",
            currentFunction->s);
    }

    if (retType &&
        currentFunction->returnType &&
        retType != currentFunction->returnType) {

        semantic_error = 1;
        fprintf(stderr,
            "Return type mismatch in %s\n",
            currentFunction->s);
    }
}

    // =========================
    // RECURSION
    // =========================
    for (int i = 0; i < t->nkids; i++) {
        buildSymtab(t->kids[i]);
    }
}