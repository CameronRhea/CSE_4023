#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable mksymtab()
{
    SymbolTable st = malloc(sizeof(*st));
    st->nEntries = 0;
    st->next = NULL;
    return st;
}

int lookup(SymbolTable st, char *s)
{
    SymbolTableEntry e = st->next;
    while (e) {
        if (strcmp(e->s, s) == 0) return 1;
        e = e->next;
    }
    return 0;
}

void insert(SymbolTable st, char *s, typeptr t)
{
    if (lookup(st, s)) return;

    SymbolTableEntry e = malloc(sizeof(*e));

    e->s = strdup(s);
    e->type = t;

    e->mutable = 1;
    e->nullable = 0;

    e->params = NULL;
    e->returnType = NULL;

    e->next = st->next;
    st->next = e;

    st->nEntries++;
}

void printTable(SymbolTable st)
{
    SymbolTableEntry e = st->next;
    while (e) {
        printf("%s : %s\n", e->s, typename(e->type));
        e = e->next;
    }
}

SymbolTableEntry lookupEntry(SymbolTable st, char *s)
{
    SymbolTableEntry e = st->next;
    while (e) {
        if (strcmp(e->s, s) == 0) return e;
        e = e->next;
    }
    return NULL;
}