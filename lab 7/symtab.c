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

void insert(SymbolTable st, char *s)
{
    if (lookup(st, s)) return; // avoid duplicates

    SymbolTableEntry e = malloc(sizeof(*e));
    e->s = strdup(s);
    e->next = st->next;
    st->next = e;
    st->nEntries++;
}

void printTable(SymbolTable st)
{
    SymbolTableEntry e = st->next;
    while (e) {
        printf("%s\n", e->s);
        e = e->next;
    }
}