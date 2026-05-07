#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "tree.h"
#include "token.h"
#include "symtab.h"
#include "k0gram.tab.h"

int serial = 0;
SymbolTable globalTable = NULL;
SymbolTableEntry currentFunction = NULL;
int semantic_error = 0;
int local_offset = 0;
int temp_offset = 10000;

typeptr getType(struct tree *t);

SymbolTableEntry lookupSymbol(SymbolTable st, char *name)
{
    return lookupEntry(st, name);
}

struct tree *maketree(int rule, int nkids, ...) {
    struct tree *t = malloc(sizeof(struct tree));
    if (!t) {
        perror("malloc");
        exit(1);
    }

    t->prodrule = rule;
    t->symbolname = NULL;
    t->nkids = nkids;
    t->leaf = NULL;
    t->id = serial++;
    t->first.region = R_NONE;
	t->follow.region = R_NONE;
	t->onTrue.region = R_NONE;
	t->onFalse.region = R_NONE;
    t->hasFirst = t->hasFollow = t->hasTrue = t->hasFalse = 0;

    for (int i = 0; i < 9; i++) {
        t->kids[i] = NULL;
    }

    va_list args;
    va_start(args, nkids);

    for (int i = 0; i < nkids; i++) {
        t->kids[i] = va_arg(args, struct tree *);
    }

    va_end(args);

    return t;
}

struct tree *makeleaf(struct token *tok) {
    struct tree *t = malloc(sizeof(struct tree));
    t->prodrule = 0;
    t->symbolname = NULL;
    t->nkids = 0;
    t->leaf = tok;
    t->id = serial++;
    t->first.region = R_NONE;
	t->follow.region = R_NONE;
	t->onTrue.region = R_NONE;
	t->onFalse.region = R_NONE;
    return t;
}

static char *humanreadable(struct tree *t) {
    if (t->leaf) {
        return t->leaf->text;
    }

    switch(t->prodrule) {
        case 1: return "statements";
        case 2: return "varDeclarationStmt";
        case 3: return "assignmentStmt";
        case 4: return "returnStmt";
        case 5: return "varDeclaration";
        case 6: return "assignment";
        case 7: return "functionDeclaration";
        case 8: return "parameterList";
        case 9: return "block";
        case 10: return "whileLoop";
        case 11: return "functionCall";
        case 12: return "plusExpr";
        case 13: return "minusExpr";
        case 14: return "multExpr";
        case 15: return "divExpr";
        case 16: return "lessExpr";
        case 17: return "greaterExpr";
        case 18: return "equalExpr";
        case 19: return "notEqualExpr";
        case 20: return "argumentList";
        case 21: return "anonymousFunction";
        default: return "node";
    }
}

void treeprint(struct tree *t, int depth)
{
    if (!t) return;

    if (t->leaf) {
        printf("%*sTOKEN(%d): %s",
               depth * 2, "",
               t->leaf->category,
               t->leaf->text);

        if (t->leaf->category == IDENTIFIER && globalTable) {
            SymbolTableEntry e = globalTable->next;
            while (e) {
                if (strcmp(e->s, t->leaf->text) == 0) {
                    printf(" : %s", typename(e->type));
                    break;
                }
                e = e->next;
            }
        }

        printf("\n");
    }
    else {
        printf("%*s%s\n", depth * 2, "", humanreadable(t));

        for (int i = 0; i < t->nkids; i++) {
            treeprint(t->kids[i], depth + 1);
        }
    }
}

static void print_typeinfo(FILE *f, struct tree *t)
{
    typeptr ty = getType(t);

    // don't print unknown types
    if (!ty)
        return;

    fprintf(f, "\\nType: %s", typename(ty));

    // non-atomic types
    if (ty->basetype == FUNC_TYPE) {

        fprintf(f,
            "\\n(struct typeinfo)"
            "\\nparams=%d",
            ty->u.f.nparams);
    }

    else if (ty->basetype == ARRAY_TYPE) {

        fprintf(f,
            "\\n(struct typeinfo)"
            "\\nsize=%d",
            ty->u.a.size);

        if (ty->u.a.elemtype) {
            fprintf(f,
                "\\nelem=%s",
                typename(ty->u.a.elemtype));
        }
    }
}

void print_graph2(struct tree *t, FILE *f)
{
    if (!t) return;

    // =========================
    // LEAF NODE
    // =========================
    if (t->leaf) {

        fprintf(f,
            "N%d [shape=box label=\"%s",
            t->id,
            t->leaf->text);

        print_typeinfo(f, t);

        fprintf(f, "\"];\n");

        return;
    }

    // =========================
    // INTERNAL NODE
    // =========================
    fprintf(f,
        "N%d [label=\"%s",
        t->id,
        humanreadable(t));

    print_typeinfo(f, t);

    fprintf(f, "\"];\n");

    // =========================
    // EDGES
    // =========================
    for (int i = 0; i < t->nkids; i++) {

        if (t->kids[i]) {

            fprintf(f,
                "N%d -> N%d;\n",
                t->id,
                t->kids[i]->id);

            print_graph2(t->kids[i], f);
        }
    }
}

void print_graph(struct tree *t, char *filename) {

    FILE *f = fopen(filename,"w");

    fprintf(f,"digraph {\n");

    print_graph2(t,f);

    fprintf(f,"}\n");

    fclose(f);
}

void printsymbol(char *s)
{
    printf("%s\n", s);
    fflush(stdout);
}

void printsyms(struct tree *t)
{
    if (!t) return;

    if (t->leaf && t->leaf->category == IDENTIFIER) {
        printsymbol(t->leaf->text);
    }

    for (int i = 0; i < t->nkids; i++) {
        printsyms(t->kids[i]);
    }
}

typeptr getType(struct tree *t)
{
    if (!t) return null_typeptr;

    // =========================
    // LEAF
    // =========================
    if (t->leaf) {
        switch (t->leaf->category) {

            case INTEGERLITERAL: return integer_typeptr;
            case REALLITERAL: return double_typeptr;
            case BOOLEANLITERAL: return boolean_typeptr;
            case CHARACTERLITERAL: return char_typeptr;
            case STRINGLITERAL: return string_typeptr;
            case NULLLITERAL: return null_typeptr;

            case IDENTIFIER: {
                SymbolTableEntry e = lookupEntry(globalTable, t->leaf->text);

                if (!e) {
                    semantic_error = 1;
                    fprintf(stderr, "Undefined variable %s\n", t->leaf->text);
                    return NULL;
                }

                return e->type;
            }
        }
    }

    typeptr left, right;

    switch (t->prodrule) {

        // =========================
        // ARITHMETIC
        // =========================
        case 12:
        case 13:
        case 14:
        case 15:

            left = getType(t->kids[0]);
            right = getType(t->kids[2]);

            if (!left || !right) return NULL;

            if (left == null_typeptr || right == null_typeptr) {
                semantic_error = 1;
                fprintf(stderr, "Null used in arithmetic expression\n");
                return NULL;
            }

            if (left == double_typeptr || right == double_typeptr)
                return double_typeptr;

            return integer_typeptr;

        // =========================
        // RELATIONAL
        // =========================
        case 16:
        case 17:
        case 18:
        case 19:
            return boolean_typeptr;

        // =========================
        // FUNCTION CALL (FULL FIX)
        // =========================
        case 11: {

    SymbolTableEntry f =
        lookupEntry(globalTable, t->kids[0]->leaf->text);

    if (!f) {
        semantic_error = 1;
        fprintf(stderr, "Undefined function %s\n",
                t->kids[0]->leaf->text);
        return NULL;
    }

    if (!f->params && !f->returnType) {
        semantic_error = 1;
        fprintf(stderr, "%s is not a function\n", f->s);
        return NULL;
    }

    // =========================
    // ARGUMENT CHECKING (NO NULLABLE LOGIC)
    // =========================
    struct tree *arg = t->kids[2];
    struct param *param = f->params;

    while (arg && param) {

        typeptr argType = getType(arg);
        typeptr paramType = param->type;

        if (argType == null_typeptr) {
            semantic_error = 1;
            fprintf(stderr,
                "Null passed to function %s\n",
                f->s);
        }

        if (argType && paramType && argType != paramType) {
            semantic_error = 1;
            fprintf(stderr,
                "Type mismatch in call to %s\n",
                f->s);
        }

        // advance argument list
        if (arg->prodrule == 20)
            arg = arg->kids[0];
        else
            arg = NULL;

        param = param->next;
    }

    if (arg || param) {
        semantic_error = 1;
        fprintf(stderr,
            "Argument count mismatch in call to %s\n",
            f->s);
    }

    return f->returnType ? f->returnType : integer_typeptr;
}
    }

    return NULL;
}

void buildSymtab(struct tree *t)
{
    if (!t) return;

    // =========================
    // VAR DECL (RULE 5)
    // =========================
    if (t->prodrule == 5) {

        char *name = t->kids[1]->leaf->text;
        typeptr varType = integer_typeptr;
        int isNullable = 0;

        if (t->nkids == 4) {
            typeptr rhs = getType(t->kids[3]);

            if (rhs == null_typeptr) {
                isNullable = 1;
            } else {
                varType = rhs;
            }
        }

        if (lookup(globalTable, name)) {
            semantic_error = 1;
            fprintf(stderr, "Redeclared variable %s\n", name);
        } else {
            insert(globalTable, name, varType);

            SymbolTableEntry e = lookupEntry(globalTable, name);
            if (e) {
                e->offset = local_offset;
                e->mutable = 1;
                e->nullable = isNullable;
            }

            local_offset += 8;
        }
    }

    // =========================
    // ASSIGNMENT (RULE 6 handled structurally, but symbol table doesn't care)
    // =========================

    else if (t->prodrule == 6) {

        char *name = t->kids[0]->leaf->text;
        SymbolTableEntry e = lookupEntry(globalTable, name);

        if (!e) {
            semantic_error = 1;
            fprintf(stderr, "Undefined variable %s\n", name);
        } else {

            if (!e->mutable) {
                semantic_error = 1;
                fprintf(stderr, "Immutable assignment %s\n", name);
            }

            typeptr rhs = getType(t->kids[2]);

            if (rhs == null_typeptr && !e->nullable) {
                semantic_error = 1;
                fprintf(stderr, "Null assigned to non-nullable %s\n", name);
            }

            if (rhs && e->type && rhs != e->type &&
                !(rhs == null_typeptr && e->nullable)) {
                semantic_error = 1;
                fprintf(stderr, "Type mismatch assignment %s\n", name);
            }
        }
    }

    // =========================
    // FUNCTION DECL (RULE 7)
    // =========================
    else if (t->prodrule == 7) {

        char *fname = t->kids[1]->leaf->text;
        local_offset = 0;

        if (lookup(globalTable, fname)) {
            semantic_error = 1;
            fprintf(stderr, "Redeclared function %s\n", fname);
            return;
        }

        SymbolTableEntry f = malloc(sizeof(*f));
        f->s = strdup(fname);
        f->type = alctype(FUNC_TYPE);
        f->mutable = 0;
        f->nullable = 0;
        f->params = NULL;
        f->returnType = integer_typeptr;

        insert(globalTable, fname, f->type);

        SymbolTableEntry e = lookupEntry(globalTable, fname);
        if (e) {
            e->params = f->params;
            e->returnType = f->returnType;
        }

        currentFunction = f;
    }

    // =========================
    // RETURN (RULE 4)
    // =========================
    else if (t->prodrule == 4) {

        if (!currentFunction) return;

        typeptr retType = getType(t->kids[1]);

        if (retType == null_typeptr) {
            semantic_error = 1;
            fprintf(stderr,
                "Null returned from function %s\n",
                currentFunction->s);
        }

        if (retType &&
            currentFunction->returnType &&
            retType != currentFunction->returnType) {

            semantic_error = 1;
            fprintf(stderr,
                "Return type mismatch in %s\n",
                currentFunction->s);
        }
    }

    // =========================
    // RECURSION
    // =========================
    for (int i = 0; i < t->nkids; i++) {
        buildSymtab(t->kids[i]);
    }
}


int labelCounter = 0;

struct addr newtemp() {
    struct addr a;
    a.region = R_LOCAL;
    a.u.offset = temp_offset;
    temp_offset += 8;
    return a;
}

void assign_first(struct tree *t)
{
    if (!t) return;

    for (int i = 0; i < t->nkids; i++)
        assign_first(t->kids[i]);

    switch (t->prodrule) {

        case 10: // while
        case 2:  // varDecl stmt
        case 3:  // assignment stmt
        case 4:  // return
            t->first = genlabel();
            break;
    }
    
    if (t->first.region != R_NONE) {
    	printf("Node %d assigned FIRST label L%d\n",
           t->id, t->first.u.offset);
		}
}

void assign_follow(struct tree *t)
{
    if (!t) return;

    switch(t->prodrule) {

        case 1: // statements → statements statement
            if (t->kids[0] && t->kids[1]) {
                t->kids[0]->follow = t->kids[1]->first;
                t->kids[1]->follow = t->follow;
            }
            break;

        case 10: // while
			{
    			struct tree *cond = NULL;
    			struct tree *body = NULL;

    			if (t->nkids == 5) {
        			cond = t->kids[2];
        			body = t->kids[4];
    			} else if (t->nkids == 3) {
        			cond = t->kids[1];
        			body = t->kids[2];
    			}
        		
        		struct addr empty = { .region = R_NONE };
        		
        		if (cond) {

					cond->onTrue = body ? body->first : empty;
        			cond->onFalse = t->follow;
    				}

    			if (body)
        		body->follow = t->first;

    			break;
			}
	}

    for (int i = 0; i < t->nkids; i++)
        assign_follow(t->kids[i]);
        
    if (t->prodrule == 10 &&
    t->first.region != R_NONE &&
    t->follow.region != R_NONE) {
    	printf("WHILE node %d: first=L%d follow=L%d\n",
           	t->id,
           	t->first.u.offset,
           	t->follow.u.offset);
	}
}

void assign_bool(struct tree *t)
{
    if (!t) return;

    switch (t->prodrule) {

        case 16: // <
        case 17: // >
        case 18: // ==
        case 19: // !=

            if (t->kids[0]) {
                t->kids[0]->onTrue = t->onTrue;
                t->kids[0]->onFalse = t->onFalse;
            }

            if (t->kids[2]) {
                t->kids[2]->onTrue = t->onTrue;
                t->kids[2]->onFalse = t->onFalse;
            }

            break;
    }

    for (int i = 0; i < t->nkids; i++)
        assign_bool(t->kids[i]);
}

void gen_code(struct tree *t)
{
    if (!t) return;
    if (semantic_error) return;

    for (int i = 0; i < t->nkids; i++)
        gen_code(t->kids[i]);

    // =========================
    // LEAF NODES
    // =========================
    if (t->leaf) {

        if (t->leaf->category == INTEGERLITERAL) {
            t->place.region = R_CONST;
            t->place.u.offset = t->leaf->ival;
        }

        else if (t->leaf->category == IDENTIFIER) {
            SymbolTableEntry e =
                lookupEntry(globalTable, t->leaf->text);

            if (!e) {
                fprintf(stderr, "Undefined variable %s\n", t->leaf->text);
                exit(3);
            }

            t->place.region = R_LOCAL;
            t->place.u.offset = e->offset;
        }

        t->code = NULL;
        return;
    }

    t->code = NULL;

    // =========================
    // VAR DECL (RULE 5)
    // =========================
    if (t->prodrule == 5) {

        char *name = t->kids[1]->leaf->text;
        SymbolTableEntry e = lookupEntry(globalTable, name);

        struct addr lhs;
        lhs.region = R_LOCAL;
        lhs.u.offset = e->offset;

        struct addr rhs;

        if (t->nkids == 4 && t->kids[3]) {
            rhs = t->kids[3]->place;
        } else {
            rhs.region = R_CONST;
            rhs.u.offset = 0;
        }

        struct instr *i = gen(
            O_ASN,
            lhs,
            rhs,
            (struct addr){R_NONE}
        );

        t->code = append(
            (t->nkids == 4 ? t->kids[3]->code : NULL),
            i
        );
    }

    // =========================
    // ASSIGNMENT (RULE 6)
    // =========================
    else if (t->prodrule == 6) {

        SymbolTableEntry e =
            lookupEntry(globalTable, t->kids[0]->leaf->text);

        struct addr lhs;
        lhs.region = R_LOCAL;
        lhs.u.offset = e->offset;

        struct instr *i = gen(
            O_ASN,
            lhs,
            t->kids[2]->place,
            (struct addr){R_NONE}
        );

        t->code = append(
            t->kids[0]->code,
            append(t->kids[2]->code, i)
        );
    }

    // =========================
    // ARITHMETIC
    // =========================
    else if (t->prodrule == 12 ||
             t->prodrule == 13 ||
             t->prodrule == 14 ||
             t->prodrule == 15) {

        int op =
            (t->prodrule == 12) ? O_ADD :
            (t->prodrule == 13) ? O_SUB :
            (t->prodrule == 14) ? O_MUL :
                                  O_DIV;

        t->place = newtemp();

        struct instr *i = gen(
            op,
            t->place,
            t->kids[0]->place,
            t->kids[2]->place
        );

        t->code = append(
            append(t->kids[0]->code, t->kids[2]->code),
            i
        );
    }

    // =========================
    // RETURN
    // =========================
    else if (t->prodrule == 4) {

        struct instr *i = gen(
            O_RET,
            t->kids[1]->place,
            (struct addr){R_NONE},
            (struct addr){R_NONE}
        );

        t->code = append(t->kids[1]->code, i);
    }

    // =========================
    // RELATIONAL
    // =========================
    else if (t->prodrule >= 16 && t->prodrule <= 19) {

        int op =
            (t->prodrule == 16) ? O_BLT :
            (t->prodrule == 17) ? O_BGT :
            (t->prodrule == 18) ? O_BEQ :
                                  O_BNE;

        t->place = newtemp();

        struct instr *i = gen(
            op,
            t->place,
            t->kids[0]->place,
            t->kids[2]->place
        );

        t->code = append(
            append(t->kids[0]->code, t->kids[2]->code),
            i
        );
    }

    // =========================
    // FUNCTION CALL
    // =========================
    else if (t->prodrule == 11) {

        struct instr *code = NULL;

        struct tree *arg = (t->nkids > 2) ? t->kids[2] : NULL;

        struct tree *args[100];
        int n = 0;

        while (arg) {
            if (arg->prodrule == 20) {
                args[n++] = arg->kids[2];
                arg = arg->kids[0];
            } else {
                args[n++] = arg;
                break;
            }
        }

        for (int i = n - 1; i >= 0; i--) {

            if (args[i]->code)
                code = append(code, args[i]->code);

            struct instr *p = gen(
                O_PARM,
                args[i]->place,
                (struct addr){R_NONE},
                (struct addr){R_NONE}
            );

            code = append(code, p);
        }

        t->place = newtemp();

        struct addr fname;
        fname.region = R_NAME;
        fname.u.name = t->kids[0]->leaf->text;

        struct instr *call = gen(
            O_CALL,
            fname,
            (struct addr){R_CONST, .u.offset = n},
            t->place
        );

        t->code = append(code, call);
    }

    // =========================
    // DEFAULT GLUE
    // =========================
    else {
        struct instr *c = NULL;

        for (int i = 0; i < t->nkids; i++) {
            if (t->kids[i] && t->kids[i]->code)
                c = append(c, t->kids[i]->code);
        }

        t->code = c;
    }
}