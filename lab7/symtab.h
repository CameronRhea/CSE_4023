#ifndef SYMTAB_H
#define SYMTAB_H

#include "type.h"

typedef struct sym_entry {
   char *s;
   typeptr type;
   int mutable;
   int nullable;

   struct param *params;
   typeptr returnType;

   struct sym_entry *next;
} *SymbolTableEntry;

typedef struct sym_table {
   int nEntries;
   SymbolTableEntry next;
} *SymbolTable;

SymbolTable mksymtab();
void insert(SymbolTable st, char *s, typeptr t);
int lookup(SymbolTable st, char *s);
void printTable(SymbolTable st);
SymbolTableEntry lookupEntry(SymbolTable st, char *s);

#endif