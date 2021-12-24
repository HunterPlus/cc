#include "c.h"

struct table {
    int level;
    Table up;
    struct entry {
        struct symbol sym;
        struct entry *link;
    } *buckets[256];
    Symbol tlist;
};
#define HASHSIZE NELEMS(((Table)0)->buckets)

static struct table globaltable;
Table gtp   = &globaltable;
Table tp    = &globaltable;
int level   = GLOBAL;
static int tempid;
List loci, symbols;

Table newtable(int arena) {
    Table new;
    NEW0(new, arena);
    return new;
}

Table table() {
    Table new = newtable(FUNC);
    new->up = tp;
    tp = new;
    new->level = level;
    return new;
}
void enterscope(void) {
    ++level;
    table();
}
void exitscope(void) {
    Symbol p;

    for (p = tp->tlist; p; p = p->tlink)
        rmtypes(p);
    assert(level >= 0);
    tp = tp->up;
    --level;
}
Symbol install(char *name, int flag, int arena) { 
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);    

    NEW0(p, arena);
    p->sym.name = name;
    p->sym.scope = level;
    p->sym.flag = flag;
    p->link = tp->buckets[h];
    tp->buckets[h] = p;
    if (flag == 0) {
        p->sym.tlink = tp->tlist;
        tp->tlist = &p->sym;
    }
    return &p->sym;
}
Symbol lookup(char *name, int flag) {
    Table table = tp;
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);

    do  
        for (p = table->buckets[h]; p; p = p->link)
            if (p->sym.name == name && p->sym.flag == flag)
                return &p->sym;
    while ((table = table->up) != NULL);
    return NULL;
}
Symbol findtype(Type ty) {
    Table table = tp;
    int i;
    struct entry *p;
    
    do
        for (i = 0; i < HASHSIZE; i++)
            for (p = table->buckets[i]; p; p = p->link)
                if (p->sym.type == ty && p->sym.sclass == TYPEDEF)
                    return &p->sym;
    while ((table = table->up) != NULL);
    return NULL;
}
Symbol sconst(Type ty, Value v) {
    char *name = stringd(genlabel(1));
    unsigned h = (unsigned long)name & (HASHSIZE - 1);
    struct entry *p;
    
    NEW0(p, PERM);
    p->sym.name = name;
    p->sym.scope = GLOBAL;
    p->sym.type = ty;
    p->sym.sclass = STATIC;
    p->sym.u.c.v = v;
    p->sym.flag = SCON;
    p->link = gtp->buckets[h];
    gtp->buckets[h] = p;
    p->sym.generated = 1;
    defsymbol(&p->sym);
    p->sym.defined = 1;
    return &p->sym;
}
int genlabel(int n) {
    static int label = 1;
    
    label += n;
    return label - n;
}












