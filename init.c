#include "c.h"

static int cseg;

static void space (int);
static void defpointer(Tree);
static void defconst (int, int, Value);
static int initarray (int, Type, int);
static int initstruct(int, Type, int);
static void initend (int);
static void segment (int);


Type initializer(Type ty, int lev) {
    int n = 0;
    Tree e;
    static char follow[] = { IF, CHAR, STATIC, 0 };

    if (isscalar(ty)) {
        if (t == '{') {
	        t = gettok();
	        e = expr1(0);
	        initend(lev);
	    } else
	        e = expr1 (0);
	    if (isptr(ty))
	        defpointer(e);
	    else 
            defconst(ty->op, ty->size, e->u.v);
	    n = ty->size;
        dellocate(STMT);
    }
    if ((isstruct(ty) || isunion(ty)) && ty->size == 0) {
        static char follow[] = { CHAR, STATIC, 0 };
        fprintf(stderr, "initializer(): struct/union must have size.\n");
        skipto(';', follow);
        return ty;
    } else if (isstruct(ty)) {
        if (t == '{') {
            t = gettok();
            n = initstruct(0, ty, lev + 1);
            expect('}');
        } else if (lev > 0)
            n = initstruct(ty->size, ty, lev + 1);
        else {
            fprintf(stderr, "initializer(): init struct missing '{'.\n");
            n = initstruct(ty->u.sym->u.flist->type->size, ty, lev + 1);
        }
    } else if (isunion(ty)) {
        if (t == '{') {
            t = gettok();
            n = initstruct(ty->u.sym->u.flist->type->size, ty, lev + 1);
            expect('}');
        } else {
            if (lev == 0)
                fprintf(stderr, "initializer(): init union missing '{'.\n");
            n = initstruct(ty->u.sym->u.flist->type->size, ty, lev + 1);
        }
    }

    if (isarray (ty) && ischar(ty->type) && t == SCON) {
        n = (ty->size == tsym->type->size - 1) ? ty->size : tsym->type->size;
        defstring(n, tsym->u.c.v.p);
        t = gettok();
    } else if (isarray(ty)) {
        if (t == '{') {
	        t = gettok();
	        n = initarray(0, ty->type, lev + 1);
	        expect ('}');
	    } else if (lev > 0 && ty->size > 0)
	        n = initarray(ty->size, ty->type, lev + 1);
        else {
	        fprintf(stderr, "initializer(): missing '{'.\n");
	        n = initarray(ty->type->size, ty->type, lev + 1);
	    }
    }


    if (ty->size) {
        if (n > ty->size)
	        fprintf(stderr, "initializer(): to many initializers.\n");
        else if (n < ty->size)
	        space(ty->size - n);
    } else if (isarray (ty) && ty->type->size > 0)
        ty = array(ty->type, n / ty->type->size, 0);
    else
        ty->size = n;
    return ty;
}

static void initend(int lev) {
    if (lev == 0 && t == ',')
        t = gettok ();
    expect('}');
}

static int initarray(int len, Type ty, int lev) {
    int n = 0;

    do {
        initializer(ty, lev);
        n += ty->size;
        if (len > 0 && n >= len || t != ',')
	        break;
        t = gettok();
    } while (t != '}');
    return n;
}
static int initstruct(int len, Type ty, int lev) {
    int a, n = 0;
    Field p = ty->u.sym->u.flist;
    
    do {
        if (p->offset > n) {
            space(p->offset - n);
            n = p->offset;
        }
        initializer(p->type, lev);
        n += p->type->size;
        if (p->link) {
            p = p->link;
            a = p->type->align;
        } else
            a = ty->align;
        if (a && n%a) {
            space(a - n%a);
            n = roundup(n, a);
        }
        if (len > 0 && n >= len || t != ',')
            break;
    } while ((t = gettok()) != '}');
    return n;
}
static void space(int n) {
    fprintf(out, "\t.zero %d\n", n);
}

void defstring(int n, char *str) {
    char *s;
    
    for (s = str; s < str+n; s++) {
        fprintf(out, "\t.byte %d\n", *s);
    }
}

static void defpointer(Tree e) {
    if (e->op == ADDR)
        fprintf(out, "\t.quad %s\n", e->u.sym->x.name);
    else if (e->op == ICON && e->u.v.i == 0) {
        static Value v;
        defconst(UNSIGNED, unsignedlong->size, v);
    } else
        fprintf(stderr, "defpointer(): invalid tree operand.\n");
}

static void defconst(int op, int size, Value v) {
    if (op == INT || op == UNSIGNED)
        switch (size) {
        case 1:
	        fprintf(out, "\t.byte %ld\n", v.i);
	        break;
        case 2:
	        fprintf(out, "\t.word %ld\n", v.i);
	        break;
        case 4:
	        fprintf(out, "\t.int %ld\n", v.i);
	        break;
        case 8:
	        fprintf(out, "\t.quad %ld\n", v.i);
	        break;
        default:
	        assert(0);
	        break;
        }
    else
        fprintf(stderr, "defconst(): unsurport const type.\n");
}

void export(Symbol p) {
    fprintf(out, "\t.globl %s\n", p->x.name);
}

void global(Symbol p) {
    fprintf(out, "\t.align %d\n", p->type->align > 8 ? 8 : p->type->align);
    if (!p->generated) {
        fprintf(out, "\t.type %s,@%s\n", p->x.name,
	        isfunc (p->type) ? "function" : "object");
        if (p->type->size > 0)
	        fprintf(out, "\t.size %s,%d\n", p->x.name, p->type->size);
    }
    if (p->u.seg == BSS) {
        if (p->sclass == STATIC)
	        fprintf(out, "\t.lcomm %s,%d\n", p->x.name, p->type->size);
        else
	        fprintf(out, "\t.comm %s,%d\n", p->x.name, p->type->size);
    } else
        fprintf(out, "%s:\n", p->x.name);
}

void defsymbol(Symbol p) {
    static char buffer[512];
    int offset;

    offset = 0;
    memset(buffer, 0, 512);
    if (p->scope >= LOCAL && p->sclass == STATIC) {
        offset = sprintf(buffer, "%s", p->name);
        sprintf (buffer + offset, ".%d", genlabel (1));
        p->x.name = string(buffer);
    } else if (p->generated) {
        sprintf(buffer, ".LC%s", p->name);
        p->x.name = string(buffer);
    } else if (p->scope == GLOBAL || p->sclass == EXTERN)
        p->x.name = p->name;
    else
        p->x.name = p->name;
}

static void segment(int n) {
    if (cseg == n)
        return;
    cseg = n;
    if (cseg == CODE)
        fprintf(out, "\t.text\n");
    else if (cseg == BSS)
        fprintf (out, "\t.bss\n");
    else if (cseg == DATA)
        fprintf(out, "\t.data\n");
}

void swtoseg(int seg) {
    if (cseg != seg)
        segment (seg);
    cseg = seg;
}
