#include "type.h"

#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct sym_entry {
   char *s;
   typeptr type; 
   struct sym_entry *next;
} *SymbolTableEntry;

typedef struct sym_table {
   int nEntries;
   struct sym_entry *next;
} *SymbolTable;

SymbolTable mksymtab();
void insert(SymbolTable st, char *s, typeptr t);
int lookup(SymbolTable st, char *s);
void printTable(SymbolTable st);

#endif