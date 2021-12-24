#include "c.h"

void inputinit();
void typeinit(void);

void dclx(void);
void *tname(Type ty);
int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("main(): arguments error\n");
        exit(1);
    }
    file = argv[1];
    inputinit();
    typeinit();
    out = fopen("asm.s", "w");
    
    t = gettok();
    program();
    


    
    return 0;
}
