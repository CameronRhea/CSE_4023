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

    printf("Opening file: %s\n", argv[fileIndex]);

    int result = yyparse();

    if (result != 0) {
        return 2; // syntax error
    }

    globalTable = mksymtab();
    insert(globalTable, "print", alctype(FUNC_TYPE));

    buildSymtab(root);
    assign_first(root);
    assign_follow(root);
    assign_bool(root);

    if (semantic_error) {
        fprintf(stderr, "Semantic error(s) found\n");
        return 3;
    }

    // =========================
    // DEBUG MODES
    // =========================
    if (treemode) {

    	treeprint(root, 0);

    	print_graph(root, "tree.dot");

    	printf("DOT graph written to tree.dot\n");

    	return 0;
	}

    if (symtabmode) {
        printf("--- symbol table for: package main ---\n");
        printTable(globalTable);
        printf("---\n");
        return 0;
    }

    // =========================
    // SAFETY CHECK (IMPORTANT FIX)
    // =========================
    if (!root) {
        fprintf(stderr, "ERROR: AST root is NULL\n");
        return 2;
    }

    // =========================
    // CODE GENERATION
    // =========================
    printf("Running gen_code...\n");
    gen_code(root);

    if (!root->code) {
        fprintf(stderr, "WARNING: No intermediate code generated\n");
    }

    // =========================
    // OUTPUT FILE (.ic)
    // =========================
    char outname[512];
    snprintf(outname, sizeof(outname), "%s.ic", argv[fileIndex]);

    FILE *out = fopen(outname, "w");
    if (!out) {
        perror("fopen");
        return 1;
    }

    printf("Writing output file: %s\n", outname);

    // REQUIRED REGIONS
    fprintf(out, ".string\n");
    fprintf(out, ".data\n");
    fprintf(out, ".code\n");

    // PRINT TAC (ONLY IF EXISTS)
    if (root->code) {
        tacprint(root->code, out);
    }

    fclose(out);
    fclose(yyin);

    return 0;
}