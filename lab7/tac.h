#ifndef TAC_H
#define TAC_H

#include <stdio.h>   // REQUIRED for FILE *

struct addr {
  int region;
  union {
    int offset;
    char *name;
  } u;
};

/* Regions */
#define R_GLOBAL 2001
#define R_LOCAL  2002
#define R_CLASS  2003
#define R_LABEL  2004
#define R_CONST  2005
#define R_NAME   2006
#define R_NONE   2007

struct instr {
   int opcode;
   struct addr dest, src1, src2;
   struct instr *next;
};

/* Opcodes */
#define O_ADD   3001
#define O_SUB   3002
#define O_MUL   3003
#define O_DIV   3004
#define O_NEG   3005
#define O_ASN   3006
#define O_ADDR  3007
#define O_LCONT 3008
#define O_SCONT 3009
#define O_GOTO  3010
#define O_BLT   3011
#define O_BLE   3012
#define O_BGT   3013
#define O_BGE   3014
#define O_BEQ   3015
#define O_BNE   3016
#define O_BIF   3017
#define O_BNIF  3018
#define O_PARM  3019
#define O_CALL  3020
#define O_RET   3021

/* pseudo instructions */
#define D_GLOB  3051
#define D_PROC  3052
#define D_LOCAL 3053
#define D_LABEL 3054
#define D_END   3055
#define D_PROT  3056

struct instr *gen(int, struct addr, struct addr, struct addr);
struct instr *concat(struct instr *, struct instr *);
struct instr *append(struct instr *, struct instr *);

char *regionname(int i);
char *opcodename(int i);
char *pseudoname(int i);

struct addr genlabel(void);

void tacprint(struct instr *head, FILE *out);

#endif
