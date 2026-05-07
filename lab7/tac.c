#include <stdio.h>
#include <stdlib.h>
#include "tac.h"

char *regionnames[] = {"global","loc", "class", "lab", "const", "", "none"};
char *regionname(int i) { return regionnames[i-R_GLOBAL]; }

char *opcodenames[] = {
   "ADD","SUB", "MUL", "DIV", "NEG", "ASN", "ADDR", "LCONT", "SCONT", "GOTO",
   "BLT", "BLE", "BGT", "BGE", "BEQ", "BNE", "BIF", "BNIF", "PARM", "CALL",
   "RETURN"
};

char *opcodename(int i) { return opcodenames[i-O_ADD]; }

char *pseudonames[] = {
   "glob","proc", "loc", "lab", "end", "prot"
};

char *pseudoname(int i) { return pseudonames[i-D_GLOB]; }

int labelcounter = 0;

struct addr genlabel(void)
{
    struct addr a;
    a.region = R_LABEL;
    a.u.offset = labelcounter++;
    return a;
}

struct instr *gen(int op, struct addr a1, struct addr a2, struct addr a3)
{
    struct instr *rv = malloc(sizeof(struct instr));
    if (!rv) {
        fprintf(stderr, "out of memory\n");
        exit(4);
    }

    rv->opcode = op;
    rv->dest = a1;
    rv->src1 = a2;
    rv->src2 = a3;
    rv->next = NULL;

    return rv;
}

struct instr *copylist(struct instr *l)
{
    if (!l) return NULL;

    struct instr *lcopy = gen(l->opcode, l->dest, l->src1, l->src2);
    lcopy->next = copylist(l->next);
    return lcopy;
}

struct instr *append(struct instr *l1, struct instr *l2)
{
    if (!l1) return l2;

    struct instr *tmp = l1;
    while (tmp->next) tmp = tmp->next;

    tmp->next = l2;
    return l1;
}

struct instr *concat(struct instr *l1, struct instr *l2)
{
    return append(copylist(l1), l2);
}

static void printaddr_file(struct addr a, FILE *out)
{
    switch (a.region) {
        case R_LOCAL:
        case R_GLOBAL:
        case R_CLASS:
        case R_LABEL:
        case R_CONST:
            fprintf(out, "%s:%d", regionname(a.region), a.u.offset);
            break;

        case R_NAME:
            fprintf(out, "%s", a.u.name);
            break;

        case R_NONE:
            fprintf(out, "none");
            break;

        default:
            fprintf(out, "?");
    }
}

void tacprint(struct instr *head, FILE *out)
{
    struct instr *curr = head;

    while (curr != NULL) {

        fprintf(out, "%s\t", opcodename(curr->opcode));

        printaddr_file(curr->dest, out);

        if (curr->src1.region != R_NONE) {
            fprintf(out, ",");
            printaddr_file(curr->src1, out);
        }

        if (curr->src2.region != R_NONE) {
            fprintf(out, ",");
            printaddr_file(curr->src2, out);
        }

        fprintf(out, "\n");

        curr = curr->next;
    }
}
