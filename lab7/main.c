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
char *current_filename = NULL;

int main(int argc, char *argv[]) {

    int dotmode = 0;

    if (argc > 1) {

        if (strcmp(argv[1], "-dot") == 0) {
            dotmode = 1;
            yyin = fopen(argv[2], "r");
            current_filename = argv[2];
        }
        else {
            yyin = fopen(argv[1], "r");
            current_filename = argv[1];
        }

        if (!yyin) {
            perror("fopen");
            return 1;
        }
    }

    int result = yyparse();

    if (result == 0) {

        if (dotmode) {
            print_graph(root, "output.dot");
        }
        else {
        	globalTable = mksymtab();
            buildSymtab(root);
			printTable(globalTable);
			treeprint(root, 0);
        }

        return 0;
    }

    return 2;
}