#include "c.h"
#include <float.h>

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
    Symbol p = install(string(name), 0, PERM);
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
        error("deref(): pointer expected.");
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
Type compose(Type ty1, Type ty2) {
    if (ty1 == ty2)
        return ty1;
    assert(ty1->op == ty2->op);
    switch (ty1->op) {
    case POINTER:
        return ptr(compose(ty1->type, ty2->type));
    case ARRAY: {
        Type ty = compose(ty1->type, ty2->type);
        if (ty1->size && (ty1->type->size && ty2->size == 0 || ty1->size == ty2->size))
            return array(ty, ty1->size/ty1->type->size, ty1->align);
        if (ty2->size && ty2->type->size && ty1->size == 0)
            return array(ty, ty2->size/ty2->type->size, ty2->align);
        return array(ty, 0, 0);
    }
    default:
        fprintf(stderr, "compose(): unsurport type error.\n");
        return NULL;
    }
}
void rmtypes(Symbol p) {
    int i; 
    struct entry *tn, **tq;

    for (i = 0; i < NELEMS(typetable); i++) {
        tq = &typetable[i];
        while ((tn = *tq) != NULL)
            if (tn->type.u.sym == p) {
                *tq = tn->link;
                return;
            } else
                tq = &tn->link;
    }       
            
}
Type freturn(Type ty) {
    if (isfunc(ty))
        return ty->type;
    fprintf(stderr, "freturn(): function type expected.\n");
    return inttype;
}
int variadic(Type ty) {
    if (isfunc(ty) && ty->u.proto) {
        int i;
        
        for (i = 0; ty->u.proto[i]; i++)
            ;
        return i > 1 && ty->u.proto[i-1] == voidtype;
    }
    return 0;
}
Type newstruct(int op, char *tag) {
    Symbol p;
    
    assert(tag);
    if (*tag == 0)
        tag = stringd(genlabel(1));
    else if ((p = lookup(tag, 0)) != NULL 
    && (p->scope == level || p->scope == PARAM && level == PARAM + 1)) {
        if (p->type->op == op && !p->defined) 
            return p->type;
        fprintf(stderr, "newstruct(): redefine struct.\n");
    }
    p = install(tag, 0, PERM);
    p->type = type(op, NULL, 0, 0, p);
    p->src = src;
    return p->type;
}
Field newfield(char *name, Type ty, Type fty) {
    Field p, *q = &ty->u.sym->u.flist;
    
    if (name == NULL)
        name = stringd(genlabel(1));
    for (p = *q; p; q = &p->link, p = *q)
        if (p->name == name)
            fprintf(stderr, "newfield(): duplicate field name %s.\n", name);
    NEW0(p, PERM);
    *q = p;
    p->name = name;
    p->type = fty;
    return p;
}
Field fieldref(char *name, Type ty) {
    Field flist;
    
    flist = ty->u.sym->u.flist;
    for (; flist; flist = flist->link)
        if (flist->name == name)
            break;
    return flist;
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
        
        p = install(string("void"), 0, PERM);
        voidtype = type(VOID, NULL, 0, 0, p);
        p->type = voidtype;
    }
    pointersym = install(string("T*"), 0, PERM);
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
    case STRUCT:
        printf("struct %s", ty->u.sym->name);
        break;
    default:
        if (ty->u.sym)
            printf("%s\n", ty->u.sym->name);
        else
            fprintf(stderr, "tname(): unsurport ty.\n");
    }
}






