#include "c.h"

struct table {
    int level;
    Table previous;
    struct entry {
        struct symbol sym;
        struct entry *link;
    } *buckets[256];
    Symbol all;
};
#define HASHSIZE NELEMS(((Table)0)->buckets)
static struct table 
    cns = { CONSTANTS },
    ext = { GLOBAL },
    ids = { GLOBAL },
    tys = { GLOBAL };
Table constans = &cns;
Table externals = &ext;
Table identifiers = &ids;
Table types = &tys;
Table labels;
int level = GLOBAL;
static int tempid;
List loci, symbols;

Table newtable(int arena) {
    Table new;
    NEW0(new, arena);
    return new;
}

Table table(Table tp, int level) {
    Table new = newtable(FUNC);
    new->previous = tp;
    new->level = level;
    if (tp)
        new->all = tp->all;
    return new;
}
void enterscope(void) {
    if (++level == LOCAL)
        tempid = 0;
}
void exitscope(void) {
    rmtypes(level);
    if (types->level == level)
        types = types->previous;
    if (identifiers->level == level)
        identifiers = identifiers->previous;
    assert(level >= GLOBAL);
    --level;
}
Symbol install(char *name, Table *tpp, int level, int arena) {
    Table tp = *tpp;
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);
    
    assert(level == 0 || level >= tp->level);
    if (level > 0 && level > tp->level)
        tp = *tpp = table(tp, level);
    NEW0(p, arena);
    p->sym.name = name;
    p->sym.scope = level;
    p->sym.up = tp->all;
    tp->all = &p->sym;
    p->link = tp->buckets[h];
    tp->buckets[h] = p;
    return &p->sym;
}
Symbol lookup(char *name, Table tp) {
    struct entry *p;
    unsigned h = (unsigned long)name & (HASHSIZE - 1);
    
    assert(tp);
    do  
        for (p = tp->buckets[h]; p; p = p->link)
            if (p->sym.name == name)
                return &p->sym;
    while ((tp = tp->previous) != NULL);
    return NULL;
}
Symbol findtype(Type ty) {
    Table tp = identifiers;
    int i;
    struct entry *p;
    
    do
        for (i = 0; i < HASHSIZE; i++)
            for (p = tp->buckets[i]; p; p = p->link)
                if (p->sym.type == ty && p->sym.sclass == TYPEDEF)
                    return &p->sym;
    while ((tp = tp->previous) != NULL);
    return NULL;
}

int genlabel(int n) {
    static int label = 1;
    
    label += n;
    return label - n;
}












