#include "c.h"
#include <float.h>

//static char buffer[512];

static struct entry {
    struct type type;
    struct entry *link;
} *typetable[128];
static int maxlevel;

static Symbol pointersym;

Type chartype;          /* char */
Type shorttype;			/* signed short int */
Type inttype;			/* signed int */
Type longtype;			/* long */
Type floattype;			/* float */
Type doubletype;		/* double */

Type unsignedchar;		/* unsigned char */
Type unsignedshort;		/* unsigned short int */
Type unsignedtype;		/* unsigned int */
Type unsignedlong;		/* unsigned long int */

Type funcptype;
Type charptype;			
Type voidptype;			
Type voidtype;
Type unsignedptr;
Type signedptr;

static Type type(int , Type, int, int, void *);

Type xxinit(int op, char *name, int size, int align) {
    Symbol p = install(string(name), &types, GLOBAL, PERM);
    Type ty = type(op, 0, size, align, p);
    
    p->type = ty;
    switch(ty->op) {
    case INT:
        p->u.limits.max.i = ones(8 * ty->size) >> 1;
        p->u.limits.min.i = p->u.limits.max.i - 1;
    case UNSIGNED:
        p->u.limits.max.u = ones(8 * ty->size);
        p->u.limits.min.u = 0;
    case FLOAT:
        if (ty->size == sizeof (float))
            p->u.limits.max.d = FLT_MAX;
        else 
            p->u.limits.max.d = DBL_MAX;
        p->u.limits.min.d = -p->u.limits.max.d;
    }
    return ty;
}
static Type type(int op, Type ty, int size, int align, void *sym) {
    unsigned h = (op ^ ((unsigned long)ty >> 3)) & (NELEMS(typetable) - 1);
    struct entry *tn;
    
    if (op != FUNCTION && (op != ARRAY || size > 0))
        for (tn = typetable[h]; tn; tn = tn->link)
            if (tn->type.op == op && tn->type.type == ty
            && tn->type.size == size && tn->type.align == align
            && tn->type.u.sym == sym)
                return &tn->type;
  
    NEW0(tn, PERM);
    tn->type.op = op;
    tn->type.type = ty;
    tn->type.size = size;
    tn->type.align = align;
    tn->type.u.sym = sym;
    tn->link = typetable[h];
    typetable[h] = tn;
    return &tn->type;
}

Type ptr(Type ty) {
    return type(POINTER, ty, 8, 8, pointersym);
}
Type deref(Type ty) {
    if (ty->op != POINTER)
        error("deref error: pointer expected.");
    return ty->type;
}
Type array(Type ty, int n, int a) {
    assert(ty);
    if (isfunc(ty)) {
        fprintf(stderr, "array(): illegal type.\n");
        return array(inttype, n, 0);
    }
    if (isarray(ty) && ty->size == 0)
        fprintf(stderr, "array(): missing array size.\n");
    if (ty->size == 0) {
        if (ty == voidtype)
            fprintf(stderr, "array(): illegal type 'array of voidtype'\n");
        else
            fprintf(stderr, "array(): warning of array type undefined.\n");
    } else if (n > INT_MAX/ty->size) {
        fprintf(stderr, "arrary(): array size exceeds INT_MAX.\n");
        n = 1;
    }
    return type(ARRAY, ty, n*ty->size, a ? a : ty->align, NULL);
}
Type func(Type ty, Type *proto) {
    if (ty && (isarray(ty) || isfunc(ty)))
        fprintf(stderr, "func(): illegal return type.\n");
    ty = type(FUNCTION, ty, 0, 0, NULL);
    ty->u.proto = proto;
    return ty;
}
Type atop(Type ty) {
    if (isarray(ty))
        return ptr(ty->type);
    fprintf(stderr, "atop(): array expected");
    return ptr(ty);
}
int eqtype(Type ty1, Type ty2, int ret) {
    if (ty1 == ty2)
        return 1;
    if (ty1->op != ty2->op)
        return 0;
    switch(ty1->op) {
    case POINTER: return eqtype(ty1->type, ty2->type, ret);
    case ARRAY:
        if (eqtype(ty1->type, ty2->type, ret)) {
            if (ty1->size == ty2->size)
                return 1;
            if (ty1->size == 0 || ty2->size == 0)
                return ret;
        }
        return 0;
    default:
        fprintf(stderr,"eqtype(): unsurport type.");
    }
}
void rmtypes(int lev) {
    if (maxlevel >= lev) {
        int i;
        maxlevel = 0;
        for (i = 0; i < NELEMS(typetable); i++) {
            struct entry *tn, **tq = &typetable[i];
            while ((tn = *tq) != NULL)
                if (tn->type.op == FUNCTION)
                    tq = &tn->link;
                else if (tn->type.u.sym && tn->type.u.sym->scope >= lev)
                    *tq = tn->link;
                else {
                    if (tn->type.u.sym && tn->type.u.sym->scope > maxlevel)
                        maxlevel = tn->type.u.sym->scope;
                    tq = &tn->link;
                }
        }
    }
}
void typeinit() {
#define xx(v,name,op,size,align) v=xxinit(op,name,size,align)
	xx( chartype,        "char",              INT,       1, 1  );
	xx( shorttype,       "short",             INT,       2, 2  );
	xx( inttype,         "int",               INT,       4, 4  );
	xx( longtype,        "long",              INT,       8, 8  );
	xx( floattype,       "float",             FLOAT,     4, 4  );
	xx( doubletype,      "double",            FLOAT,     8, 8  );
	xx( unsignedchar,    "unsigned char",     UNSIGNED,  1, 1  );
	xx( unsignedshort,   "unsigned short",    UNSIGNED,  2, 2  );
	xx( unsignedtype,    "unsigned int",      UNSIGNED,  4, 4  );
	xx( unsignedlong,    "unsigned long",     UNSIGNED,  8, 8  );
#undef xx   
    {
        Symbol p;
        
        p = install(string("void"), &types, GLOBAL, PERM);
        voidtype = type(VOID, NULL, 0, 0, p);
        p->type = voidtype;
    }
    pointersym = install(string("T*"), &types, GLOBAL, PERM);
    pointersym->u.limits.max.p = (void *)ones(8 * 8);
    pointersym->u.limits.min.p = 0;
    voidptype = ptr(voidtype);
    funcptype = ptr(func(voidtype, NULL));
    charptype = ptr(chartype);
    unsignedptr = unsignedlong;
    signedptr = longtype;
}
void *tname(Type ty) {
    assert(ty);

    switch(ty->op) {
    case POINTER:
        printf("pointer to ");
        tname(ty->type);
        break;
    case ARRAY:
        printf("arry[%d] of ", ty->size);
        tname(ty->type);
        break;
    case FUNCTION:
        printf("function returning ");
        tname(ty->type);
        break;
    default:
        if (ty->u.sym)
            printf("%s\n", ty->u.sym->name);
        else
            fprintf(stderr, "tname(): unsurport ty.\n");
    }
}






