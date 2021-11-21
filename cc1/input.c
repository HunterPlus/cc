#include <sys/stat.h>
#include "c.h"

enum { BLANK=01,  NEWLINE=02, LETTER=04,
       DIGIT=010, HEX=020,    OTHER=040 };

static char map[128] = { /* 000 nul */	0,
				   /* 001 soh */	0,
				   /* 002 stx */	0,
				   /* 003 etx */	0,
				   /* 004 eot */	0,
				   /* 005 enq */	0,
				   /* 006 ack */	0,
				   /* 007 bel */	0,
				   /* 010 bs  */	0,
				   /* 011 ht  */	BLANK,
				   /* 012 nl  */	NEWLINE,
				   /* 013 vt  */	BLANK,
				   /* 014 ff  */	BLANK,
				   /* 015 cr  */	0,
				   /* 016 so  */	0,
				   /* 017 si  */	0,
				   /* 020 dle */	0,
				   /* 021 dc1 */	0,
				   /* 022 dc2 */	0,
				   /* 023 dc3 */	0,
				   /* 024 dc4 */	0,
				   /* 025 nak */	0,
				   /* 026 syn */	0,
				   /* 027 etb */	0,
				   /* 030 can */	0,
				   /* 031 em  */	0,
				   /* 032 sub */	0,
				   /* 033 esc */	0,
				   /* 034 fs  */	0,
				   /* 035 gs  */	0,
				   /* 036 rs  */	0,
				   /* 037 us  */	0,
				   /* 040 sp  */	BLANK,
				   /* 041 !   */	OTHER,
				   /* 042 "   */	OTHER,
				   /* 043 #   */	OTHER,
				   /* 044 $   */	0,
				   /* 045 %   */	OTHER,
				   /* 046 &   */	OTHER,
				   /* 047 '   */	OTHER,
				   /* 050 (   */	OTHER,
				   /* 051 )   */	OTHER,
				   /* 052 *   */	OTHER,
				   /* 053 +   */	OTHER,
				   /* 054 ,   */	OTHER,
				   /* 055 -   */	OTHER,
				   /* 056 .   */	OTHER,
				   /* 057 /   */	OTHER,
				   /* 060 0   */	DIGIT,
				   /* 061 1   */	DIGIT,
				   /* 062 2   */	DIGIT,
				   /* 063 3   */	DIGIT,
				   /* 064 4   */	DIGIT,
				   /* 065 5   */	DIGIT,
				   /* 066 6   */	DIGIT,
				   /* 067 7   */	DIGIT,
				   /* 070 8   */	DIGIT,
				   /* 071 9   */	DIGIT,
				   /* 072 :   */	OTHER,
				   /* 073 ;   */	OTHER,
				   /* 074 <   */	OTHER,
				   /* 075 =   */	OTHER,
				   /* 076 >   */	OTHER,
				   /* 077 ?   */	OTHER,
				   /* 100 @   */	0,
				   /* 101 A   */	LETTER|HEX,
				   /* 102 B   */	LETTER|HEX,
				   /* 103 C   */	LETTER|HEX,
				   /* 104 D   */	LETTER|HEX,
				   /* 105 E   */	LETTER|HEX,
				   /* 106 F   */	LETTER|HEX,
				   /* 107 G   */	LETTER,
				   /* 110 H   */	LETTER,
				   /* 111 I   */	LETTER,
				   /* 112 J   */	LETTER,
				   /* 113 K   */	LETTER,
				   /* 114 L   */	LETTER,
				   /* 115 M   */	LETTER,
				   /* 116 N   */	LETTER,
				   /* 117 O   */	LETTER,
				   /* 120 P   */	LETTER,
				   /* 121 Q   */	LETTER,
				   /* 122 R   */	LETTER,
				   /* 123 S   */	LETTER,
				   /* 124 T   */	LETTER,
				   /* 125 U   */	LETTER,
				   /* 126 V   */	LETTER,
				   /* 127 W   */	LETTER,
				   /* 130 X   */	LETTER,
				   /* 131 Y   */	LETTER,
				   /* 132 Z   */	LETTER,
				   /* 133 [   */	OTHER,
				   /* 134 \   */	OTHER,
				   /* 135 ]   */	OTHER,
				   /* 136 ^   */	OTHER,
				   /* 137 _   */	LETTER,
				   /* 140 `   */	0,
				   /* 141 a   */	LETTER|HEX,
				   /* 142 b   */	LETTER|HEX,
				   /* 143 c   */	LETTER|HEX,
				   /* 144 d   */	LETTER|HEX,
				   /* 145 e   */	LETTER|HEX,
				   /* 146 f   */	LETTER|HEX,
				   /* 147 g   */	LETTER,
				   /* 150 h   */	LETTER,
				   /* 151 i   */	LETTER,
				   /* 152 j   */	LETTER,
				   /* 153 k   */	LETTER,
				   /* 154 l   */	LETTER,
				   /* 155 m   */	LETTER,
				   /* 156 n   */	LETTER,
				   /* 157 o   */	LETTER,
				   /* 160 p   */	LETTER,
				   /* 161 q   */	LETTER,
				   /* 162 r   */	LETTER,
				   /* 163 s   */	LETTER,
				   /* 164 t   */	LETTER,
				   /* 165 u   */	LETTER,
				   /* 166 v   */	LETTER,
				   /* 167 w   */	LETTER,
				   /* 170 x   */	LETTER,
				   /* 171 y   */	LETTER,
				   /* 172 z   */	LETTER,
				   /* 173 {   */	OTHER,
				   /* 174 |   */	OTHER,
				   /* 175 }   */	OTHER,
				   /* 176 ~   */	OTHER, 
                   /* 177 DEL */    0      };

struct stat st;
static int bsize;
static char *bufferp;
char *cp;
static char *end;
static FILE *f;
char *file;
char *line;
int lineno;

Coordinate src;
int t;
char *token;
Symbol tsym;

void inputinit(void) {
    stat(file, &st);
    bufferp = (char *) malloc(st.st_size + 1);
    
    if ((f = freopen(file, "r", stdin)) == NULL) 
        fatal("freopen file error\n");
    bsize = fread(bufferp, 1, st.st_size, stdin);
    fclose(f);
    
    assert(bsize == st.st_size);
    end = bufferp + bsize;
    *end = EOI;
    cp = line = bufferp;
    lineno = 1;
}

void nextline(void) {
    lineno++;
    for (line = cp; *cp == ' ' || *cp == '\t'; cp++)
        ;
}

int gettok(void) {
    for (;;) {

        while (map[*cp] & BLANK)
            cp++;
        if (cp == end)
            return EOI;            
        token = cp;
        src.file = file;
        src.x = cp - line + 1;
        src.y = lineno;
        
        switch (*cp++) {
        case '/':
            if (*cp == '*') {
                int c = 0;
                for (cp++; *cp != '/' || c != '*';) {
                    if (cp == end) {
                        token = stringn(token, cp - token);
                        fatal("unclosed comment");
                    }
                    c = *cp++;
                    if (map[cp[-1]] & NEWLINE)
                        nextline();
                }
                cp++; continue;
            }
            return '/';
        case '<':
            if (*cp == '=') return LEQ;
            if (*cp == '<') return LSHIFT;
            return '<';
        case '>':
            if (*cp == '=') return GEQ;
            if (*cp == '>') return RSHIFT;
            return '>';
        case '-':
            if (*cp == '>') return DEREF;
            if (*cp == '-') return DECR;
            return '-';
        case '=': return *cp == '=' ? cp++, EQL     : '=';
        case '!': return *cp == '=' ? cp++, NEQ     : '!';
        case '|': return *cp == '|' ? cp++, OROR    : '|';
        case '&': return *cp == '&' ? cp++, ANDAND  : '&';
        case '+': return *cp == '+' ? cp++, INCR    : '+';
        case ';': case ',': case ':':
        case '*': case '~': case '%': case '^': case '?':
        case '[': case ']': case '{': case '}': case '(': case ')':
            return cp[-1];
        case '\n': case '\v': case '\r': case '\f':
            nextline(); continue;
            
        case 'a':
            if (cp[0] == 'u'
            &&  cp[1] == 't'
            &&  cp[2] == 'o'
            && !(map[cp[3]] & (DIGIT | LETTER))) {
                cp = cp + 3;
                token = stringn(token, cp - token);
                return AUTO;
            }
            goto id;
        case 'b':
            if (cp[0] == 'r'
            &&  cp[1] == 'e'
            &&  cp[2] == 'a'
            &&  cp[3] == 'k'
            && !(map[cp[4]] & (DIGIT | LETTER))) {
                cp = cp + 4;
                token = stringn(token, cp - token);
                return BREAK;
            }
            goto id;
        case 'c':
			if (cp[0] == 'a'
			&&  cp[1] == 's'
			&&  cp[2] == 'e'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
				return CASE;
			}
			if (cp[0] == 'h'
			&&  cp[1] == 'a'
			&&  cp[2] == 'r'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
			/*	tsym = chartype->u.sym; */
				return CHAR;
			}			
			if (cp[0] == 'o'
			&&  cp[1] == 'n'
			&&  cp[2] == 's'
			&&  cp[3] == 't'
			&& !(map[cp[4]] & (DIGIT | LETTER))) {
				cp = cp + 4;
				token = stringn(token, cp - token);
				return CONST;
			}
			if (cp[0] == 'o'
			&&  cp[1] == 'n'
			&&  cp[2] == 't'
			&&  cp[3] == 'i'
			&&  cp[4] == 'n'
			&&  cp[5] == 'u'
			&&  cp[6] == 'e'
			&& !(map[cp[7]] & (DIGIT | LETTER))) {
				cp = cp + 7;
				token = stringn(token, cp - token);
				return CONTINUE;
			}
			goto id;
		case 'd':
			if (cp[0] == 'e'
			&&  cp[1] == 'f'
			&&  cp[2] == 'a'
			&&  cp[3] == 'u'
			&&  cp[4] == 'l'
			&&  cp[5] == 't'
			&& !(map[cp[6]] & (DIGIT | LETTER))) {
				cp = cp + 6;
				token = stringn(token, cp - token);
				return DEFAULT;
			}		
			if (cp[0] == 'o'
			&&  cp[1] == 'u'
			&&  cp[2] == 'b'
			&&  cp[3] == 'l'
			&&  cp[4] == 'e'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
			/*	tsym = doubletype->u.sym; */
				return DOUBLE;
			}	
			if (cp[0] == 'o'
			&& !(map[cp[1]] & (DIGIT | LETTER))) {
				cp = cp + 1;
				token = stringn(token, cp - token);
				return DO;
			}			
			goto id;
		case 'e':
			if (cp[0] == 'l'
			&&  cp[1] == 's'
			&&  cp[2] == 'e'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
				return ELSE;
			}		
			if (cp[0] == 'n'
			&&  cp[1] == 'u'
			&&  cp[2] == 'm'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
				return ENUM;
			}	
			if (cp[0] == 'x'
			&&  cp[1] == 't'
			&&  cp[2] == 'e'
			&&  cp[3] == 'r'
			&&  cp[4] == 'n'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return EXTERN;
			}			
			goto id;
		case 'f':
			if (cp[0] == 'l'
			&&  cp[1] == 'o'
			&&  cp[2] == 'a'
			&&  cp[3] == 't'
			&& !(map[cp[4]] & (DIGIT | LETTER))) {
				cp = cp + 4;
				token = stringn(token, cp - token);
			/*	tsym = floattype->sym; */
				return FLOAT;
			}
			if (cp[0] == 'o'
			&&  cp[1] == 'r'
			&& !(map[cp[2]]&(DIGIT|LETTER))) {
				cp = cp + 2;
				token = stringn(token, cp - token);
				return FOR;
			}
			goto id;
		case 'g':
			if (cp[0] == 'o'
			&&  cp[1] == 't'
			&&  cp[2] == 'o'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
				return GOTO;
			}
			goto id;
		case 'i':
			if (cp[0] == 'f'
			&& !(map[cp[1]] & (DIGIT | LETTER))) {
				cp = cp + 1;
				token = stringn(token, cp - token);
				return IF;
			}
			if (cp[0] == 'n'
			&&  cp[1] == 't'
			&& !(map[cp[2]] & (DIGIT | LETTER))) {
				cp = cp + 2;
				token = stringn(token, cp - token);
				tsym = inttype->u.sym;  
				return INT;
			}
			goto id;			
		case 'l':
			if (cp[0] == 'o'
			&&  cp[1] == 'n'
			&&  cp[2] == 'g'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
				return LONG;
			}
			goto id;
		case 'r':
			if (cp[0] == 'e'
			&&  cp[1] == 'g'
			&&  cp[2] == 'i'
			&&  cp[3] == 's'
			&&  cp[4] == 't'
			&&  cp[5] == 'e'
			&&  cp[6] == 'r'
			&& !(map[cp[7]] & (DIGIT | LETTER))) {
				cp = cp + 7;
				token = stringn(token, cp - token);
				return REGISTER;
			}		
			if (cp[0] == 'e'
			&&  cp[1] == 't'
			&&  cp[2] == 'u'
			&&  cp[3] == 'r'
			&&  cp[4] == 'n'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return RETURN;
			}
			goto id;			
		case 's':
			if (cp[0] == 'h'
			&&  cp[1] == 'o'
			&&  cp[2] == 'r'
			&&  cp[3] == 't'
			&& !(map[cp[4]] & (DIGIT | LETTER))) {
				cp = cp + 4;
				token = stringn(token, cp - token);
				return SHORT;
			}
			if (cp[0] == 'i'
			&&  cp[1] == 'g'
			&&  cp[2] == 'n'
			&&  cp[3] == 'e'
			&&  cp[4] == 'd'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return SIGNED;
			}
			if (cp[0] == 'i'
			&&  cp[1] == 'z'
			&&  cp[2] == 'e'
			&&  cp[3] == 'o'
			&&  cp[4] == 'f'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return SIZEOF;
			}
			if (cp[0] == 't'
			&&  cp[1] == 'a'
			&&  cp[2] == 't'
			&&  cp[3] == 'i'
			&&  cp[4] == 'c'
			&& !(map[cp[5]]&(DIGIT|LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return STATIC;
			}
			if (cp[0] == 't'
			&&  cp[1] == 'r'
			&&  cp[2] == 'u'
			&&  cp[3] == 'c'
			&&  cp[4] == 't'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return STRUCT;
			}
			if (cp[0] == 'w'
			&&  cp[1] == 'i'
			&&  cp[2] == 't'
			&&  cp[3] == 'c'
			&&  cp[4] == 'h'
			&& !(map[cp[5]] & (DIGIT | LETTER))) {
				cp = cp + 5;
				token = stringn(token, cp - token);
				return SWITCH;
			}
			goto id;
		case 't':
			if (cp[0] == 'y'
			&&  cp[1] == 'p'
			&&  cp[2] == 'e'
			&&  cp[3] == 'd'
			&&  cp[4] == 'e'
			&&  cp[5] == 'f'
			&& !(map[cp[6]] & (DIGIT | LETTER))) {
				cp = cp + 6;
				token = stringn(token, cp - token);
				return TYPEDEF;
			}
			goto id;
		case 'u':
			if (cp[0] == 'n'
			&&  cp[1] == 'i'
			&&  cp[2] == 'o'
			&&  cp[3] == 'n'
			&& !(map[cp[4]] & (DIGIT | LETTER))) {
				cp = cp + 4;
				token = stringn(token, cp - token);
				return UNION;
			}
			if (cp[0] == 'n'
			&&  cp[1] == 's'
			&&  cp[2] == 'i'
			&&  cp[3] == 'g'
			&&  cp[4] == 'n'
			&&  cp[5] == 'e'
			&&  cp[6] == 'd'
			&& !(map[cp[7]] & (DIGIT | LETTER))) {
				cp = cp + 7;
				token = stringn(token, cp - token);
				return UNSIGNED;
			}
			goto id;
		case 'v':
			if (cp[0] == 'o'
			&&  cp[1] == 'i'
			&&  cp[2] == 'd'
			&& !(map[cp[3]] & (DIGIT | LETTER))) {
				cp = cp + 3;
				token = stringn(token, cp - token);
			/*	tsym = voidtype->u.sym; */
				return VOID;
			}
			if (cp[0] == 'o'
			&&  cp[1] == 'l'
			&&  cp[2] == 'a'
			&&  cp[3] == 't'
			&&  cp[4] == 'i'
			&&  cp[5] == 'l'
			&&  cp[6] == 'e'
			&& !(map[cp[7]] & (DIGIT | LETTER))) {
				cp = cp + 7;
				token = stringn(token, cp - token);
				return VOLATILE;
			}
			goto id;
		case 'w':
			if (cp[0] == 'h'
			&&  cp[1] == 'i'
			&&  cp[2] == 'l'
			&&  cp[3] == 'e'
			&& !(map[cp[4]] & (DIGIT | LETTER))) {
				cp = cp + 4;
				token = stringn(token, cp - token);
				return WHILE;
			}
			goto id;
		case 'h': case 'j': case 'k': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z':	case '_':
id:
            while (map[*cp] & (DIGIT | LETTER))
                cp++;
            token = stringn(token, cp - token);
            tsym = lookup(token, identifiers);
            return ID;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            unsigned long n = 0;

            if (*token == '0' && (*cp == 'x' || *cp == 'X')){
                int d;
                while (++cp != end) {
                    if (map[*cp] & DIGIT)
                        d = *cp - '0';
                    else if (*cp >= 'a' && *cp <= 'f')
                        d = *cp - 'a' + 10;
                    else if (*cp >= 'A' && *cp <= 'F')
                        d = *cp - 'A' + 10;
                    else
                        break;
                    if (n & ~(~0UL >> 4)) {
                        token = stringn(token, cp - token);
                        error("Hex number is overflow");
                    } else
                        n = (n << 4) + d;
                }
                if (cp - token <= 2) {
                    token = stringn(token, cp - token);
                    error("invalid hexadecimal constant");
                }
            /*    tsym = icon(n, overflow, 16); */
            } else if (*token == '0') {
                for (; map[*cp] & DIGIT; cp++) {
                    if (*cp == '8' || *cp == '9') {
                        token = stringn(token, cp - token);
                        error("invalid octal constant");
                    }
                    if (n & ~(~0UL >> 3)) {
                        token = stringn(token, cp - token);
                        error("Octal number is overflow");
                    }
                    else
                        n = (n << 3) + (*cp - '0');
                }
                if (*cp == '.' || *cp == 'e' || *cp == 'E') {
                /*    tsym = fcon(); */
                    token = stringn(token, cp - token);
                    return FCON;
                }
            /*    tsym = icon(n, 8); */
            } else {
                for (n = *token-'0'; map[*cp] & DIGIT;) {
                    int d = *cp++ - '0';
                    if (n > (ULONG_MAX - d) / 10) {
                        token = stringn(token, cp - token);
                        error("Decimal number is overflow");
                    }
                    else
                        n = 10 * n + d;
                }
                if (*cp == '.' || *cp == 'e' || *cp == 'E') {
                    token = stringn(token, cp - token);
                /*    tsym = fcon(); */
                    return FCON;
                }
            /*    tsym = icon(n, 10); */
            }
            token = stringn(token, cp - token);
            return ICON;
        }

        default:
            if (cp == end)
                fprintf(stderr, "Input buffer is end.\n");
            else {
                token = stringn(token, 10);
                error("unsurport token");
            }
            return cp[-1];
        }




        
    }
}





