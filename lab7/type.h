#ifndef TYPE_H
#define TYPE_H

#include "tree.h"
typedef struct param {
   char *name;
   struct typeinfo *type;
   struct param *next;
   } *paramlist;

struct field {			/* members (fields) of structs */
   char *name;
   struct type *elemtype;
};

/* base types. How many more base types do we need? */
#define FIRST_TYPE   1000000

#define NONE_TYPE    1000000
#define INT_TYPE     1000001
#define DOUBLE_TYPE   1000002
#define BOOL_TYPE    1000003
#define CHAR_TYPE    1000004
#define NULL_TYPE    1000005
#define STRING_TYPE  1000006
#define ARRAY_TYPE   1000007
#define FUNC_TYPE    1000008
#define CLASS_TYPE   1000009 /* in k0 these are only for a couple built-ins */
#define PACKAGE_TYPE 1000010 /* do we need this for built-ins in k0? maybe */
/* if we were doing a Lisp-like language, such as Python
#define LIST_TYPE    1000011
#define DICT_TYPE    1000012
*/
#define ANY_TYPE     1000011

#define LAST_TYPE    1000011

typedef struct typeinfo {
   int basetype;
   union {
      struct funcinfo {
	 char *name; /* ? */
	 int defined; /* 0 == prototype, 1 == not prototype */
	 struct sym_table *st;
	 struct typeinfo *returntype;
	 int nparams;
	 struct param *parameters;
	 }f;
      struct arrayinfo {
         int size; /* -1 == unspecified/unknown/dontcare */
	 struct typeinfo *elemtype;
         }a;
      } u;
   } *typeptr;

/* add constructors for each of the types if needed */
typeptr alctype(int);
/* probably move these to tree.h; there is a circular include issue
typeptr alcfunctype(struct tree * r, struct tree * p, struct sym_table * st);
typeptr alcarraytype(struct tree * s, struct tree * e);
 */
char *typename(typeptr t);

extern struct sym_table *global_table;
extern typeptr integer_typeptr;
extern typeptr double_typeptr;
extern typeptr char_typeptr;
extern typeptr null_typeptr;
extern typeptr string_typeptr;
extern typeptr boolean_typeptr;

extern char *typenam[];

#endif
