#ifndef NUM_H
#define NUM_H
#include <stdint.h>
#include "lexer.h"
typedef enum {
    NUM_BOOL=0,
    NUM_CHAR=1,
    NUM_UCHAR=-1,
    NUM_SHORT=2,
    NUM_USHORT=-2,
    NUM_INT=3,
    NUM_UINT=-3,
    NUM_LONG=4,
    NUM_ULONG=-4,
    NUM_FLOAT=5,
    NUM_DOUBLE=6,
    NUM_LONGDOUBLE=7
} numberTypes;
typedef struct {
    union {
        char cVal;
        unsigned char ucVal;
        short sVal;
        unsigned short usVal;
        int iVal;
        unsigned int uiVal;
        int64_t lVal;
        uint64_t ulVal;
        float fVal;
        double dVal;
        long double ldVal;
        bool bVal;
    } value;
    numberTypes type;
} num;
typedef struct {
    num a;
    num b;
} pair;
void sprintfNum(num n, char* t, bool includeType);
pair convertNums(num a, num b);
num doBinaryOperation(char* op, num a, num b);
num doUnaryOperation(char* op, num a);
num convertNum(num a, numberTypes n);
num strToNum(token t);
int cmpNum(num a, num b);
#endif