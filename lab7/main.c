#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "symtab.h"

extern FILE *yyin;
extern int yyparse(void);
extern struct tree *root;
extern SymbolTable globalTable;
extern int semantic_error;

char *current_filename = NULL;

int main(int argc, char *argv[])
{
    int treemode = 0;
    int symtabmode = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./k0 [-tree|-symtab] file\n");
        return 1;
    }

    int fileIndex = 1;

    if (strcmp(argv[1], "-tree") == 0) {
        treemode = 1;
        fileIndex = 2;
    }
    else if (strcmp(argv[1], "-symtab") == 0) {
        symtabmode = 1;
        fileIndex = 2;
    }
    
    if (fileIndex >= argc) {
    fprintf(stderr, "Missing input file\n");
    return 1;
}

    yyin = fopen(argv[fileIndex], "r");
    current_filename = argv[fileIndex];

    if (!yyin) {
        perror("fopen");
        return 1;
    }

    printf("%s\n", current_filename);

    int result = yyparse();

        if (result == 0) {
        globalTable = mksymtab();
        insert(globalTable, "print", alctype(FUNC_TYPE));
        buildSymtab(root);

        if (semantic_error) {
            fprintf(stderr, "Semantic error(s) found\n");
            return 3;
        }

        if (treemode) {
            treeprint(root, 0);
        }
        else if (symtabmode) {
            printf("--- symbol table for: package main ---\n");
            printTable(globalTable);
            printf("---\n");
        }
        else {
            printf("No errors\n");
        }

        return 0;
    }

    return 2;
}