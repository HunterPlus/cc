#include "c.h"

void inputinit();
void typeinit(void);

Type decl(Symbol *p);
void *tname(Type ty);
int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("arguments error\n");
        exit(1);
    }
    file = argv[1];
    inputinit();
    typeinit();
    
    Symbol p;
    Type ty;
    t = gettok();
    ty = decl(&p);
    
    printf("%s: ", p->name);
    tname(ty);
/*
    while ((t = gettok()) != EOI) {
        ty = speci(&cls);
        printf ("%s\n", ty->sym->name);

    }
    
    ty = ptr(unsignedlong);
    ty = array(unsignedtype, 5, 0);

 
    printsymbol();
*/    
    
    return 0;
}
