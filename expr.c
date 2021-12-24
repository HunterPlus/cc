#include "c.h"


static char prec[] = {
#define xx(a,b,c,d,e,f,g) c,
#define yy(a,b,c,d,e,f,g) c,
#include "token.h"
};

static int oper[] = {
#define xx(a,b,c,d,e,f,g) d,
#define yy(a,b,c,d,e,f,g) d,
#include "token.h"
};                      

static Tree expr2(void);
static Tree expr3(int);
static Tree nullcheck(Tree);
static Tree postfix(Tree);
static Tree unaray(void);
static Tree primary(void);
static Type super(Type);

static Tree primary() {
    Tree p;
    
    switch (t) {
    case ICON:
        p = tree(ICON, tsym->type, NULL, NULL);
        p->u.v = tsym->u.c.v;     
        break;
    case FCON:        
    case SCON:
        tsym->u.c.v.p = stringn(tsym->u.c.v.p, tsym->type->size);
        tsym = sconst(tsym->type, tsym->u.c.v);
//        global(tsym);
//        defstring(token);
        p = idtree(tsym);
        break;
    case ID:
        if (tsym == NULL)
            error("primary(): id is not defined in symbol table.\n");
        p = idtree(tsym);
        break;
    default:
        error("primary(): unsurport type.");
    }
    t = gettok();
    return p;
}
Tree idtree(Symbol p) {
    int op = ADDR;
    Tree e;
    Type ty = p->type;
    
    if (isarray(ty))
        e = tree(op, atop(ty), NULL, NULL);
    else 
        e = tree(op, ptr(ty), NULL, NULL);
    e->u.sym = p;     
    if (!isarray(ty) && !isfunc(ty))
        e = rvalue(e);
    return e;
}
Tree rvalue(Tree p) {
    Type ty = deref(p->type);
    
    return tree(INDIR, ty, p, NULL);
}
Tree lvalue(Tree p){
    Type ty;
    
    if (p->op != INDIR)
        fprintf(stderr, "lvalue(): lvalue required.\n");
    else if (p->type == voidtype)
        fprintf(stderr, "lvalue(): can't be voidtype.\n");
    return p->kids[0];
}
Tree retype(Tree p, Type ty) {
    Tree q;
    
    if (p->type == ty)
        return p;
    q = tree(p->op, ty, p->kids[0], p->kids[1]);
    q->u = p->u;
    return q;
}
Tree pointer(Tree p) {
    if (isarray(p->type))
        p = retype(p, atop(p->type));
    else if (isfunc(p->type))
        p = retype(p, ptr(p->type));
    return p;
}
static Tree unary(void) {
    Tree p;
    
    switch (t) {
    case '*': 
        t = gettok(); 
        p = unary(); 
        p = rvalue(p);
        return p;
    case '&':
        t = gettok();
        p = unary();
        p = lvalue(p);
        return p;
    case '+':
        t = gettok();
        p = unary();
        return p;
    case '-':
        t = gettok();
        p = unary();
        if (p->op == ICON)
            p->u.v.i *= -1;
        else
            p = tree(NEG, p->type, p, NULL);
        return p;
    case '~':
        t = gettok();
        p = unary();
        p = tree(BCOM, p->type, p, NULL);
        return p;
    case '!':
        t = gettok();
        p = unary();
        p = tree(NOT, p->type, p, NULL);
        return p;
    case INCR:
    case DECR:
        t = gettok();
        p = unary();
        p = tree(t, p->type, NULL, lvalue(p));
        return p;
    case SIZEOF: {
        Type ty;
        p = NULL;
        t = gettok();
        if (t == '(') {
            t = gettok();
            if (istypename(t, tsym)) {
                ty = typename();
                expect(')');
            } else {
                p = postfix(expr(')'));
                ty = p->type;
            }
        } else {
            p = unary();
            ty = p->type;
        }
        p = tree(CNST, unsignedlong, NULL, NULL);
        p->u.v.u = ty->size;
        return p;
    }
    case '(': {
        t = gettok();
        if (istypename(t, tsym)) {
            Type ty, ty1 = typename(), pty;
            expect(')');
            p = unary();
/*            p = tree(CAST, ty1, p, NULL);     */
            p = retype(p, ty1);
        } else
            p = postfix(expr(')'));
        return p;
    }
    default:
        p = postfix(primary());
    }
    return p;
}
static Tree postfix(Tree p) {
    for (;;) 
        switch (t) {
        case INCR:
        case DECR:
            p = tree(t, p->type, lvalue(p), NULL);
            t = gettok();
            break;
        case '[': 
            {
                Tree q;
                t = gettok();
                q = expr(']');
                p = tree(POINTER, p->type, p, q); //(*optree['+'])(ADD, p, q);
                p = rvalue(p);
            }
            break;
        case '(':
            {
                ;
            }
            break;
        case '.':
            t = gettok();
            if (t == ID) 
                if (isstruct(p->type)) 
                    p = field(lvalue(p), token);
                else
                    fprintf(stderr, "postfix(): left of . incompatible.\n");
            else
                fprintf(stderr, "postfix(): field name expected.\n");
            t = gettok();
            break;
        case DEREF:
            t = gettok();
            if (t == ID) 
                if (isptr(p->type) && isstruct(p->type->type))
                    p = field(p, token);
                else
                    fprintf(stderr, "postfix(): left of -> incompatible.\n");
            else
                fprintf(stderr, "postfix(): field name expected.\n");
            t = gettok();
            break;
        default:
            return p;
        }
}
static Tree expr3(int k) {
    int k1;
    Tree p = unary();
    
    for (k1 = prec[t]; k1 >= k; k1--)
        while (prec[t] == k1 && *cp != '=') {
            Tree r;
            int op = t;
            t = gettok();
            if (op == ANDAND || op == OROR)
                r = expr3(k1);
            else
                r = expr3(k1 + 1);
            p =  (*optree[op])(oper[op], p, r);
        }
    return p;
}
Tree expr(int tok) {
    Tree p = expr1(0);
    
    while (t == ',') {
        Tree q;
        t = gettok();
        q = expr1(0);
        p = tree(RIGHT, q->type, p, q);
    }
    if (tok)
        expect(tok);
    return p;
}
Tree expr1(int tok) {
    Tree p = expr2();
    
    if (t == '=' || (prec[t] >= 6 && prec[5] <= 8) 
    || (prec[t] >= 11 && prec[t] <= 13)) {
        int op = t;
        t = gettok();
        if (oper[op] == ASGN)
            p = asgntree(op, p, expr1(0));
        else {
            expect('=');
            p = incr(op, p, expr1(0));
        }
    }
    if (tok)
        expect(tok);
    return p;
}
static Tree expr2(void) {
    Tree p = expr3(4);
    
    if (t == '?') {
        Tree l, r;
        t = gettok();
        l = expr(':');
        r = expr2();
        p = tree('?', NULL, p, tree(':', NULL, l, r));
    }
    return p;
}
Type binary(Type xty, Type yty) {
#define xx(t) if (xty == t || yty == t) return t
        xx(doubletype);
        xx(floattype);
        xx(unsignedlong);
        xx(longtype);
        xx(unsignedtype);
        return inttype;
#undef xx
}
Tree incr(int op, Tree v, Tree e) {
    return asgntree(ASGN, v, (*optree[op])(oper[op], v, e));
}
Tree field(Tree p, char *name) {
    Field q;
    Type ty = p->type;
    
    if (isptr(ty))
        ty = deref(ty);
    if ((q = fieldref(name, ty)) == NULL) {
        fprintf(stderr, "field(): unknown field %s.\n", name);
        return rvalue(p);
    }
    if (isarray(q->type)) {
        ty = atop(q->type);
        p = tree(ADD, ty, p, cnsttree(inttype, q->offset));
    } else {
        ty = ptr(q->type);
        p = tree(ADD, ty, p, cnsttree(inttype, q->offset));
        p = rvalue(p);
    }
    return p;
}



