#include "c.h"

static Tree addtree(int, Tree, Tree);
static Tree andtree(int, Tree, Tree);
static Tree cmptree(int, Tree, Tree);
static int compatible(Type, Type);
static int isnullptr(Tree);
static Tree multree(int, Tree, Tree);
static Tree subtree(int, Tree, Tree);
#define isvoidptr(ty) (isptr(ty) && ty->type == voidtype)

Tree (*optree[])(int, Tree, Tree) = {
#define xx(a,b,c,d,e,f,g)   e,
#define yy(a,b,c,d,e,f,g)   e,
#include "token.h"
};

int where = STMT;

Tree tree(int op, Type type, Tree left, Tree right) {
    Tree p;
    
    NEW0(p, where);
    p->op = op;
    p->type = type;
    p->kids[0] = left;
    p->kids[1] = right;
    return p;
}
Tree texpr(Tree (*f)(int), int tok, int a) {
    int save = where;
    Tree p;
    
    where = a;
    p = (*f)(tok);
    where = save;
    return p;
}
static Tree addtree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if (isarith(l->type) && isarith(r->type))
        ty = binary(l->type, r->type);
    else if (isptr(r->type) && isint(l->type))
        return addtree(op, r, l);
    else if (isptr(l->type) && isint(r->type) && !isfunc(l->type->type)) {
        ty = l->type;
        if (ty->type->size == 0)
            fprintf(stderr, "addtree(): unknown size of pointer to.\n");
        return tree(POINTER, ty, l, r);
    } else
        fprintf(stderr, "addtree(): unsurport add operad.\n");
    return tree(op, ty, l, r);
}
static Tree andtree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if (!isscalar(l->type) || !isscalar(r->type))
        fprintf(stderr, "andtree(): scalar type needed.\n");
    return tree(op, ty, l, r);
}
static Tree cmptree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if (isarith(l->type) && isarith(r->type) 
    || compatible(l->type, r->type))
        return tree(op, ty, l, r);
    fprintf(stderr, "cmptree(): wrong type for compare.\n");
    return tree(op, ty, l, r);
}
static Tree multree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if (isarith(l->type) && isarith(r->type)) 
        ty = binary(l->type, r->type);
    else
        fprintf(stderr, "multree(): unsurport type.\n");
    return tree(op, ty, l, r);
}
static Tree subtree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if(isarith(l->type) && isarith(r->type))
        ty = binary(l->type, r->type);
    else if (isptr(l->type) && !isfunc(l->type) && isint(r->type)) {
        ty = l->type;
        if (ty->type->size == 0)
            fprintf(stderr, "subtree(): unknown size of pointer to.\n");
        return tree(POINTER, ty, l, tree(NEG, r->type, r, NULL));
    } else if (compatible(l->type, r->type))
        ty = longtype;
    else
        fprintf(stderr, "subtree(): unsurport sub type.\n");
    return tree(op, ty, l, r);
}
Tree bittree(int op, Tree l, Tree r) {
    Type ty = inttype;
    
    if (isint(l->type) && isint(r->type))
        ty = binary(l->type, r->type);
    else
        fprintf(stderr, "bittree(): need int or unsigned type.\n");
    return tree(op, ty, l, r);
}
Tree eqtree(int op, Tree l, Tree r) {
    return cmptree(op, l, r);
}
Tree shtree(int op, Tree l, Tree r) {
    if (!isint(l->type) || !isint(r->type))
        fprintf(stderr, "shtree(): must be int or unsigned type.\n");
    return tree(op, l->type, l, r);
}
Tree asgntree(int op, Tree l, Tree r) {
    Type ty;
    
    /* check incompatible type of l and r */
    ty = l->type;
    l = lvalue(l);
    return tree(op, ty, l, r);
}
Tree cnsttree(Type ty, ...) {
    Tree p; 
    va_list ap;
    
    if (ty->op == INT || ty->op == UNSIGNED)
        p = tree(ICON, ty, NULL, NULL);
    else if (ty->op == FLOAT)
        p = tree(FCON, ty, NULL, NULL);
    va_start(ap, ty);
    switch (ty->op) {
    case INT:       p->u.v.i = va_arg(ap, long);            break;
    case UNSIGNED:  p->u.v.u = va_arg(ap, unsigned long);   break;
    case FLOAT:     p->u.v.d = va_arg(ap, double);          break;
    case POINTER:   p->u.v.p = va_arg(ap, void *);          break;
    default:
        assert(0);
    }
    va_end(ap);
    return p;
}
static int compatible(Type ty1, Type ty2) {
    return isptr(ty1) && !isfunc(ty1->type)
        && isptr(ty2) && !isfunc(ty2->type)
        && eqtype(ty1->type, ty2->type, 0);
}
static int isnullptr(Tree e) {
    Type ty = e->type;
    
    return e->op == CNST
        && (ty->op == INT && e->u.v.i == 0
        || ty->op == UNSIGNED && e->u.v.u == 0
        || isvoidptr(ty) && e->u.v.p == NULL);
}

