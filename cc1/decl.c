#include "c.h"

static Type specifier(int *);
static Type structdcl(int);
static Type enumdcl(void);
static Type dcl(char **, Type);
static Type dirdcl(char **);
static Symbol *parameters(Type);

void progrm(void) {
    int n;
    
    level = 0;
    for (n = 0; t != EOI; n++) 
        if (kind[t] == CHAR || kind[t] == STATIC
        || t == ID || t == '*' || t == '(') {
            ;
        } else if (t == ';') {
            printf("program: empty declaration.\n");
            t = gettok();
        } else {
            error("unrecognized declaration.");
            t = gettok();
        }
    if (n == 0) 
        fprintf(stderr, "program: empty input file.\n");
}
Type decl(Symbol *p) {
    Type ty;
    char *id;
    int cls = 0;
    
    ty = specifier(&cls);
    ty = dcl(&id, ty);
    *p = install(id, &identifiers, level, PERM);
    (*p)->sclass = cls;
    return ty;
}
static Type specifier(int *sclass) {
    int sign = 0;

    if (t == STATIC || t == EXTERN || t == TYPEDEF) {
        if (sclass)
            *sclass = t;
        t = gettok();
    }
    /*
    if (sclass == NULL)
        cls = AUTO;
    */
    for (;;) 
        switch (t) {
        case AUTO:  case REGISTER:  /* unsurport */ 
            t = gettok();                       
            continue;
        case CONST: case VOLATILE:   case SIGNED: /* unsurport */ 
            t = gettok();                       
            continue;
        case UNSIGNED: 
            sign = t;   
            t = gettok();       
            continue;
        case SHORT:
            t = gettok();
            return sign ? unsignedshort : shorttype;
        case LONG:
            t = gettok();
            return sign ? unsignedlong : longtype;
        case VOID:
            t = gettok();
            return voidtype;
        case CHAR:
            t = gettok();
            return sign ? unsignedchar : chartype;
        case INT:
            t = gettok();
            return sign ? unsignedtype : inttype;
        case FLOAT:
            t = gettok();
            return floattype;
        case DOUBLE:    
            t = gettok();
            return doubletype;
        case ENUM:  
            return enumdcl();
        case STRUCT: case UNION: 
            return structdcl(t);
        case ID:
            if (sign)
                return unsignedtype;
            Symbol p = lookup(token, identifiers);
            if (p && p->sclass == TYPEDEF) {
                t = gettok();
                return p->type;
            }
        default:
            fprintf(stderr, "specifier(): type dcl error.\n");
            return inttype;
        }
}
static Type tnode(int op, Type type) {
    Type ty;
    
    NEW0(ty, STMT);
    ty->op = op;
    ty->type = type;
    return ty;
}
static Type dcl(char **id, Type ty) {
    int ns;
    Type node;
    
    for (ns = 0; t == '*'; t = gettok())
        ++ns;
    node = dirdcl(id);
    while (ns-- > 0)
        node = tnode(POINTER, node);
    if (ty) {
        for ( ; node; node = node->type)
            if (node->op == POINTER)
                ty = ptr(ty);
            else if (node->op == ARRAY)
                ty = array(ty, node->size, 0);
            else if (node->op == FUNCTION)
                ty = func(ty, NULL);
            else
                error("dcl(): unsurport dcl");
        return ty;
        
    } else
        return node;
}
static Type dirdcl(char **id) {
    Type node = NULL;
    
    if (t == '(') {
        t = gettok();
        node = dcl(id, NULL);
        if (t != ')')
            error("dirdcl(): syntax error: expect ')'.");
    } else if (t == ID) {
        *id = token;
        
    } else
        error("dirdcl(): expect id name or dcl.");
    while ((t = gettok()) == '(' || t == '[')
        if (t == '(') {
            t = gettok();
            node = tnode(FUNCTION, node);
            parameters(node);
        } else {
            t = gettok();
            int n = 0;
            if (kind[t] == ID)
                n = 10 /*intexpr(']', 1) */;
            else if (t != ']')
                error("dirdcl error: expect '].");
            node = tnode(ARRAY, node);
            node->size = n;
        }
    return node;
}
static Symbol dclparam(char *id, Type ty) {
    Symbol p;
    
    if (isfunc(ty))
        ty = ptr(ty);
    else if (isarray(ty))
        ty = atop(ty);
    if (id == NULL)
        id = stringd(genlabel(1));
    p = lookup(id, identifiers);
    if (p && p->scope == level)
        fprintf(stderr, "dclparam(): duplicate dcl.\n");
    p = install(id, &identifiers, level, FUNC);
    p->sclass = REGISTER;
    p->type = ty;
    p->defined = 1;
    return p;
}
static Symbol parameter() {
    char *id = NULL;
    Symbol p;
    Type ty;
    if (t == ')') {

        return dclparam(NULL, voidtype);
    }
    if (t == ELLIPSIS) {
        
    }
    if (!istypename(t, tsym))
        error("missing parameter type\n");
    ty = dcl(&id, specifier(NULL));
    p = dclparam(id, ty);
    return p;
}
static Symbol *parameters(Type fty) {
    List list = NULL;
    Symbol *params;
    int n;
    
    do 
        list = append(parameter(), list);
    while (t == ',' && (t = gettok()) != ')');
    fty->u.proto = newarray(length(list) + 1, sizeof(Type), PERM);
    params = ltov(&list, FUNC);
    for (n = 0; params[n]; n++)
        fty->u.proto[n] = params[n]->type;
    fty->u.proto[n] = NULL;
    return params;
}

static Type enumdcl(void) {
    
}
static Type structdcl(int op) {
    
}


Type speci(int *sclass) {
    specifier(sclass);
}



