#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

enum { ADDR = 128, CAST };
FILE *out;

#define NEW(p,a) ((p) = allocate(sizeof *(p), a))
#define NEW0(p,a) memset(NEW(p,a), 0, sizeof *(p))
#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))
#define ones(n) ((n) >= 8 * sizeof (unsigned long) ? ~0UL : ~((~0UL) << (n)))

#define ischar(t)   ((t)->size == 1 && isint(t))
#define isint(t)    ((t)->op == INT || (t)->op == UNSIGNED)
#define isarith(t)  ((t)->op <= UNSIGNED)
#define isscalar(t) ((t)->op <= POINTER || (t)->op == ENUM)
#define isptr(t)    ((t)->op == POINTER)
#define isarray(t)  ((t)->op == ARRAY)
#define isstruct(t) ((t)->op == STRUCT)
#define isunion(t)  ((t)->op == UNION)
#define isfunc(t)   ((t)->op == FUNCTION)

#define istypename(t,tsym) (kind[t] == CHAR \
        || t == ID && tsym && tsym->sclass == TYPEDEF)

enum { PERM=0, FUNC, STMT };
enum {
#define xx(a,b,c,d,e,f,g) a=b,
#define yy(a,b,c,d,e,f,g)
#include "token.h"
	LAST
};
#define gop(name,value) name=value<<4,
#define op(name,type,sizes)
enum {
#include "ops.h"
	LASTOP
};
#undef gop
#undef op
enum {
	AND=38<<4,
	NOT=39<<4,
	OR=40<<4,
	COND=41<<4,
	RIGHT=42<<4,
	FIELD=43<<4
};
enum { CODE=1, BSS, DATA, LIT };

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
typedef struct symbol *Symbol;
typedef struct table *Table;
typedef struct node *Node;
typedef struct tree *Tree;

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

enum {GLOBAL, PARAM, LOCAL};
struct symbol {
    char *name;
    int scope;
    Coordinate src;
    Symbol tlink;       /* type link list */
    int sclass;
    unsigned structarg : 1;
    unsigned addressed : 1;
    unsigned computed : 1;
    unsigned temporary : 1;
    unsigned generated : 1;
    unsigned defined : 1;
    Type type;
    Symbol tsym;
    int flag;           /* type=0, ID, ICON, FCON, SCON */
    float ref;
    union {
        struct { int label; Symbol equatedto; } l;
        Field flist;
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
    struct {
        char *name;
        int offset;
    } x;
};

struct tree {
    int op;
    Type type;
    Tree kids[2];
    Node node;
    union {
        Value v;
        Symbol sym;
        Field field;
    } u;
};
struct node {
    short op;
    short count;
    Symbol syms[3];
    Node kids[2];
    Node link;
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
extern int getchr(void);

/* ----------------alloc.c ------------------ */
extern void *allocate(unsigned long, unsigned a);
extern void dellocate(unsigned);
extern void *newarray(unsigned long, unsigned long, unsigned);


extern char *string(char *);
extern char *stringn(char *, int);
extern char *stringd(long);

extern List append(void *, List);
extern int  length(List);
extern void *ltov (List *, unsigned);

/* ----------------type.c ------------------ */
extern Type voidtype;
extern Type chartype;           /* char */
extern Type shorttype;			/* signed short int */
extern Type inttype;			/* signed int */
extern Type longtype;			/* long */
extern Type floattype;			/* float */
extern Type doubletype; 	    /* double */
extern Type unsignedchar;		/* unsigned char */
extern Type unsignedshort;		/* unsigned short int */
extern Type unsignedtype;		/* unsigned int */
extern Type unsignedlong;   	/* unsigned long int */
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
extern void rmtypes(Symbol);
extern Type compose(Type, Type);
extern Type freturn(Type);
extern int variadic(Type);
extern Type newstruct(int, char *);
extern Field newfield(char *, Type, Type);
extern Field fieldref(char *, Type);

/* ----------------error.c ------------------ */
extern char kind[];
extern int errcnt;

extern void fatal(char *);
extern void error(char *);
extern void expect(int);
extern void skipto(int, char *);
extern void test(int, char *);

/* ----------------symbol.c ------------------ */
extern int level;
extern Table gtp;
extern Table tp;
/*
extern Table constans;
extern Table externals;
extern Table identifiers;
extern Table types;
extern Table labels;
*/
extern void enterscope(void);
extern void exitscope(void);
extern Symbol install(char *, int, int);
extern Symbol lookup(char *, int);
extern Symbol findtype(Type);
extern Symbol sconst(Type, Value);
extern int genlabel(int);

/* ------------------- decl.c ----------------------- */
extern Type typename(void);
extern Type initializer(Type, int);
extern void program(void);
extern void defglobal(Symbol, int);

/* ------------------- tree.c ---------------------*/
extern Tree (*optree[])(int, Tree, Tree);
extern Tree tree(int, Type, Tree, Tree);
extern Tree cnsttree(Type, ...);
extern Tree bittree(int, Tree, Tree);
extern Tree eqtree(int, Tree, Tree);
extern Tree shtree(int, Tree, Tree);
extern Tree asgntree(int, Tree, Tree);


/* ---------------------- expr.c ------------------*/
extern Tree expr(int);
extern Tree expr1(int);
extern Tree incr(int, Tree, Tree);
extern Tree idtree(Symbol);
extern Tree rvalue(Tree);
extern Tree lvalue(Tree);
extern Type binary(Type, Type);
extern Tree retype(Tree, Type);
extern Tree field(Tree, char *);

/* ---------------- emit.c ------------------------ */
extern int emit(Tree);

/* ------------------ init.c ------------------------*/
extern void swtoseg(int);
extern void global(Symbol);
extern void export(Symbol);
extern void defsymbol(Symbol);
extern void defstring(int, char *);
