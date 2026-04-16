#include <stdio.h>
#include "tac.h"

struct addr mkaddr(int region, int value)
{
    struct addr a;
    a.region = region;
    a.u.offset = value;
    return a;
}

struct addr mkname(char *name)
{
    struct addr a;
    a.region = R_NAME;
    a.u.name = name;
    return a;
}

int main()
{
    struct instr *code = NULL;

    code = append(code,
        gen(O_ASN,
            mkaddr(R_LOCAL, 0),
            mkaddr(R_CONST, 5),
            mkaddr(R_NONE, 0)));

    code = append(code,
        gen(O_MUL,
            mkaddr(R_LOCAL, 8),
            mkaddr(R_LOCAL, 0),
            mkaddr(R_LOCAL, 0)));

    code = append(code,
        gen(O_ADD,
            mkaddr(R_LOCAL, 16),
            mkaddr(R_LOCAL, 8),
            mkaddr(R_CONST, 1)));

    code = append(code,
        gen(O_ASN,
            mkaddr(R_LOCAL, 0),
            mkaddr(R_LOCAL, 16),
            mkaddr(R_NONE, 0)));

    code = append(code,
        gen(O_PARM,
            mkaddr(R_LOCAL, 0),
            mkaddr(R_NONE, 0),
            mkaddr(R_NONE, 0)));

    code = append(code,
        gen(O_PARM,
            mkaddr(R_GLOBAL, 0),
            mkaddr(R_NONE, 0),
            mkaddr(R_NONE, 0)));

    code = append(code,
    	gen(O_CALL,
        	mkname("printf"),
        	mkaddr(R_CONST, 2),
        	mkaddr(R_LOCAL, 24)));

    code = append(code,
        gen(O_RET,
            mkaddr(R_NONE, 0),
            mkaddr(R_NONE, 0),
            mkaddr(R_NONE, 0)));

    tacprint(code);

    return 0;
}