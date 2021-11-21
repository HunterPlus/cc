#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>



#define NEW(p,a) ((p) = allocate(sizeof *(p), a))
#define NEW0(p,a) memset(NEW(p,a), 0, sizeof *(p))
#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))
#undef roundup
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))
#define ones(n) ((n) >= 8 * sizeof (unsigned long) ? ~0UL : ~((~0UL) << (n)))

#define isptr(t)    (t->op == POINTER)
#define isarray(t)  (t->op == ARRAY)
#define isfunc(t)   (t->op == FUNCTION)

#define istypename(t,tsym) (kind[t] == CHAR \
        || t == ID && tsym && tsym->sclass == TYPEDEF)

enum { PERM=0, FUNC, STMT };
enum {
#define xx(a,b,c,d,e,f,g) a=b,
#define yy(a,b,c,d,e,f,g)
#include "token.h"
	LAST
};



typedef struct list {
    void *x;        
    struct list *link;
} *List;
typedef struct coord {
	char *file;     
	unsigned x, y;
} Coordinate;
typedef union value {
    long i;
    unsigned long u;
    double d;
    void *p;
    void (*g)(void);
} Value;
typedef struct field *Field;
typedef struct type *Type;
typedef struct node *Node;
typedef struct symbol *Symbol;
typedef struct table *Table;

struct field {
    char *name;
    Type type;
    int offset;
    short bitsize;
    short lsb;
    Field link;
};
struct type {
    char *name;
    int op;
    Type type;
    int align;
    int size;
    union {
        Symbol sym;
        Type *proto;
    } u;
};
struct node {
    short op;
    short count;
    Symbol syms[3];
    Node kids[2];
    Node link;
};
enum {CONSTANTS, GLOBAL, PARAM, LOCAL};
struct symbol {
    char *name;
    int scope;
    Coordinate src;
    Symbol up;
    int sclass;
    unsigned structarg : 1;
    unsigned addressed : 1;
    unsigned computed : 1;
    unsigned temporary : 1;
    unsigned generated : 1;
    unsigned defined : 1;
    Type type;
    Symbol tsym;
    float ref;
    union {
        struct { int label; Symbol equatedto; } l;
        struct { unsigned cfields : 1, vFields : 1; Field flist; } s;
        int value;
        Symbol *idlist;
        struct { Value min, max; } limits;
        struct { Value v; Symbol loc; } c;
        struct {
            Coordinate pt; int label; int ncalls; Symbol *callee;
        } f;
        int seg;
        Symbol alias;
        struct {
            Node cse; int replace; Symbol next;
        } t;
    } u;
};


/* ----------------input.c ------------------ */
extern char *file;
extern char *line;
extern int lineno;
extern char *cp;
extern Coordinate src;
extern int t;
extern char *token;
extern Symbol tsym;;

extern void input_init(void);
extern void nextline(void);
extern int gettok(void);

/* ----------------alloc.c ------------------ */
extern void *allocate(unsigned long, unsigned a);
extern void dellocate(unsigned);
extern void *newarray(unsigned long, unsigned long, unsigned);

/* ----------------type.c ------------------ */
extern Type voidtype;
extern Type chartype;          /* char */
extern Type shorttype;			/* signed short int */
extern Type inttype;			/* signed int */
extern Type longtype;			/* long */
extern Type floattype;			/* float */
extern Type doubletype; 	/* double */
extern Type unsignedchar;		/* unsigned char */
extern Type unsignedshort;		/* unsigned short int */
extern Type unsignedtype;		/* unsigned int */
extern Type unsignedlong;		/* unsigned long int */

extern Type funcptype;
extern Type charptype;			
extern Type voidptype;			
extern Type voidtype;
extern Type unsignedptr;
extern Type signedptr;

extern Type ptr(Type);
extern Type array(Type, int, int);
extern Type func(Type, Type *);
extern Type deref(Type);
extern Type atop(Type);
extern int eqtype(Type, Type, int);
extern void rmtypes(int);

/* ----------------error.c ------------------ */
extern char kind[];
extern int errcnt;

extern void fatal(char *);
extern void error(char *);
extern void expect(int);

/* ----------------string.c ------------------ */
extern unsigned hash(char *s);
extern char *string(char *);
extern char *stringn(char *, int);
extern char *stringd(long);
extern void *NEWA(unsigned long, unsigned long);
extern List append(void *x, List list);
extern int  length(List list);
extern void *ltov (List *, unsigned);

/* ----------------symbol.c ------------------ */
extern int level;
extern Table constans;
extern Table externals;
extern Table identifiers;
extern Table types;
extern Table labels;

extern void enterscope(void);
extern void exitscope(void);
extern Symbol install(char *, Table *, int, int);
extern Symbol lookup(char *, Table);
extern Symbol findtype(Type);

extern int genlabel(int);





