#include "c.h"

char kind[] = {
#define xx(a,b,c,d,e,f,g) f,
#define yy(a,b,c,d,e,f,g) f,
#include "token.h"
};

int errcnt;

/*
void error0(char *s, int n) {
    errcnt++;
    fprintf(stderr, "%s:%d:%d: Token<%s> %s\n", 
        file, src.y, src.x, stringn(token, n), s);
}
*/
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