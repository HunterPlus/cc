#include "c.h"

static int alloc();
static int delloc();
static char *reg[8];

static int leaq(Symbol);
static int indir(Type, int);
static int loadcnst(Tree);

static int add(Type, int, int);

static char *movx(Type);

int emit(Tree p) {
    int r0, r1;
    r0 = r1 = -1;
    
    switch (p->op) {
    case ADDR:  return leaq(p->u.sym);
    case ICON:  return loadcnst(p);
    default:
        break;
    }

    if (p->kids[0])
        r0 = emit(p->kids[0]);
    if (p->kids[1])
        r1 = emit(p->kids[1]);
        
    switch (p->op) {
    case ADD:   return add(p->type, r0, r1);
    case INDIR: return indir(p->type, r0);
    default:
        return -1;
    }        
}

static int leaq(Symbol p) {
    int r;
    
    r = alloc();
    fprintf(out, "\tleaq\t%s, %s\n", p->x.name, reg[r]);
    return r;
}
static int indir(Type ty, int r0) {
    int r;
    char *mov;

    mov = movx(ty);
    r = alloc();
    fprintf(out, "\t%s\t(%s), %s\n", mov, reg[r0], reg[r]);
    delloc(r0);
    return r;
}
static int loadcnst(Tree p) {
    int r;
    Type ty;
    
    r = alloc();
    ty = p->type;
    if (ty->op == INT)
        fprintf(out, "\tmovq\t$%ld, %s\n", p->u.v.i, reg[r]);  
    else if (ty->op == UNSIGNED)
        fprintf(out, "\tmovq\t$%lu, %s\n", p->u.v.u, reg[r]);  
    else
        fprintf(stderr, "loadcnst(): unsurport type.\n");
    return r;
}
static int add(Type ty, int r0, int r1) {
    if (isptr(ty)) 
        fprintf(out, "\tleaq\t(, %s, %s, %d), %s\n", 
            reg[r1], reg[r0], ty->type->size, reg[r1]);
    else if (isint(ty))
        fprintf(out, "\taddq\t%s, %s\n", reg[r0], reg[r1]);
    else
        fprintf(stderr, "add(): unsurport type.\n");
    delloc(r0);
    return r1;
}
static char *movx(Type ty) {
    char *mov;
    
    if (isptr(ty))
        mov = "movq";
    else if (ty->op == INT) 
        switch (ty->size) {
        case 1: mov = "movsbq";    break;
        case 2: mov = "movswq";    break;
        case 4: mov = "movslq";    break;
        case 8: mov = "movq";      break;
        default:
            assert(0); break;
        }
    else if (ty->op == UNSIGNED)
        switch (ty->size) {
        case 1: mov = "movzbq";    break;
        case 2: mov = "movzwq";    break;
        case 4: mov = "movzlq";    break;
        case 8: mov = "movq";      break;
        default:
            assert(0); break;
        }
    else
        fprintf(stderr, "movx(): unsurport type.\n");
    return mov;
}
static int alloc() {
    
}
static int delloc() {
    
}
