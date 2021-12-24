#include "c.h"

char kind[] = {
#define xx(a,b,c,d,e,f,g) f,
#define yy(a,b,c,d,e,f,g) f,
#include "token.h"
};

int errcnt;

void error(char *s) {
    errcnt++;
    fprintf(stderr, "%s:%d:%d: Token<%s> %s\n", 
        file, src.y, src.x, string(token), s);
}
void fatal(char *s) {
    fprintf(stderr, "fatal error:\n");
    error(s);
    exit(1);
}
void expect(int tok) {
    if (t == tok)
        t = gettok();
    else
        error("syntax error found.");
}
void skipto(int tok, char set[]) {
    char *s;
    
    while ((t = gettok()) != EOI && t != tok) {
        for (s = set; *s && kind[t] != *s; s++)
            ;
        if (kind[t] = *s)
            break;
    }
}
void test(int tok, char set[]) {
    
}