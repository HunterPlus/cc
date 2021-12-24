#include "c.h"

static Type specifier(int *);
static Type structdcl(int);
static Type enumdcl(void);
static void decl(void);
static Type dcl(char **, Type, Symbol **);
static Type dirdcl(char **, Symbol **);
static Symbol *parameters(Type);
static Type structdcl(int);
static void fields(Type);
static void funcdefn(int, char *, Type, Symbol *, Coordinate);
static Symbol dclglobal(int, char *, Type, Coordinate *);

void program(void) {
    int n;
    
    level = 0;
    for (n = 0; t != EOI; n++) 
        if (kind[t] == CHAR || kind[t] == STATIC
        || t == ID || t == '*' || t == '(') {
            decl();
            dellocate(STMT);
        } else if (t == ';') {
            printf("program(): empty declaration.\n");
            t = gettok();
        } else {
            error("program(): unrecognized declaration.");
            t = gettok();
        }
    if (n == 0) 
        fprintf(stderr, "program(): empty input file.\n");
}

static void decl() {
    int sclass = 0;
    Type ty, ty1;
    
    ty = specifier(&sclass);
    while (t == ID || t == '*' || t == '(' || t == '[') {
        Coordinate pos = src;
        char *id = NULL;
        Symbol *params = NULL;
        
        ty1 = dcl(&id, ty, &params);
        if (id == NULL) {
            fprintf(stderr, "decl(): missing identifier.\n");
            id = stringd(genlabel(1));
        }
        if (t == '{' && level == GLOBAL && isfunc(ty1) && params) {
            funcdefn(sclass, id, ty1, params, pos);
            break;
        } else if (isfunc(ty1)) {
            Symbol p = lookup(id, ID);
            if (p != NULL)
                fprintf(stderr, "decl(): redeclaration faunction %s.\n", id);
            else {
                p = install(id, ID, level < LOCAL ? PERM : FUNC);
                p->type = ty1;
                p->sclass = EXTERN;
                p->src = pos;
            }
        } else if (sclass == TYPEDEF) {
            Symbol p = lookup(id, ID);
            if (p && p->scope == level)
                fprintf(stderr, "decl(): id %s redeclaration.\n", id);
            p = install(id, ID, level < LOCAL ? PERM : FUNC);
            p->type = ty1;
            p->sclass = TYPEDEF;
            p->src = pos;
            break;
        } else
            dclglobal(sclass, id, ty1, &pos);
        if (t == ',')
            t = gettok();
        else
            break;
    } //while (t == ',' && ((t = gettok()) == '*' || t == '(' || t == ID));
    expect(';');
}

static Type specifier(int *sclass) {
    int sign = 0;

    if (t == STATIC || t == EXTERN || t == TYPEDEF) {
        if (sclass)
            *sclass = t;
        t = gettok();
    }
    for (;;) 
        switch (t) {
        case AUTO:  case REGISTER:                  /* unsurport */ 
            t = gettok();               continue;
        case CONST: case VOLATILE:   case SIGNED:   /* unsurport */ 
            t = gettok();               continue;
        case UNSIGNED:  sign = t;   t = gettok();   continue;
        case SHORT:     t = gettok();
            return sign ? unsignedshort : shorttype;
        case LONG:      t = gettok();
            return sign ? unsignedlong : longtype;
        case VOID:      t = gettok();   return voidtype;
        case CHAR:      t = gettok();
            return sign ? unsignedchar : chartype;
        case INT:       t = gettok();
            return sign ? unsignedtype : inttype;
        case FLOAT:     t = gettok();   return floattype;
        case DOUBLE:    t = gettok();   return doubletype;
        case ENUM:                      return enumdcl();
        case STRUCT: case UNION:        return structdcl(t);
        case ID:
            if (sign)
                return unsignedtype;
            Symbol p = lookup(token, ID);
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
static Type dcl(char **id, Type ty, Symbol **params) {
    int ns;
    Type node;
    
    for (ns = 0; t == '*'; t = gettok())
        ++ns;
    node = dirdcl(id, params);
    while (ns-- > 0)
        node = tnode(POINTER, node);
    if (ty) {
        for ( ; node; node = node->type)
            if (node->op == POINTER)
                ty = ptr(ty);
            else if (node->op == ARRAY)
                ty = array(ty, node->size, 0);
            else if (node->op == FUNCTION)
                ty = func(ty, node->u.proto);
            else
                error("dcl(): unsurport dcl");
        return ty;
        
    } else
        return node;
}
static Type dirdcl(char **id, Symbol **params) {
    Type node = NULL;
    
    if (t == '(') {
        t = gettok();
        node = dcl(id, NULL, params);
        if (t != ')')
            error("dirdcl(): syntax error: expect ')'.");
    } else if (t == ID) 
        *id = token;
    else {
/*        t = gettok();
        return node;        */
    }
    while ((t = gettok()) == '(' || t == '[')
        if (t == '(') {
            Symbol *q;
            t = gettok();
            node = tnode(FUNCTION, node);
            enterscope();
            q = parameters(node);
            if (params)
                *params = q;
        } else {
            t = gettok();
            int n = 0;
            if (t == ICON) {
                n = (tsym->type->op == INT)? tsym->u.c.v.i : tsym->u.c.v.u;
                t = gettok();
            } 
            if (t != ']')
                error("dirdcl(): expect '].");
            node = tnode(ARRAY, node);
            node->size = n;
        }
    if (node && isfunc(node) && t != '{')
        exitscope();
    return node;
}
static Symbol dclparam(char *id, Type ty, Coordinate *pos) {
    Symbol p;
    
    if (isfunc(ty))
        ty = ptr(ty);
    else if (isarray(ty))
        ty = atop(ty);
    if (id == NULL)
        id = stringd(genlabel(1));
    p = lookup(id, ID);
    if (p && p->scope == level)
        fprintf(stderr, "dclparam(): duplicate dcl.\n");
    p = install(id, ID, FUNC);
    p->sclass = REGISTER;
    p->type = ty;
    p->src = *pos;
    p->defined = 1;
    return p;
}
static Symbol parameter() {
    char *id = NULL;
    Symbol p;
    Type ty;
    Coordinate pos = src;
    if (t == ')') 
        return dclparam(NULL, voidtype, &pos);
    if (t == ELLIPSIS) {
        static struct symbol sentinel;
        if (sentinel.type == NULL) {
            sentinel.type = voidtype;
            sentinel.defined = 1;
        }
        t = gettok();
        return &sentinel;
    }
    if (!istypename(t, tsym))
        error("missing parameter type\n");
    ty = dcl(&id, specifier(NULL), NULL);
    p = dclparam(id, ty, &pos);
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
    /* check parameter error */
    return params;
}

static Type enumdcl(void) {
    
}
static Type structdcl(int op) {
    char *tag;
    Type ty;
    Symbol p;
    Coordinate pos;
    
    t = gettok();
    pos = src;
    if (t == ID) {
        tag = token;
        t = gettok();
    } else
        tag = "";
    if (t == '{') {
        ty = newstruct(op, tag);
        ty->u.sym->src = pos;
        ty->u.sym->defined = 1;
        t = gettok();
        if (istypename(t, tsym))
            fields(ty);
        else
            error("structdcl(): invalid field on decl.\n");
        expect('}');
    } else if (*tag && (p = lookup(tag, 0)) != NULL && p->type->op == op) {
        ty = p->type;
        if (t == ';' && p->scope < level)
            ty = newstruct(op, tag);
    } else {
        if (*tag == 0)
            error("structdcl(): missing tag.\n");
        ty = newstruct(op, tag);
    }
    return ty;
}
static void fields(Type ty) {
    for (; istypename(t, tsym); t = gettok()) {
        Type ty1 = specifier(NULL);
        do {
            Field p;
            char *id = NULL;
            Type fty = dcl(&id, ty1, NULL);
            p = newfield(id, ty, fty);
            if (id == NULL)
                fprintf(stderr, "fields(): mising field name.\n");
            else if (isfunc(p->type))
                fprintf(stderr, "fields(): illegal field type.\n");
            else if (p->type->size == 0)
                fprintf(stderr, "fields(): field type size undefined.\n");
        } while (t == ',' && ((t = gettok()) == '*' || t == ID || t == '('));
        if (t != ';') {
            fprintf(stderr, "fields(): expect ';'.\n");
            while ((t = gettok()) != ';')
                ;
        }
    }
    int off = 0;
    Field p = ty->u.sym->u.flist;
    ty->align = 0;
    
    for (; p; p = p->link) {
        int a = p->type->align;
        if (ty->op == UNION)
            off = 0;
        else
            off = roundup(off, a);
        if (a > ty->align)
            ty->align = a;
        p->offset = off;
        off += p->type->size;
        ty->size = off;
    }
    ty->size = roundup(ty->size, ty->align);
}

static Symbol dclglobal(int sclass, char *id, Type ty, Coordinate *pos) {
    Symbol p;
    
    if (sclass == 0)
        sclass = AUTO;
    else if (sclass != EXTERN && sclass != STATIC) {
        fprintf(stderr, "dclglobal(): invalid storage class %d\n", sclass);
        sclass = AUTO;
    }
    p = lookup(id, ID);
    if (p && p->scope == level) {
        if (p->sclass != TYPEDEF && eqtype(ty, p->type, 1)) {
            ty = compose(ty, p->type);
            if (ty == NULL)
                fprintf(stderr, "dclglobal(): inconsistent type to compose.\n");
        } else
            fprintf(stderr, "dclglobal(): redeclaration symbol %s.\n", id);
        if (p->sclass == EXTERN && sclass == STATIC
        || p->sclass == STATIC && sclass == AUTO
        || p->sclass == AUTO && sclass == STATIC)
            fprintf(stderr, "dclglobal(): inconsisten linkage %s.\n", id);
    } else {
        p = install(id, ID, level < PARAM ? PERM : FUNC);
        p->sclass = sclass;
        defsymbol(p);
    }
    p->sclass = sclass;
    p->flag = ID;
    p->type = ty;
    p->src = *pos;
    if (t == '=') {
        defglobal(p, DATA);
        t = gettok();
        p->type = initializer(p->type, 0); 
    }
    if (isarray(p->type) && p->type->size == 0) 
        fprintf(stderr, "dclglobal(): array size error %s\n", id);
    if (p->sclass == EXTERN)
        p->sclass = AUTO;
}
void defglobal(Symbol p, int seg) {
    p->u.seg = seg;
    swtoseg(p->u.seg);
    if (p->sclass != STATIC)
        export(p);
    global(p);
    p->defined = 1;
}

static void funcdefn(int sclass, char *id, Type ty, Symbol params[], Coordinate pt) {
    
}
Type typename() {
    char *id = NULL;
    Type ty;
    
    ty = dcl(&id, specifier(NULL), NULL);
    return ty;
}
void dclx() {
    decl();
}


