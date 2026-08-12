/*
 * parser.c --
 *
 * Copyright 2011, 2012, 2013, 2014, 2015, 2016 Free Software Foundation, Inc.
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

#ifdef HAVE_CONFIG_H
/* <> is used to support out-of-source autoconf builds. */
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "common.h"
#include "backend.h"
#include "frontend.h"
#include "parser.h"
#include "moves.h"


extern Board boards[MAX_MOVES];
extern int PosFlags(int nr);
int yyboardindex;
int yyskipmoves = FALSE;
/* a bit ridiculous size? */
char currentMoveString[4096];
char * yy_text;

#define PARSEBUFSIZE 10000

static FILE * inputFile;
static char *inPtr, *parsePtr, *parseStart;
static char inputBuf[PARSEBUFSIZE];
static char yytext[PARSEBUFSIZE];
static char fromString = 0, lastChar = '\n';

#define NOTHING 0
#define NUMERIC 1
#define ALPHABETIC 2
#define BADNUMBER (-2000000000)

#define XCO 0
#define YCO 53
#define PIECE 94
#define MISC 155
#define JIS 200

/* clang-format off */
unsigned char kanjiTab[] = {
 /* kanji notation for arabic digits */
 '1', 0357, 0274, 0221,
 '2', 0357, 0274, 0222,
 '3', 0357, 0274, 0223,
 '4', 0357, 0274, 0224,
 '5', 0357, 0274, 0225,
 '6', 0357, 0274, 0226,
 '7', 0357, 0274, 0227,
 '8', 0357, 0274, 0230,
 '9', 0357, 0274, 0231,
 'x', 0345, 0220, 0214,
 /* sente */
 's', 0345, 0205, 0210,
 /* gote */
 'g', 0345, 0276, 0214,
 /* resign */
 '-', 0346, 0212, 0225,
 0,
 /* In reality, these are numbers in Japanese: a=1, b=2, etc. */
 'a', 0344, 0270, 0200,
 'b', 0344, 0272, 0214,
 'c', 0344, 0270, 0211,
 'd', 0345, 0233, 0233,
 'e', 0344, 0272, 0224,
 'f', 0345, 0205, 0255,
 'g', 0344, 0270, 0203,
 'h', 0345, 0205, 0253,
 'i', 0344, 0271, 0235,
 ' ', 0343, 0200, 0200,
 0,
 /* piece names */
 'K', 0347, 0216, 0211,
 'K', 0347, 0216, 0213,
 'G', 0351, 0207, 0221,
 'S', 0351, 0212, 0200,
 'R', 0351, 0243, 0233,
 'B', 0350, 0247, 0222,
 'N', 0346, 0241, 0202,
 'L', 0351, 0246, 0231,
 'P', 0346, 0255, 0251,
 'r', 0351, 0276, 0215,
 'b', 0351, 0246, 0254,
 'p', 0343, 0201, 0250,
 'r', 0347, 0253, 0234,
 '+', 0346, 0210, 0220,
 'G', 0, 0, 0,
 0,
 /* helper */
 '+', 0346, 0210, 0220,
 '@', 0346, 0211, 0223,
 /* player */
 'p', 0346, 0211, 0213,
 ':', 0357, 0274, 0232,
 '-', 0344, 0272, 0206,
 'f', 0344, 0270, 0212,
 's', 0345, 0257, 0204,
 'b', 0345, 0274, 0225,
 'r', 0345, 0267, 0246,
 'l', 0345, 0217, 0263,
 'v', 0347, 0233, 0264,
 0,
 /* shift-JIS */
 '1', 0202, 0120, 0,
 '2', 0202, 0121, 0,
 '3', 0202, 0122, 0,
 '4', 0202, 0123, 0,
 '5', 0202, 0124, 0,
 '6', 0202, 0125, 0,
 '7', 0202, 0126, 0,
 '8', 0202, 0127, 0,
 '9', 0202, 0130, 0,
 'x', 0223, 0257, 0,
 's', 0220, 0346, 0,
 'g', 0214, 0343, 0,
 '-', 0223, 0212, 0,
 0,
 'a', 0210, 0352, 0,
 'b', 0223, 0361, 0,
 'c', 0216, 0117, 0,
 'd', 0216, 0154, 0,
 'e', 0214, 0334, 0,
 'f', 0230, 0132, 0,
 'g', 0216, 0265, 0,
 'h', 0224, 0252, 0,
 'i', 0213, 0343, 0,
 ' ', 0201, 0100, 0,
 0,
 'K', 0213, 0312, 0,
 'K', 0213, 0312, 0,
 'G', 0213, 0340, 0,
 'S', 0213, 0342, 0,
 'R', 0224, 0362, 0,
 'B', 0212, 0160, 0,
 'N', 0214, 0152, 0,
 'L', 0215, 0201, 0,
 'P', 0225, 0340, 0,
 'r', 0227, 0264, 0,
 'b', 0224, 0156, 0,
 'p', 0202, 0306, 0,
 'r', 0227, 0263, 0,
 '+', 0220, 0254, 0,
 'G', 0, 0, 0,
 0,
 '+', 0220, 0254, 0,
 '@', 0221, 0305, 0,
 'p', 0216, 0350, 0,
 ':', 0201, 0106, 0,
 '-', 0227, 0271, 0,
 'f', 0217, 0343, 0,
 's', 0212, 0361, 0,
 'b', 0210, 0370, 0,
 'r', 0215, 0266, 0,
 'l', 0211, 0105, 0,
 'v', 0222, 0274, 0,
 0,
};
/* clang-format on */

int NextUnit(char ** p);

int kifu = 0, xqUBB = 0;

char GetKanji(char ** p, int start) {
    unsigned char * q = *(unsigned char **)p;
    int i;

    if ((*q & 0x80) == 0) {
        /* Plain ASCII, so refuse to parse. */
        return 0;
    }
    if ((**p & 0xc0) == 0x80) {
        /* This is an illegal starting code in utf-8, so assume shift-JIS. */
        for (i = start + JIS; kanjiTab[i]; i += 4) {
            if (q[0] == kanjiTab[i + 1] && q[1] == kanjiTab[i + 2]) {
                (*p) += 2;
                kifu = 0x80;
                return kanjiTab[i];
            }
        }
        /* Assume this is an unrecognized kanji when reading kif files. */
        (*p) += (kifu ? 2 : 1);
        return 0;
    }

    for (i = start; kanjiTab[i]; i += 4) {
        if (q[0] == kanjiTab[i + 1] && q[1] == kanjiTab[i + 2] && q[2] == kanjiTab[i + 3]) {
            (*p) += 3;
            kifu = 0x80;
            return kanjiTab[i];
        }
    }

    if ((q[0] & 0xe0) == 0xc0 && (q[1] & 0xc0) == 0x80) {
        (*p) += 2;
    } else /* For now, skip unrecognized utf-8 characters. */
        if ((q[0] & 0xf0) == 0xe0 && (q[1] & 0xc0) == 0x80 && (q[2] & 0xc0) == 0x80) {
            (*p) += 3;
        } else if ((q[0] & 0xf8) == 0xf0 && (q[1] & 0xc0) == 0x80 && (q[2] & 0xc0) == 0x80 && (q[3] & 0xc0) == 0x80) {
            (*p) += 4;
        } else if (**p & 0x80) {
            /* Not valid utf-8. */
            return -1;
        }

    /* Unrecognized but valid kanji (skipped), or plain ASCII. */
    return 0;
}

int KifuMove(char ** p) {
    static char buf[MSG_SIZ];
    char *ptr = buf + 3, *q, k;
    int wom = quickFlag ? quickFlag & 1 : WhiteOnMove(yyboardindex);
    k = GetKanji(p, XCO);
    if (k < 0) {
        /* TODO: must try shift-JIS here. */
        (*p)++;
        return Nothing;
    }
    if (k >= '1' && k <= '9') {
        buf[0] = k;
        buf[1] = GetKanji(p, YCO);
        /* to-square coords */
    } else if (k == 'x') {
        if (GetKanji(p, YCO) != ' ') {
            /* Skip spacer kanji after recapture. */
            (*p) -= 3;
        }
    } else if ((k == 's' || k == 'g') && GetKanji(p, MISC) == 'p' && GetKanji(p, MISC) == ':') {
        /* Player name. */
        /* Construct PGN tag. */
        snprintf(yytext, MSG_SIZ, "[%s \"", k == 's' ? "White" : "Black");
        for (q = yytext + 8; **p && **p != '\n' && **p != '\r' && q < yytext + MSG_SIZ;) {
            *q++ = *(*p)++;
        }
        strcpy(q, "\"]\n");
        parseStart = yytext;
        lastChar = '\n';
        return PGNTag;
    } else if (k == '-' && GetKanji(p, MISC) == '-') {
        /* Resign. */
        int res;
        parseStart = yytext;
        if (wom) {
            res = BlackWins, strcpy(yytext, "{sente resigns} 0-1");
        } else {
            res = WhiteWins, strcpy(yytext, "{gote resigns} 1-0");
        }
        return res;
    } else {
        while (**p && **p != '\n') {
            /* Unrecognized Japanese kanji: skip to end of line. */
            (*p)++;
        }
        return Nothing;
    }
    /* Piece ID. */
    buf[3] = GetKanji(p, PIECE);
    if (buf[3] == '+') {
        /* +N, +L, +S */
        buf[2] = '+';
        buf[3] = GetKanji(p, PIECE);
    }
    k = GetKanji(p, MISC);
    if (k == '@') {
        /* Drop move. */
        buf[4] = '@', buf[5] = buf[0], buf[6] = buf[1];
        buf[7] = NULLCHAR;
        if (appData.debugMode) {
            fprintf(debugFP, "kifu drop %s\n", ptr);
        }
        return NextUnit(&ptr);
    }

    kifu = 0x80;
    do {
        /* Read disambiguation (and promotion) kanji. */
        switch (k) {
        case '+':
            kifu |= 1;
            break;
        case 'f':
            kifu |= 2;
            break;
        case 'b':
            kifu |= 4;
            break;
        case 's':
            kifu |= 8;
            break;
        case 'l':
            kifu |= 0x10;
            break;
        case 'r':
            kifu |= 0x20;
            break;
        case 'v':
            kifu |= 0x40;
            break;
        }
    } while ((k = GetKanji(p, MISC)));

    if (**p == '(' && (*p)[3] == ')') {
        /* kif disambiguation */
        buf[4] = (*p)[1];
        buf[5] = (*p)[2] + 'a' - '1';
        buf[6] = buf[0];
        buf[7] = buf[1];
        buf[8] = (kifu & 1) * '+';
        buf[9] = NULLCHAR;
        (*p) += 4;
        /* Strip off piece name if we know full from-square. */
        ptr++;
        if (appData.debugMode) {
            fprintf(debugFP, "kifu move %s\n", ptr);
        }
        return NextUnit(&ptr);
    } else {
        /* kif2 */
        char * q = buf + 4;
        if (islower(buf[3])) {
            /* Kludge: kanji for promoted types translate as lower case, so prefix with '+'. */
            buf[3] += 'A' - 'a';
            buf[2] = '+';
            ptr--;
        }
        if (kifu * ~1) {
            /* Disambiguation was given, and thus is probably needed. */
            if (buf[3] != 'B' && buf[3] != 'R') {
                /* Stepper, so distance must be <= 1: N or L never need vertical disambiguation! */
                if (kifu & 0x10) {
                    /* Translate left/right/straight to PSN file disambiguators. */
                    *q++ = buf[0] - (wom ? -1 : 1);
                }
                if (kifu & 0x20) {
                    *q++ = buf[0] + (wom ? -1 : 1);
                }
                if (kifu & 0x40) {
                    /* Kludge: 'straight' only needs disambiguation if forward! */
                    *q++ = buf[0], kifu |= 2;
                }
                if (kifu & 2) {
                    /* Translate forward/backward/sideway to PSN rank disambiguators. */
                    *q++ = buf[1] + (wom ? -1 : 1);
                }
                if (kifu & 4) {
                    *q++ = buf[1] - (wom ? -1 : 1);
                }
                if (kifu & 8) {
                    *q++ = buf[1];
                }
            }
            /* For B, R, +B and +R, it gets ugly because we cannot deduce the distance: the Disambiguate callback has to directly
            look at 'kifu'. */
        }
        *q++ = buf[0];
        *q++ = buf[1];
        *q++ = (kifu & 1) * '+';
        *q = NULLCHAR;
        if (appData.debugMode) {
            fprintf(debugFP, "kif2 move %s\n", ptr);
        }
        return NextUnit(&ptr);
    }
}

/* Read one line from the input file, and append it to the buffer. */
int ReadLine(void) {
    int c;
    char * start = inPtr;
    if (fromString) {
        /* We're parsing a string, so the end is a hard end. */
        return 0;
    }
    if (!inputFile) {
        return 0;
    }
    while ((c = fgetc(inputFile)) != EOF) {
        *inPtr++ = c;
        if (c == '\n') {
            *inPtr = NULLCHAR;
            return 1;
        }
        if (inPtr - inputBuf > PARSEBUFSIZE - 2) {
            /* Prevent crashing upon overflow. */
            inPtr--;
        }
    }
    if (inPtr == start) {
        return 0;
    }
    /* Repair a missing linefeed at EOF. */
    *inPtr++ = '\n';
    *inPtr = NULLCHAR;
    return 1;
}

/* Line-spanning skip to mentioned character or EOF. */
int Scan(char c, char ** p) {
    do {
        while (**p) {
            if (*(*p)++ == c) {
                return 0;
            }
        }
    } while (ReadLine());
    /* No closing bracket, so force match for the entire rest of the file. */
    return 1;
}

/* Skip spaces, tabs, and newlines.  Return 1 if anything was skipped. */
int SkipWhite(char ** p) {
    char * start = *p;
    do {
        while (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r') {
            (*p)++;
        }
    } while (**p == NULLCHAR && ReadLine());
    return *p != start;
}

static inline int Match(char * pattern, char ** ptr) {
    char *p = pattern, *s = *ptr;
    while (*p && (*p == *s++ || (s[-1] == '\r' && *p--))) {
        p++;
    }
    if (*p == 0) {
        *ptr = s;
        return 1;
    }
    /* No match, no ptr update. */
    return 0;
}

static inline int Word(char * pattern, char ** p) {
    if (Match(pattern, p)) {
        return 1;
    }
    if (*pattern >= 'a' && *pattern <= 'z' && *pattern - **p == 'a' - 'A') {  /* capitalized */
        (*p)++;
        if (Match(pattern + 1, p)) {
            return 1;
        }
        (*p)--;
    }
    return 0;
}

int Verb(char * pattern, char ** p) {
    int res = Word(pattern, p);
    if (res && !Match("s", p)) {
        /* Consume any conjugation suffix that might be present. */
        Match("ed", p);
    }
    return res;
}

int Number(char ** p) {
    int val = 0;
    if (**p < '0' || **p > '9') {
        return BADNUMBER;
    }
    while (**p >= '0' && **p <= '9') {
        val = 10 * val + *(*p)++ - '0';
    }
    return val;
}

int RdTime(char c, char ** p) {
    /* Increment *p, because it was pointing to the opening ( or {. */
    char * start = ++(*p);
    char * sec;
    if (Number(p) == BADNUMBER) {
        return 0;
    }
    sec = *p;
    if (Match(":", p) && Number(p) != BADNUMBER && *p - sec == 3) {
        /* Well-formed. */
        sec = *p;
        if (Match(".", p) && Number(p) != BADNUMBER && *(*p)++ == c) {
            /* Well-formed fraction. */
            return 1;
        }
        *p = sec;
        if (*(*p)++ == c) {
            /* Matching bracket without fraction. */
            return 1;
        }
    }
    /* Failure. */
    *p = start;
    return 0;
}

char PromoSuffix(char ** p) {
    char * start = *p;
    if (**p == ' ') {
        /* Common case, test explicitly for speed. */
        return NULLCHAR;
    }
    if (**p == 'e' && (Match("ep", p) || Match("e.p.", p))) {
        /* Non-compliant e.p. suffix is no promoChar! */
        *p = start;
        return NULLCHAR;
    }
    if (**p == '+' && IS_SHOGI(gameInfo.variant)) {
        (*p)++;
        return '+';
    }
    if (**p == '=' || ((gameInfo.variant == VariantSChess) && **p == '/')) {
        /* Optional '=' (or '/' for Seirawan gating). */
        (*p)++;
    }
    if (**p == '(' && (*p)[2] == ')' && isalpha((*p)[1])) {
        (*p) += 3;
        return ToLower((*p)[-2]);
    }
    if (isalpha(**p) && **p != 'x') {
        /* TODO: Consider reserving 'x' for multi-leg captures. */
        return ToLower(*(*p)++);
    }
    if (*p != start) {
        /* Must be the optional = (or =+) */
        return **p == '+' ? *(*p)++ : '=';
    }
    /* No suffix detected. */
    return NULLCHAR;
}

/* Main parser routine. */
int NextUnit(char ** p) {
    int coord[4], n, result, piece, i;
    char type[4], promoted, separator, slash, *oldp, *commentEnd, c;
    int wom = quickFlag ? quickFlag & 1 : WhiteOnMove(yyboardindex);

    /* Try white first, because it is so common. */
    if (**p == ' ' || **p == '\n' || **p == '\t') {
        parseStart = (*p)++;
        return Nothing;
    }

    /* Check if there is actually something to parse at all. */
    if (**p == NULLCHAR) {
        if (fromString) {
            /* We are parsing a string, so the end is really the end. */
            return 0;
        }
        *p = inPtr = parseStart = inputBuf;
        if (!ReadLine()) {
            /* EOF */
            return 0;
        }
    } else if (inPtr > inputBuf + PARSEBUFSIZE / 2) {
        /* buffer fills up with already parsed stuff */
        char * q = *p;
        char * r = inputBuf;
        while ((*r++ = *q++)) {}
        *p = inputBuf;
        inPtr = r - 1;
    }
    /* Remember where we begin. */
    oldp = *p;
    parseStart = *p;

    /* Attempt to recognize a SAN move in the leading non-blank text. */
    piece = separator = promoted = slash = n = 0;
    for (i = 0; i < 4; i++) {
        coord[i] = -1, type[i] = NOTHING;
    }
    if (**p & 0x80) {
        /* Non-ASCII.  It could be some kanji notation for Shogi or Xiangqi. */
        return KifuMove(p);
    }
    if (**p == '+') {
        (*p)++, promoted++;
    }
    if (**p >= 'a' && **p <= 'z' && (*p)[1] == '@') {
        piece = *(*p)++ + 'A' - 'a';
    } else if (**p >= 'A' && **p <= 'Z') {
        static char s[] = SUFFIXES;
        char * q;
        /* N.B.: We could test for 2-byte non-ASCII names here. */
        piece = *(*p)++;
        if ((q = strchr(s, **p))) {
            (*p)++, piece += 64 * (q - s + 1);
        }
        if (**p == '/') {
            slash = *(*p)++;
        }
    }

    if (xqUBB) {
        /* Xiangqi UBB movelist. */
        while (isdigit(**p)) {
            type[n] = NUMERIC;
            coord[n++] = *(*p)++ - '0';
            if (n >= 4) {
                break;
            }
        }
        if (n < 4) {
            *p -= n, xqUBB = n = 0;
        } else {
            type[0] = type[2] = ALPHABETIC, coord[1] = 9 - coord[1], coord[3] = 9 - coord[3];
        }
    }
    while (n < 4) {
        if (**p >= 'a' && **p < 'x') {
            coord[n] = *(*p)++ - 'a', type[n++] = ALPHABETIC;
        } else if ((i = Number(p)) != BADNUMBER) {
            coord[n] = i, type[n++] = NUMERIC;
        } else {
            break;
        }
        if (n == 2 && type[0] == type[1]) {
            /* If the two types are identical, then the opposite type in between them must have been missing. */
            type[2] = type[1];
            coord[2] = coord[1];
            type[1] = NOTHING;
            coord[1] = -1;
            n++;
        }
    }

    /* We always get here, and might have read a +, a piece, and upto 4 potential coordinates. */
    if (n <= 2) {
        /* could be from-square or disambiguator, when -:xX follow, or drop with @ directly after piece, but also to-square */
        if (**p == '-' || **p == ':' || **p == 'x' || **p == 'X' || /* these cannot be move suffix, so to-square must follow */
         (**p == '@' || **p == '*') && n == 0 && !promoted && piece) { /* P@ must also be followed by to-square */
            separator = *(*p)++;
            if (n == 1) {
                /* must be disambiguator, but we do not know which yet */
                coord[1] = coord[0];
            }
            n = 2;
            while (n < 4) {
                /* attempt to read to-square */
                if (**p >= 'a' && **p < 'x') {
                    coord[n] = *(*p)++ - 'a', type[n++] = ALPHABETIC;
                } else if ((i = Number(p)) != BADNUMBER) {
                    coord[n] = i, type[n++] = NUMERIC;
                } else {
                    break;
                }
            }
        } else if ((**p == '+' || **p == '=') && n == 1 && piece && type[0] == NUMERIC) {
            /* can be traditional Xiangqi notation */
            separator = *(*p)++;
            n = 2;
            if ((i = Number(p)) != BADNUMBER) {
                coord[n] = i, type[n++] = NUMERIC;
            }
        } else if (n == 2) {
            /* only one square mentioned, must be to-square */
            while (n < 4) {
                coord[n] = coord[n - 2], type[n] = type[n - 2], coord[n - 2] = -1, type[n - 2] = NOTHING;
                n++;
            }
        }
    } else if (n == 3 && type[1] != NOTHING) {
        /* must be hyphenless disambiguator + to-square */
        for (i = 3; i > 0; i--) {
            /* move to-square to where it belongs */
            coord[i] = coord[i - 1];
            type[i] = type[i - 1];
        }
        /* disambiguator goes in first two positions */
        type[1] = NOTHING;
        n = 4;
    }
    /* We always get here.  The move must be completely read now, with to-square coord(s) at its end. */
    if (n == 3) {
        /* Incomplete to-square.  Could be Xiangqi traditional, or stuff like fxg. */
        if (piece && type[1] == NOTHING && type[0] == NUMERIC && type[2] == NUMERIC &&
         (separator == '+' || separator == '=' || separator == '-')) {
            /* Xiangqi traditional */
            /* For now, treat as invalid. */
            return ImpossibleMove;
        }
        /* fxg stuff, but also things like 0-0, 0-1 and 1-0 */
        if (!piece && type[1] == NOTHING && type[0] == ALPHABETIC && type[2] == ALPHABETIC &&
         (coord[0] != 14 || coord[2] != 14) /* reserve oo for castling! */) {
            piece = 'P';
            /* kludge alert: fake full to-square */
            n = 4;
        }
    } else if (n == 1 && type[0] == NUMERIC && coord[0] > 1) {
        while (**p == '.') {
            (*p)++;
        }
        return Nothing;
    }  /* fast exit for move numbers */
    if (n == 4 && type[2] != type[3] &&  /* we have a valid to-square (kludge: type[3] can be NOTHING on fxg type move) */
     (piece || !promoted) &&  /* promoted indicator only valid on named piece type */
     (type[2] == ALPHABETIC || IS_SHOGI(gameInfo.variant))) {  /* in Shogi also allow alphabetic rank */
        DisambiguateClosure cl;
        int fromX, fromY, toX, toY;

        if (slash && (!piece || type[1] == NOTHING)) {
            /* slash after piece only in ICS long format */
            goto badMove;
        }
        if (yyskipmoves) {
            /* not disambiguated */
            return (int)AmbiguousMove;
        }

        if (type[2] == NUMERIC) {  /* alpha-rank */
            coord[2] = BOARD_RGHT - BOARD_LEFT - coord[2];
            coord[3] = BOARD_HEIGHT - coord[3];
            if (coord[0] >= 0) {
                coord[0] = BOARD_RGHT - BOARD_LEFT - coord[0];
            }
            if (coord[1] >= 0) {
                coord[1] = BOARD_HEIGHT - coord[1];
            }
        }
        toX = cl.ftIn = (currentMoveString[2] = coord[2] + 'a') - AAA;
        toY = cl.rtIn = (currentMoveString[3] = coord[3] + '0') - ONE;
        if (type[3] == NOTHING) {
            /* For fxg-type moves, ask for toY disambiguation. */
            cl.rtIn = -1;
        } else if (toY >= BOARD_HEIGHT || toY < 0) {
            /* vert off-board to-square */
            return ImpossibleMove;
        }
        if (toX < BOARD_LEFT || toX >= BOARD_RGHT) {
            return ImpossibleMove;
        }
        if (piece) {
            cl.pieceIn = CharToPiece(wom ? piece : piece + 'a' - 'A');
            if (cl.pieceIn == EmptySquare) {
                /* non-existent piece */
                return ImpossibleMove;
            }
            if (promoted) {
                cl.pieceIn = (ChessSquare)(CHUPROMOTED(cl.pieceIn));
            }
        } else {
            cl.pieceIn = EmptySquare;
        }
        if (separator == '@' || separator == '*') {
            /* drop move. We only get here without from-square or promoted piece */
            fromY = DROP_RANK;
            fromX = cl.pieceIn;
            currentMoveString[0] = piece;
            currentMoveString[1] = '@';
            currentMoveString[4] = NULLCHAR;
            return LegalityTest(
             boards[yyboardindex], PosFlags(yyboardindex) & ~F_MANDATORY_CAPTURE, fromY, fromX, toY, toX, NULLCHAR);
        }
        if (type[1] == NOTHING && type[0] != NOTHING) {
            /* there is a disambiguator */
            if (type[0] != type[2]) {
                /* it was a rank-disambiguator */
                coord[0] = -1;
                type[1] = type[0];
                type[0] = NOTHING;
            }
        }
        if (type[1] != type[2] &&  /* means fromY is of opposite type as ToX, or NOTHING */
         (type[0] == NOTHING || type[0] == type[2])) {  /* well formed */
            int suffix = 7;
            fromX = (currentMoveString[0] = coord[0] + 'a') - AAA;
            fromY = (currentMoveString[1] = coord[1] + '0') - ONE;
            currentMoveString[4] = cl.promoCharIn = PromoSuffix(p);
            currentMoveString[5] = NULLCHAR;
            if (**p == ',' && gameInfo.variant == VariantDuck) {
                /* Duck square follows */
                currentMoveString[7] = currentMoveString[4];
                currentMoveString[4] = ';';
                currentMoveString[5] = *++*p;
                killX = **p - AAA;
                currentMoveString[6] = *++*p;
                killY = *(*p)++ - ONE;
                currentMoveString[8] = NULLCHAR;
            }
            if (**p == 'x' && !cl.promoCharIn) {
                /* other leg follows */
                char * q = *p;
                int x = *++*p, y;
                ++*p;
                y = Number(p);
                if (**p == '-' || **p == 'x') {
                    /* Three-leg move! */
                    /* What we thought was a to-square is in fact the first kill square of two. */
                    currentMoveString[7] = (kill2X = toX) + AAA;
                    currentMoveString[8] = (kill2Y = toY) + ONE; /* append it after 2nd kill-square */
                    toX = x - AAA;  /* kludge alert: this will become 2nd kill square */
                    toY = y + '0' - ONE;
                    suffix += 2;
                } else {
                    /* Two-leg move: rewind to leave reading of the second leg to code below. */
                    *p = q;
                }
            }
            if (!cl.promoCharIn && (**p == '-' || **p == 'x')) {
                /* Lion-type multi-leg move */
                /* What we thought was a to-square is in fact a kill square. */
                currentMoveString[5] = (killX = toX) + AAA;
                currentMoveString[6] = (killY = toY) + ONE;  /* append it as suffix behind long algebraic move */
                currentMoveString[4] = ';';
                currentMoveString[suffix + 1] = NULLCHAR;
                /* Read new to-square.  (VERY non-robust!  Assumes correct (non-alpha-rank) syntax, and messes up on errors.) */
                toX = cl.ftIn = (currentMoveString[2] = *++*p) - AAA;
                ++*p;
                toY = cl.rtIn = (currentMoveString[3] = Number(p) + '0') - ONE;
                currentMoveString[suffix] = cl.promoCharIn = PromoSuffix(p);
            }
            if (type[0] != NOTHING && type[1] != NOTHING && type[3] != NOTHING) {
                /* fully specified. */
                ChessSquare realPiece = boards[yyboardindex][fromY][fromX];
                /* Note that Disambiguate does not work for illegal moves, but flags them as impossible */
                if (piece) {
                    /* Check if correct piece is indicated. */
                    if (PieceToChar(realPiece) == '~') {
                        realPiece = (ChessSquare)(DEMOTED(realPiece));
                    }
                    if (!(appData.icsActive && PieceToChar(realPiece) == '+') /* trust ICS if it moves promoted pieces */
                     && piece && realPiece != cl.pieceIn) {
                        return ImpossibleMove;
                    }
                } else if (!separator && **p == '+') {
                    /* Could be a protocol move, where bare '+' suffix means Shogi-style promotion. */
                    if (PieceToChar(CHUPROMOTED(realPiece)) == '+') {
                        /* It seems to be a protocol move. */
                        if (!autoProm[realPiece]) {
                            **p++;
                        } else {
                            /* For now, ignore promosuffix on auto-promoting pieces' protocol moves. */
                            /* Append promochar after all. */
                            currentMoveString[4] = cl.promoCharIn = *(*p)++;
                        }
                    }
                }
                result = LegalityTest(boards[yyboardindex], PosFlags(yyboardindex), fromY, fromX, toY, toX, cl.promoCharIn);
                if (currentMoveString[4] == NULLCHAR) {
                    /* Supply missing mandatory promotion character. */
                    if (result == WhitePromotion || result == BlackPromotion) {
                        switch (gameInfo.variant) {
                        case VariantCourier:
                        case VariantShatranj:
                            currentMoveString[4] = PieceToChar(BlackFerz);
                            break;
                        case VariantGreat:
                            currentMoveString[4] = PieceToChar(BlackMan);
                            break;
                        case VariantShogi:
                            currentMoveString[4] = '+';
                            break;
                        case VariantSChess:
                            if (!gameInfo.holdingsSize) {
                                /* missing auto-gating */
                                ChessSquare p = boards[yyboardindex][fromY ^ 1][fromX];
                                if (p == DarkSquare) {
                                    p = boards[yyboardindex][fromY ^ 1][toX < fromX ? BOARD_LEFT : BOARD_RGHT - 1];
                                }
                                currentMoveString[4] = ToLower(PieceToChar(p));
                                break;
                            }
                            /* Intentionally fall through. */
                        default:
                            currentMoveString[4] = PieceToChar(BlackQueen);
                        }
                    } else if (result == WhiteNonPromotion || result == BlackNonPromotion) {
                        currentMoveString[4] = '=';
                    }
                } else if (appData.testLegality &&
                 gameInfo.variant != VariantSChess &&  /* strip off unnecessary and false promo characters */
                 !(result == WhitePromotion || result == BlackPromotion || result == WhiteNonPromotion ||
                  result == BlackNonPromotion)) {
                    currentMoveString[4] = NULLCHAR;
                }
                return result;
            } else if (cl.pieceIn == EmptySquare) {
                cl.pieceIn = wom ? WhitePawn : BlackPawn;
            }
            cl.ffIn = type[0] == NOTHING ? -1 : coord[0] + 'a' - AAA;
            cl.rfIn = type[1] == NOTHING ? -1 : coord[1] + '0' - ONE;

            Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);

            if (cl.kind == ImpossibleMove && !piece && type[1] == NOTHING  /* fxg5 type */
             && toY == (wom ? 4 : 3)) {  /* could be improperly written e.p. */
                cl.rtIn += wom ? 1 : -1;  /* shift target square to e.p. square */
                Disambiguate(boards[yyboardindex], PosFlags(yyboardindex), &cl);
                if ((cl.kind != WhiteCapturesEnPassant && cl.kind != BlackCapturesEnPassant)) {
                    /* Nice try, but no cigar. */
                    return ImpossibleMove;
                }
            }

            currentMoveString[0] = cl.ff + AAA;
            currentMoveString[1] = cl.rf + ONE;
            currentMoveString[3] = cl.rt + ONE;
            if (killX < 0) {  /* [HGM] lion: do not overwrite kill-square suffix */
                currentMoveString[4] = cl.promoChar;
            }

            if ((cl.kind == WhiteCapturesEnPassant || cl.kind == BlackCapturesEnPassant) && !Match("ep", p)) {
                Match("e.p.", p);
            }

            return (int)cl.kind;
        }
    }
badMove:
    /* We failed to find an algebraic move. */
    *p = oldp;

    /* Next, we check for some common symbols where the first character commits us to things that cannot possibly be a move. */

    /* ********* PGN tags ****************************************** */
    if (**p == '[') {
        oldp = ++(*p);
        kifu = 0;
        if (Match("--", p)) {  /* "[--" could be start of position diagram */
            if (!Scan(']', p) && (*p)[-3] == '-' && (*p)[-2] == '-') {
                return PositionDiagram;
            }
            *p = oldp;
        }
        SkipWhite(p);
        if (Match("DhtmlXQ", p)) {  /* Xiangqi UBB tags */
            int res = Nothing;
            if (**p == ']') {
                strcpy(parseStart = yytext, "[Variant \"xiangqi\"]"), res = PGNTag;
            } else if (Match("_movelist", p)) {
                xqUBB = 1;
            } else {
                Scan(']', p);
            }
            Scan(']', p);  /* for non-movelist tags this skips to the closing tag (disarming any enclosed kanji)! */
            return res;
        } else if (Match("/DhtmlXQ", p)) {
            Scan(']', p);
            return Nothing;
        }
        if (isdigit(**p) || isalpha(**p)) {
            do {
                (*p)++;
            } while (isdigit(**p) || isalpha(**p) || **p == '+' || **p == '-' || **p == '=' || **p == '_' || **p == '#');
            SkipWhite(p);
            if (**p == '"') {
                (*p)++;
                while (**p != '\n' && (*(*p)++ != '"' || (*p)[-2] == '\\'))
                    ;  /* look for unescaped quote */
                if ((*p)[-1] != '"') {
                    /* String-closing delimiter missing. */
                    *p = oldp;
                    Scan(']', p);
                    return Comment;
                }
                SkipWhite(p);
                if (*(*p)++ == ']') {
                    return PGNTag;
                }
            }
        }
        Scan(']', p);
        return Comment;
    }

    /* SAN Castling */
    if (**p == 'O' || **p == 'o' || (**p == '0' && !Match("00:", p) /* exclude 00 within timestamps */)) {
        int castlingType = 0;
        if (Match("O-O-O", p) || Match("o-o-o", p) || Match("0-0-0", p) || Match("OOO", p) || Match("ooo", p) || Match("000", p)) {
            castlingType = 2;
        } else if (Match("O-O", p) || Match("o-o", p) || Match("0-0", p) || Match("OO", p) || Match("oo", p) || Match("00", p)) {
            castlingType = 1;
        }
        if (castlingType) {
            /* Code from old parser, collapsed for both castling types, and streamlined a bit. */
            int rf, ff, rt, ft;
            ChessSquare king;
            char promo = NULLCHAR, gate = 0;

            if (gameInfo.variant == VariantSChess) {
                promo = PromoSuffix(p);
                if (promo && **p >= 'a' && **p < AAA + BOARD_RGHT) {
                    gate = *(*p)++;
                }
            }

            if (yyskipmoves) {
                /* Not disambiguated. */
                return (int)AmbiguousMove;
            }

            if (wom) {
                rf = castlingRank[0];
                rt = castlingRank[0];
                king = WhiteKing;
            } else {
                rf = castlingRank[3];
                rt = castlingRank[3];
                king = BlackKing;
            }
            /* This would be the d-file. */
            ff = (BOARD_WIDTH - 1) >> 1;
            if (boards[yyboardindex][rf][ff] == king) {
                /* ICS wild castling */
                ft = castlingType == 1 ? BOARD_LEFT + 1 : (gameInfo.variant == VariantJanus ? BOARD_RGHT - 2 : BOARD_RGHT - 3);
            } else {
                char * q;
                /* This would be the e-file. */
                ff = BOARD_WIDTH >> 1;
                ft = castlingType == 1 ? BOARD_RGHT - 2 : BOARD_LEFT + 2;
                if (pieceDesc[king] && (q = strchr(pieceDesc[king], 'O'))) {
                    /* Redefined to non-default King stride. */
                    ft = (castlingType == 1 ? ff + atoi(q + 1) : ff - atoi(q + 1));
                }
            }
            if (PosFlags(0) & F_FRC_TYPE_CASTLING) {
                if (wom) {
                    ff = initialRights[2];
                    ft = initialRights[castlingType - 1];
                } else {
                    ff = initialRights[5];
                    ft = initialRights[castlingType + 2];
                }
                if (appData.debugMode) {
                    fprintf(debugFP, "Parser FRC (type=%d) %d %d\n", castlingType, ff, ft);
                }
                if (ff == NoRights || ft == NoRights) {
                    return ImpossibleMove;
                }
            }
            if (gate) {
                /* Gating disambiguator is present. */
                if (gate != ff + AAA) {
                    int h = ft;
                    ft = ff;
                    ff = h;
                    /* Reverse for gating at Rook square. */
                    if (gate != AAA + initialRights[castlingType + (wom ? -1 : 2)]) {
                        return ImpossibleMove;
                    }
                }
            }
            sprintf(currentMoveString, "%c%c%c%c%c", ff + AAA, rf + ONE, ft + AAA, rt + ONE, promo);
            if (appData.debugMode) {
                fprintf(debugFP, "(%d-type) castling %d %d\n", castlingType, ff, ft);
            }
            if (**p == ',' && gameInfo.variant == VariantDuck) {
                killX = (*p)[1] - AAA;
                killY = (*p)[2] - ONE;
                sprintf(currentMoveString + 4, ";%c%c", (*p)[1], (*p)[2]);
                *p += 3;
            }

            return (int)LegalityTest(boards[yyboardindex],
             PosFlags(yyboardindex) & ~F_MANDATORY_CAPTURE,  /* [HGM] losers: e.p.! */
             rf, ff, rt, ft, promo);
        } else if (Match("01", p)) {
            /* Prevent this from being mistaken for move number 1. */
            return Nothing;
        }
    }

    /* Variations (nesting). */
    if (**p == '(') {
        if (RdTime(')', p)) {
            return ElapsedTime;
        }
        return Open;
    }
    if (**p == ')') {
        (*p)++;
        return Close;
    }
    if (**p == ';') {
        while (**p != '\n') {
            (*p)++;
        }
        return Comment;
    }

    /* Comments and result messages. */
    *p = oldp;
    commentEnd = NULL;
    result = 0;
    if (**p == '{') {
        if (RdTime('}', p)) {
            return ElapsedTime;
        }
        if (lastChar == '\n' && Match("--------------\n", p)) {
            char * q;
            i = Scan('}', p);
            q = *p - 16;
            if (Match("\n--------------}\n", &q)) {
                return PositionDiagram;
            }
        } else {
            i = Scan('}', p);
        }
        commentEnd = *p;
        if (i) {
            /* Return a comment that runs to EOF immediately. */
            return Comment;
        }
    }
    if (commentEnd) {
        SkipWhite(p);
    }
    if (kifu && **p == '*') {
        /* .kif comment */
        char * q = yytext;
        while (**p && **p != '\n') {
            if (q < yytext + 10 * MSG_SIZ - 3) {
                *q++ = **p;
            }
            (*p)++;
        }
        parseStart = yytext;
        /* Wrap in braces. */
        *yytext = '{';
        strcpy(q, "}\n");
        return Comment;
    }
    if (Match("*", p)) {
        result = GameUnfinished;
    } else if (**p == '0') {
        if (Match("0-1", p) || Match("0/1", p) || Match("0:1", p) || Match("0 - 1", p) || Match("0 / 1", p) || Match("0 : 1", p)) {
            result = BlackWins;
        }
    } else if (**p == '1') {
        if (Match("1-0", p) || Match("1/0", p) || Match("1:0", p) || Match("1 - 0", p) || Match("1 / 0", p) || Match("1 : 0", p)) {
            result = WhiteWins;
        } else if (Match("1/2 - 1/2", p) || Match("1/2:1/2", p) || Match("1/2 : 1/2", p) || Match("1 / 2 - 1 / 2", p) ||
         Match("1 / 2 : 1 / 2", p) || Match("1/2", p) || Match("1 / 2", p)) {
            result = GameIsDrawn;
        }
    }
    if (result) {
        if ((Match(" (", p) && !Scan(')', p)) || (Match(" {", p) && !Scan('}', p))) {
            /* There is a comment after the PGN result. */
            /* So, the comment before it is a normal comment.  We return that first. */
            if (commentEnd) {
                *p = commentEnd;
                return Comment;
            }
        }
        /* This returns a possible preceeding comment as result details. */
        return result;
    }
    if (commentEnd) {
        *p = commentEnd;
        /* There was no PGN result following, so return as normal comment. */
        return Comment;
    }

    /* Move numbers (after castlings or PGN results!) */
    if ((i = Number(p)) != BADNUMBER) {
        /* A single number was read as part of our attempt to read a move. */
        char * numEnd = *p;
        if (**p == '.') {
            (*p)++;
        }
        SkipWhite(p);
        if (**p == '+' || isalpha(**p) || (gameInfo.variant == VariantShogi && *p != numEnd && isdigit(**p))) {
            *p = numEnd;
            return i == 1 ? MoveNumberOne : Nothing;
        }
        *p = numEnd;
        return Nothing;
    }

    /* Non-compliant game-result indicators. */
    if (Match("+-+", p) || Word("stalemate", p)) {
        return GameIsDrawn;
    }
    if (Match("++", p) || Verb("resign", p) || ((Word("check", p) || 1) && Word("mate", p))) {
        return (wom ? BlackWins : WhiteWins);
    }
    c = ToUpper(**p);
    if ((Word("w", p) && (Match("hite", p) || 1)) || (Word("b", p) && (Match("lack", p) || 1))) {
        if (**p != ' ') {
            return Nothing;
        }
        ++*p;
        if (Verb("disconnect", p)) {
            return GameUnfinished;
        }
        if (Verb("resign", p) || Verb("forfeit", p) || Word("mated", p) || Word("lost", p) || Word("loses", p)) {
            return (c == 'W' ? BlackWins : WhiteWins);
        }
        if (Word("mates", p) || Word("wins", p) || Word("won", p)) {
            return (c != 'W' ? BlackWins : WhiteWins);
        }
        return Nothing;
    }
    if (Word("draw", p)) {
        if (**p == 'n') {
            (*p)++;
        }
        if (**p != ' ') {
            return GameIsDrawn;
        }
        oldp = ++*p;
        if (Word("agreed", p)) {
            return GameIsDrawn;
        }
        if (Match("by ", p) && (Word("repetition", p) || Word("agreement", p))) {
            return GameIsDrawn;
        }
        *p = oldp;
        if (*(*p)++ == '(') {
            while (**p != '\n') {
                if (*(*p)++ == ')') {
                    break;
                }
            }
            if ((*p)[-1] == ')') {
                return GameIsDrawn;
            }
        }
        *p = oldp - 1;
        return GameIsDrawn;
    }

    /* Numeric annotation glyph. */
    if (**p == '$') {
        (*p)++;
        if (Number(p) != BADNUMBER) {
            return NAG;
        }
        return Nothing;
    }

    /* By now, we are getting down to the silly stuff. */
    if (Word("gnu", p) || Match("GNU", p)) {
        if (**p == ' ') {
            (*p)++;
        }
        if (Word("chess", p) || Match("CHESS", p)) {
            char * q;
            if ((q = strstr(*p, "game")) || (q = strstr(*p, "GAME")) || (q = strstr(*p, "Game"))) {
                (*p) = q + 4;
                return GNUChessGame;
            }
        }
        return Nothing;
    }
    if (lastChar == '\n' && (Match("# ", p) || Match("; ", p) || Match("% ", p))) {
        while (**p != '\n' && **p != ' ') {
            (*p)++;
        }
        if (**p == ' ' && (Match(" game file", p) || Match(" position file", p))) {
            /* Skip to the end of the line. */
            while (**p != '\n') {
                (*p)++;
            }
            return XBoardGame;
        }
        /* We might need to re-match the skipped stuff. */
        *p = oldp;
    }

    /* Prevent separators from being parsed as a null move. */
    if (Match("---", p)) {
        while (**p == '-') {
            (*p)++;
        }
        return Nothing;
    }
    if (Match("@@@@", p) || Match("--", p) || Match("Z0", p) || Match("pass", p) || Match("null", p)) {
        strncpy(currentMoveString, "@@@@", 5);
        return yyboardindex & F_WHITE_ON_MOVE ? WhiteDrop : BlackDrop;
    }

    /* Efficient skipping of (mostly) alphabetic chatter. */
    while (isdigit(**p) || isalpha(**p) || **p == '-') {
        (*p)++;
    }
    if (*p != oldp) {
        if (**p == '\'') {
            while (isdigit(**p) || isalpha(**p) || **p == '-' || **p == '\'') {
                (*p)++;
            }
            /* random word */
            return Nothing;
        }
        if (lastChar == '\n' && Match(": ", p)) {
            /* mail header, skip indented lines */
            do {
                while (**p != '\n') {
                    (*p)++;
                }
                if (!ReadLine()) {
                    /* append next line if not EOF */
                    return Nothing;
                }
            } while (Match("\n ", p) || Match("\n\t", p));
        }
        return Nothing;
    }

    /* Prevent 00 within unprotected time stamps from being mistaken for castling. */
    if (Match(":00", p)) {
        return Nothing;
    }

    /* Could not match to anything. Return offending character. */
    (*p)++;
    return Nothing;
}

/* Return offset of next pattern in the current file. */
int yyoffset(void) {
    /* Subtract what has been read but has not yet been parsed. */
    return ftell(inputFile) - (inPtr - parsePtr);
}

/* Prepare parse buffer for reading file. */
void yynewfile(FILE * f) {
    inputFile = f;
    inPtr = parsePtr = inputBuf;
    fromString = 0;
    lastChar = '\n';
    /* Make sure we will start by reading a line. */
    *inPtr = NULLCHAR;
}

void yynewstr(char * s) {
    parsePtr = s;
    inputFile = NULL;
    fromString = 1;
}

/* This replaces the flex-generated parser. */
int yylex(void) {
    int result = NextUnit(&parsePtr);
    char * p = parseStart;
    char * q = yytext;
    if (p == yytext) {
        /* Kludge to allow kanji expansion. */
        return result;
    }
    /* Copy the matched text to yytext[]. */
    while (p < parsePtr) {
        *q++ = *p++;
    }
    *q = NULLCHAR;
    lastChar = q[-1];
    return result;
}

/* [HGM] Wrapper for yylex, which treats nesting of parentheses. */
int Myylex(void) {
    int symbol, nestingLevel = 0, i = 0;
    char * p;
    static char buf[256 * MSG_SIZ];
    buf[0] = NULLCHAR;
    do {
        /* Eat away anything not at level 0. */
        symbol = yylex();
        if (symbol == Open) {
            nestingLevel++;
        }
        if (nestingLevel) {
            /* Save all parsed text between (and including) the (). */
            for (p = yytext; *p && i < 256 * MSG_SIZ - 2;) {
                buf[i++] = *p++;
            }
            buf[i] = NULLCHAR;
        }
        if (symbol == 0) {
            /* Encountered EOF. */
            break;
        }
        if (symbol == Close) {
            symbol = Comment, nestingLevel--;
        }
    } while (nestingLevel || symbol == Nothing);
    yy_text = buf[0] ? buf : (char *)yytext;
    return symbol;
}

ChessMove yylexstr(int boardIndex, char * s, char * buf, int buflen) {
    ChessMove ret;
    char * savPP = parsePtr;
    fromString = 1;
    yyboardindex = boardIndex;
    parsePtr = s;
    ret = (ChessMove)Myylex();
    strncpy(buf, yy_text, buflen - 1);
    buf[buflen - 1] = NULLCHAR;
    parsePtr = savPP;
    fromString = 0;
    return ret;
}
