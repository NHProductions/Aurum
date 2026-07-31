/*
--- sysFunctions.c --- 
This file contains several standard library functions, and functions/macros that other libraries use.

Files to check next: mathFunction.c, complexFunctions.c, ioFunction.c, lalgFunctions.h, randomFunctions.c, timeFunctions.c
*/


#define _USE_MATH_DEFINES
#include "Tundora.h"
#include "parser.h"
#include "vm.h"
#include "bytecoder.h"
#include "float.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "winInclude.h"

#include <errno.h>
#include <Windows.h>
#include <direct.h>
#include "num.h"

#include "mathFunctions.h"
#include "timeFunctions.h"
#include "randomFunctions.h"
#include "complexFunctions.h"
#include "ioFunctions.h"
#ifdef _WIN32
    #define getcwd _getcwd
#else 
    #include <unistd.h>
#endif
// Safe realloc (throws an error if it fails).
void* srealloc(void* mem, int size) {
    void* tmp = realloc(mem, size);
    if (tmp == NULL) {
        if (isDebug) printf("srealloc failed!");
        exit(1);
    }
    return tmp;
}
// Converts a number to a typedValue.
typedValue* numToTV(num n) {
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .ptr = NULL,
        .valueType = TYPE_NUM,
        .value.numberValue = n
    };
    return toReturn;
}
// Initializes a new typedValue array.
typedValue* newTVArray(int len, int type) {
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .ptr = NULL,
        .valueType = TYPE_ARRAY,
        .value.av = (arrayValue){
            .arrayType = type,
            .data = malloc(len <= 0 ? 1 : len*sizeof(typedValue*)),
            .len = len
        }
    };
    return toReturn;
}
// Converts a typedValue that's an array of characters into a char*.
char* strArrToChar(typedValue* tv) {
    
    if (tv->valueType != TYPE_ARRAY) {fatalError(0x31, "Invalid string parse", -1);}
    //if (!tv->value.av.isNumArray) {printf("Invalid strArrToChar Type!"); exit(1);}
    if (tv->value.av.len == 0) return strdup("");
    char* toReturn = malloc(tv->value.av.len+1);
    typedValue* endidx = ((typedValue**)tv->value.av.data)[tv->value.av.len-1];
    if (endidx->value.numberValue.value.cVal == '\0') toReturn = srealloc(toReturn, tv->value.av.len);
    toReturn[0] = '\0';
    for (int i = 0; i < tv->value.av.len; i++) {
        typedValue* t = ((typedValue**)tv->value.av.data)[i];
        if (t == NULL) continue;
        if (t->valueType != TYPE_NUM) {fatalError(0x31, "Invalid string parse", -1);}
        struct typedValue** data = (typedValue**)tv->value.av.data;
        num tt = data[i]->value.numberValue;
        if (tt.type != NUM_CHAR && tt.type != NUM_UCHAR) fatalError(0x31, "Invalid string parse", -1);
        int n = strlen(toReturn);
        toReturn[n] = tt.value.cVal;
        if (toReturn[n] != '\0') toReturn[n+1] = '\0';
    }
    return toReturn;

}
// C implementations for scani & scand
void sScanT(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    char* s = strdup("");
    if (arg0->valueType == TYPE_ARRAY) {
        free(s);
        s = strArrToChar(arg0);
    }
    printf("\n%s", s);
    free(s);
    typedValue* toAppend = NULL;
    if (strcmp(identifier, "scand") == 0) {
        long double n = 0;
        scanf("%Lf", &n);
        
        toAppend = numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal = n});
    }
    else {
        int64_t n = 0;
        scanf("%lld", &n);
        toAppend = numToTV((num){.type = NUM_LONG, .value.lVal = n});
    }
    freeTypedValue(arg0);
    pushArray(vms->stack,toAppend);
    return;
}
// C implementation for printInt & printDec
void sPrintNS(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    typedValue* arg1 = (typedValue*)getArray(args,  1);
    typedValue* arg2 = NULL;
    char* s = strdup("");
    if (arg0->valueType == TYPE_ARRAY) {
        free(s);
        s = strArrToChar(arg0);
    }
    if (strcmp(identifier, "printInt") == 0) {
        int64_t lld = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
        printf("\n%s%lld", s, lld);
    }
    else if (strcmp(identifier, "printDec") == 0) {
        arg2 = (typedValue*)getArray(args,  2);
        if (arg2->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);};
        long double ld = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
        printf("\n%s%.*Lf", s, arg2->value.numberValue.value.iVal, ld);
    }
    freeTypedValue(arg0);
    freeTypedValue(arg1);
    free(s);
}
// C implementation for printString().
void sPrintS(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    char* s = strdup("");
    if (arg0->valueType == TYPE_ARRAY) {
        free(s);
        s = strArrToChar(arg0);
    }
    printf("\n%s", s);
    
    freeTypedValue(arg0);
    free(s);
}
// C implementation for strToInt() and strToDec().
void sConvertNum(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {
        fatalError(0x30, "Invalid input into a system function.",-1);
    }
    char* s = strArrToChar(arg0);
    typedValue* toReturn = poolAlloc(globalPool);
    int type = strcmp(identifier, "strToInt") == 0 ? NUM_LONG : NUM_LONGDOUBLE;
    *toReturn = (typedValue){
        .ptr = NULL,
        .valueType = TYPE_NUM,
        .value.numberValue = (num){
            .type = type,
        }
    };
    if (strcmp(identifier, "strToInt") == 0) {
        toReturn->value.numberValue.value.lVal = strtoll(s, NULL, 10);
    }
    else {
        toReturn->value.numberValue.value.ldVal = strtold(s, NULL);
    }
    free(s);
    freeTypedValue(arg0);
    pushArray(vms->stack,toReturn);
}
// C implementation for scaleInt
void sScaleInt(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    typedValue* arg1 = (typedValue*)getArray(args,  1);
    if (arg1->valueType != TYPE_NUM || arg0->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
    
    if (arg1->value.numberValue.type >= NUM_FLOAT || arg0->value.numberValue.type >= NUM_FLOAT) {fatalError(0x30, "Invalid input into a system function.",-1);}
    
    int64_t arg1Val = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
    int actualType = -NUM_FLOAT;
    switch (arg1Val) {
        case 1LL: {actualType = NUM_BOOL; break;}
        case 8LL: {actualType = NUM_CHAR; break;}
        case 16LL: {actualType = NUM_SHORT; break;}
        case 32LL: {actualType = NUM_INT; break;}
        case 64LL: {actualType = NUM_LONG; break;}
        case -1LL: {actualType = NUM_UCHAR; break;}
        case -8LL: {actualType = NUM_UCHAR; break;}
        case -16LL: {actualType = NUM_USHORT; break;}
        case -32LL: {actualType = NUM_UINT; break;}
        case -64LL: {actualType = NUM_ULONG; break;}
    }
    if (actualType == -NUM_FLOAT) {fatalError(0x30, "Invalid input into a system function.",-1);}
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .ptr = NULL, .value.numberValue = (num){.type = NUM_BOOL, .value.bVal = false}, .valueType = TYPE_NUM
    };
    if (actualType == 0) {
        int64_t llval = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        toReturn->value.numberValue = (num){.type = NUM_BOOL, .value.bVal = llval != 0};
    }
    else {
        int64_t llval = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        int64_t maxVal = 0;
        int64_t minVal = 0;
        switch (actualType) {
            case NUM_CHAR: {maxVal = CHAR_MAX; minVal = CHAR_MIN; break;}
            case NUM_SHORT: {maxVal = SHRT_MAX; minVal = SHRT_MIN; break;}
            case NUM_INT: {maxVal = INT_MAX; minVal = INT_MIN; break;}
            case NUM_LONG: {maxVal = LLONG_MAX; minVal = LLONG_MIN; break;}
            case NUM_UCHAR: {maxVal = UCHAR_MAX; minVal = 0; break;}
            case NUM_USHORT: {maxVal = USHRT_MAX; minVal = 0; break;}
            case NUM_UINT: {maxVal = UINT_MAX; minVal = 0; break;}
            case NUM_ULONG: {maxVal = ULLONG_MAX; minVal = 0; break;}
        }
        if (llval < minVal || llval > maxVal) {
            fatalError(0x30, "Invalid input into a system function.",-1);
        }
        switch (actualType) {
            case NUM_CHAR: {toReturn->value.numberValue = (num){.type = actualType, .value.cVal = convertNum(arg0->value.numberValue, NUM_CHAR).value.cVal}; break;}
            case NUM_SHORT: {toReturn->value.numberValue = (num){.type = actualType, .value.sVal = convertNum(arg0->value.numberValue, NUM_SHORT).value.sVal}; break;}
            case NUM_INT: {toReturn->value.numberValue = (num){.type = actualType, .value.iVal = convertNum(arg0->value.numberValue, NUM_INT).value.iVal}; break;}
            case NUM_LONG: {toReturn->value.numberValue = (num){.type = actualType, .value.lVal = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal}; break;}
            case NUM_UCHAR: {toReturn->value.numberValue = (num){.type = actualType, .value.ucVal = convertNum(arg0->value.numberValue, NUM_UCHAR).value.ucVal}; break;}
            case NUM_USHORT: {toReturn->value.numberValue = (num){.type = actualType, .value.usVal = convertNum(arg0->value.numberValue, NUM_USHORT).value.usVal}; break;}
            case NUM_UINT: {toReturn->value.numberValue = (num){.type = actualType, .value.uiVal = convertNum(arg0->value.numberValue, NUM_UINT).value.uiVal}; break;}
            case NUM_ULONG: {toReturn->value.numberValue = (num){.type = actualType, .value.ulVal = convertNum(arg0->value.numberValue, NUM_ULONG).value.ulVal}; break;}
        }
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
}
// C implementation for scaleDec
void sScaleDec(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    typedValue* arg1 = (typedValue*)getArray(args,  1);
    if (arg1->valueType != TYPE_NUM || arg0->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
    
    if (arg1->value.numberValue.type >= NUM_FLOAT) {fatalError(0x30, "Invalid input into a system function.",-1);}
    
    int64_t arg1Val = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
    int actualType = -NUM_FLOAT;
    switch (arg1Val) {
        case 128LL: {actualType = NUM_LONGDOUBLE;break;}
        case 64LL: {actualType = NUM_DOUBLE;break;}
        case 32LL: {actualType = NUM_FLOAT;break;}
    }
    if (actualType == -NUM_FLOAT) {fatalError(0x30, "Invalid input into a system function.",-1);}
    typedValue* toReturn = numToTV((num){.type = NUM_BOOL, .value.bVal = false});
    long double ldval = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double maxVal = 0;
    long double minVal = 0;
    switch (actualType) {
        case NUM_FLOAT: {maxVal = FLT_MAX; minVal = FLT_MIN; break;}
        case NUM_DOUBLE: {maxVal = DBL_MAX; minVal = DBL_MIN; break;}
        case NUM_LONGDOUBLE: {maxVal = LDBL_MAX; minVal = LDBL_MIN; break;}
    }
    if (ldval < minVal || ldval > maxVal) {
        fatalError(0x32, "Invalid Type Conversion",-1);   
    }
    switch (actualType) {
        case NUM_FLOAT: {toReturn->value.numberValue = (num){.type = actualType, .value.fVal = convertNum(arg0->value.numberValue, NUM_FLOAT).value.fVal}; break;}
        case NUM_DOUBLE: {toReturn->value.numberValue = (num){.type = actualType, .value.dVal = convertNum(arg0->value.numberValue, NUM_DOUBLE).value.dVal}; break;}
        case NUM_LONGDOUBLE: {toReturn->value.numberValue = (num){.type = actualType, .value.ldVal = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal}; break;}
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
}
// C implementations for isInf, isNaN, and isNormal
void sFloatBool(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){.ptr = NULL, .value.numberValue = (num){.type = NUM_BOOL, .value.bVal = false}, .valueType = TYPE_NUM};
    if (arg0->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
    long double ldVal = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    if (strcmp(identifier, "isInf") == 0) {
        toReturn->value.numberValue.value.bVal = !isfinite(ldVal);
    }
    else if (strcmp(identifier, "isNaN") == 0) {
        toReturn->value.numberValue.value.bVal = !isnan(ldVal);
    }
    else if (strcmp(identifier, "isNormal") == 0) {
        toReturn->value.numberValue.value.bVal = !isnormal(ldVal);
    }
    freeTypedValue(arg0);
    pushArray(vms->stack,toReturn);
}
// C implementations for toExp() and toSciNo()
void sToExp(auFunc) {
    // arg0 is the number, arg1 is the base, arg2 is if it's a string or not.
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = NULL;
    bool toSciNo = (strcmp(identifier, "toSciNo") == 0);
    if (!toSciNo) {arg1 = getArray(args,  1);}
    if (arg0->valueType != TYPE_NUM || (!toSciNo && arg1->valueType != TYPE_NUM) ) {fatalError(0x30, "Invalid input into a system function.",-1);}
    long double base = toSciNo ? 10.0L : convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double toConvert = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    if (base <= 0 || base == 1) {fatalError(0x33, "toExp()/toSciNo() Input Error: Base cannot be negative or 1.", -1);}
    // converts it into the format of a*base^b = toConvert, where b is the maximum integer power of base, & a is the remaining coefficient.

    long double exp = floorl(logl(fabsl(toConvert) )/logl(base)); // logb(toConvert)
    long double coeff = toConvert/powl(base, exp);

    // coeff*base^exp = toConvert
    char n[500]; n[0] = '\0'; sprintf(n, "%Lg*%Lg^%Lg", coeff, base, exp);
    long double d[3] = {coeff, base, exp};
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .valueType = TYPE_STRUCT,
        .ptr = NULL,
        .value.so = (struct structObject){
            .def = getStructDefViaName("floatConversion", *vms->bc),
            .fields = malloc(sizeof(typedValue)*2)
        }
    };
    typedValue* strResult = newTVArray(strlen(n)+1, AT_CHARARR);
    typedValue* doubResult = newTVArray(3, AT_NUM);
    for (int i = 0; i < 3; i++) {
        ((typedValue**)doubResult->value.av.data)[i] = numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal = d[i]});
    }
    for (int i = 0; i <= strlen(n); i++) {
        ((typedValue**)strResult->value.av.data)[i] = numToTV((num){.type = NUM_CHAR, .value.cVal = i == strlen(n) ? 0 : n[i]});
    }
    ((typedValue**)toReturn->value.so.fields)[0] = doubResult;
    ((typedValue**)toReturn->value.so.fields)[1] = strResult;
    
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    if (arg1 != NULL) freeTypedValue(arg1);
}
// C implementations for array_push, array_append, and array_insertElement
void sPush(auFunc) {
    typedValue* arr = getArray(args,  0);
    typedValue* toPush = getArray(args,  1);
    
    if (arr->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.",-1);}
    typedValue** currentValues = arr->value.av.data;
    arr->value.av.data = malloc((sizeof(typedValue*)*(arr->value.av.len+1)));
    if (strcmp(identifier, "array_push") == 0) {
        ((typedValue**)arr->value.av.data)[0] = deepcopyTypedValue(toPush);
        for (int i = 0; i < arr->value.av.len; i++) {
            ((typedValue**)arr->value.av.data)[i+1] = currentValues[i];
        }
        arr->value.av.len++;
    }
    else if (strcmp(identifier, "array_append") == 0) {
        for (int i = 0; i < arr->value.av.len; i++) {
            ((typedValue**)arr->value.av.data)[i] = currentValues[i];
        }
        ((typedValue**)arr->value.av.data)[arr->value.av.len] = deepcopyTypedValue(toPush);
        arr->value.av.len++;
    }
    else if (strcmp(identifier, "array_insertElement") == 0) {
        typedValue* idx = getArray(args,  2);
        if (idx->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
        int64_t llidx = convertNum(idx->value.numberValue, NUM_LONG).value.lVal;
        arr->value.av.len++;
        for (int i = 0; i < arr->value.av.len; i++) {
            if (i < llidx) {
                ((typedValue**)arr->value.av.data)[i] = currentValues[i];
            }
            else if (i > llidx) {
                ((typedValue**)arr->value.av.data)[i] = currentValues[i-1];
            }
            else if (i == llidx) {
                ((typedValue**)arr->value.av.data)[i] = deepcopyTypedValue(toPush);
            }
        }
        freeTypedValue(idx);
    }

    stackPtr* sp = arr->ptr != NULL ? (stackPtr*)arr->ptr : NULL;
    if (sp != NULL) {
        if (sp->isTV) {
            freeTVArray(((typedValue*)sp->addr)->value.av);
            ((typedValue*)sp->addr)->value.av = arr->value.av;
        }
        else {
            freeTVArray(((stackVariable*)sp->addr)->value->value.av);
            ((stackVariable*)sp->addr)->value->value.av = arr->value.av;
        }
    }
    else {fatalError(0x30, "???", -1);}
    free(currentValues);
    freeTypedValue(toPush);
    
}
// C implementations for array_pop & array_removeElement.
void sPop(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.",-1);}
    if (arg0->value.av.len == 0) {fatalError(0x34, "Cannot use array.removeElement() or array.pop() when its length is zero.",-1);}
    if (arg1->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
    bool returnRemoved = convertNum(arg1->value.numberValue, NUM_BOOL).value.bVal;
    typedValue* toReturn = NULL;
    typedValue** currentValues = arg0->value.av.data;
    arg0->value.av.data = malloc((sizeof(typedValue*)*(arg0->value.av.len-1)));
    int64_t toRemove = 0;
    typedValue* arg2 = NULL;
    if (strcmp(identifier, "array_removeElement") == 0) {
        arg2 = getArray(args,  2);
        if (arg2->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.",-1);}
        toRemove = convertNum(arg2->value.numberValue, NUM_LONG).value.lVal;
    }
    for (int i = 0; i < arg0->value.av.len; i++) {
        if (i < toRemove) {
            ((typedValue**)arg0->value.av.data)[i] = deepcopyTypedValue(currentValues[i]);
        }
        else if (i > toRemove) {
            ((typedValue**)arg0->value.av.data)[i-1] = deepcopyTypedValue(currentValues[i]);
        }
        else {
            if (returnRemoved) toReturn = deepcopyTypedValue(currentValues[i]);
        }
    }
    arg0->value.av.len--;
    stackPtr* sp = (stackPtr*)arg0->ptr;
    if (toReturn != NULL) pushArray(vms->stack,deepcopyTypedValue(toReturn));
    if (sp->isTV) {
        freeTVArray(((typedValue*)sp->addr)->value.av);
        ((typedValue*)sp->addr)->value.av = arg0->value.av;
    }
    else {
        freeTVArray(((stackVariable*)sp->addr)->value->value.av);
        ((stackVariable*)sp->addr)->value->value.av = arg0->value.av;
    }

    if (arg2 != NULL) {freeTypedValue(arg2);}
    freeTVArray((arrayValue){.len = arg0->value.av.len+1, .data = currentValues});
    freeTypedValue(arg1);
}
bool rev = false;
// Compares two numbers (for sorting)
int cmpNumC(const void* av, const void* bv) {
    num* a = (num*)av;
    num* b = (num*)bv;
    if (doBinaryOperation(">", *a, *b).value.bVal) return rev ? 1 : -1;
    if (doBinaryOperation("<", *a, *b).value.bVal) return rev ? -1 : 1;
    return 0;
}
// C implementation for array_sort
void sSort(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    if (arg0->value.av.arrayType != AT_NUM) {fatalError(0x35, "ICannot sort an array that have a nonnumerical element.", -1);}
    if (arg1->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}

    bool r = convertNum(arg1->value.numberValue, NUM_BOOL).value.bVal;
    num* numarr = malloc(sizeof(num)*arg0->value.av.len);
    for (int i = 0; i < arg0->value.av.len; i++) {
        numarr[i] = ((typedValue**)arg0->value.av.data)[i]->value.numberValue;
    }
    rev = r;
    qsort(numarr, arg0->value.av.len, sizeof(num), cmpNumC);
    rev = false;

    typedValue** tv = malloc(arg0->value.av.len*sizeof(typedValue*));
    for (int i = 0; i < arg0->value.av.len; i++) {
        tv[i] = poolAlloc(globalPool);
        *tv[i] = (typedValue){
            .ptr = NULL,
            .valueType = TYPE_NUM,
            .value.numberValue = (num){
                .type = numarr[i].type,
                .value = numarr[i].value
            }
        };
    }

    stackPtr* sp = (stackPtr*)arg0->ptr;
    if (sp->isTV) {
        freeTVArray(((typedValue*)sp->addr)->value.av);
        ((typedValue*)sp->addr)->value.av.data = tv;
    }
    else {
        freeTVArray(((stackVariable*)sp->addr)->value->value.av);
        ((stackVariable*)sp->addr)->value->value.av.data = tv;
    }
    free(numarr);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
}
// C implementation for array_reverse
void sReverse(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}

    typedValue** tv = malloc(arg0->value.av.len*sizeof(typedValue*));
    for (int i = 0; i < arg0->value.av.len; i++) {
        tv[arg0->value.av.len-i-1] = ((typedValue**)arg0->value.av.data)[i];
    }
    
    stackPtr* sp = (stackPtr*)arg0->ptr;
    if (sp->isTV) {
        freeTVArray(((typedValue*)sp->addr)->value.av);
        ((typedValue*)sp->addr)->value.av.data = tv;
    }
    else {
        freeTVArray(((stackVariable*)sp->addr)->value->value.av);
        ((stackVariable*)sp->addr)->value->value.av.data = tv;
    }
    freeTypedValue(arg0);
}
// Converts a char to a string.
char* strFromChar(char n) {
    char* toReturn = malloc(2);
    toReturn[0] = n;
    toReturn[1] = '\0';
    return toReturn;
}
// Converts a typedValue to a string.
char* typedValueToString(typedValue* tv) {
    char* toReturn = malloc(256); // todo: make function to estimate string size of a typedValue.
    toReturn[0] = '\0';
    if (tv->valueType == TYPE_NUM) {
        if (tv->value.numberValue.type == NUM_BOOL) {
            if (tv->value.numberValue.value.bVal) toReturn = srealloc(toReturn, strlen(toReturn)+1+4);
            else toReturn = srealloc(toReturn, strlen(toReturn)+1+5);
            sprintf(toReturn, "%s", tv->value.numberValue.value.bVal ? "true" : "false");
        }
        else {
            char t[256];
            t[0] = '\0';
            sprintf(t, "%Lg", convertNum(tv->value.numberValue, NUM_LONGDOUBLE).value.ldVal);
            
            toReturn = srealloc(toReturn, strlen(toReturn)+1+strlen(t));
            sprintf(toReturn, "%s", t);
        }
        return toReturn;
    }
    else if (tv->valueType == TYPE_ARRAY) {
        toReturn = srealloc(toReturn, 2);
        toReturn[0] = '['; toReturn[1] = '\0';
        for (int i = 0; i < tv->value.av.len; i++) {
            typedValue* tc = ((typedValue**)tv->value.av.data)[i];
            char* tcStr = tv->value.av.arrayType != AT_CHARARR ? typedValueToString(tc) : strFromChar(tc->value.numberValue.value.cVal);

            char* after = (i == tv->value.av.len-1 ? "]" : tv->value.av.arrayType != AT_CHARARR ? ", " : "");
            int tosrealloc = strlen(toReturn)+strlen(tcStr)+strlen(after)+1;

            toReturn = srealloc(toReturn, tosrealloc );
            char n[256];
            n[0] = '\0'; sprintf(n, "%s%s", tcStr, after);
            strcat(toReturn, n);
            free(tcStr); 
        }
        return toReturn;
    }
    else if (tv->valueType == TYPE_STRUCT) {
        bool includeFieldNames = true;
        structDefinition* sd = tv->value.so.def;
        toReturn = srealloc(toReturn, 2);
        toReturn[0] = '{'; toReturn[1] = '\0';
        for (int i = 0; i < sd->fields->length; i++) {
            char* fieldName = ((structField*)getArray(sd->fields, i))->name;
            char n[256]; n[0] = '\0'; 
            char* after = i == sd->fields->length - 1 ? "}" : ", ";
            typedValue* tv2 = ((typedValue**)tv->value.so.fields)[i];
            char* valueConverted = typedValueToString(tv2);
            if (includeFieldNames) sprintf(n, "%s : %s%s", fieldName, valueConverted, after);
            
            int tosrealloc = strlen(toReturn)+strlen(n)+strlen(after)+1;
            toReturn = srealloc(toReturn, tosrealloc);

            strcat(toReturn, n);

            free(valueConverted);
        }
        return toReturn;
    }
    else if (tv->valueType == TYPE_NULL) {fatalError(0x30, "NULL type found", -1);}
    return NULL;
}
// C implementation for newStr().
void sNewStr(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    char* tc = typedValueToString(arg0);
    typedValue* toReturn = newTVArray(strlen(tc)+1, AT_CHARARR);
    for (int i = 0; i <= strlen(tc); i++) {
        ((typedValue**)toReturn->value.av.data)[i] = numToTV((num){.type = NUM_CHAR, .value.cVal = i == strlen(tc) ? '\0' : tc[i]});;
    }
    pushArray(vms->stack,toReturn);
    free(tc);
    freeTypedValue(arg0);
}
// C implementation for string_swapCase(), string_toUpper(), and string_toLower().
void sSCase(auFunc) {
    typedValue* arg0 = (typedValue*)getArray(args,  0);
    typedValue* arg1 = (typedValue*)getArray(args,  1);
    
    if (arg1->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    int64_t amt = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
    if (amt == 0) amt = arg0->value.av.len-1;
    typedValue* toReturn = newTVArray(0, AT_CHARARR);
    for (int i = 0; i < arg0->value.av.len; i++) {
        typedValue* t = ((typedValue**)arg0->value.av.data)[i];
        char currentChar = t->value.numberValue.value.cVal;
        if (i < amt && ( (currentChar >= 'A' && currentChar <= 'Z') || (currentChar >= 'a' && currentChar <= 'z') ) ) {
            if (strcmp(identifier, "string_swapCase") == 0) currentChar = currentChar ^ ' '; // switch cases
            if (strcmp(identifier, "string_toUpper") == 0) currentChar = currentChar >= 'a' && currentChar <= 'z' ? currentChar ^ ' ' : currentChar; // switch cases
            if (strcmp(identifier, "string_toLower") == 0) currentChar = currentChar >= 'A' && currentChar <= 'Z' ? currentChar ^ ' ' : currentChar;
        }
        appendTypedValue(toReturn, numToTV((num){.type = NUM_CHAR, .value.cVal = currentChar}));
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg1);
    freeTypedValue(arg0);
}
// Cycles a char* (e.x abcd -> bcde)
void cycle(char* input, char newChar) {
    int len = strlen(input);

    // abcd -> bcde
    for (int i = 0; i < len-1; i++) {
        input[i] = input[i+1];
    }
    input[len-1] = newChar; 
    input[len] = '\0';
}
// C implementation for string_findOccurances, string_split, string_splitStr, string_eqSplit, & string_nSplit
void sSSplit(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}

    char* n = strArrToChar(arg0);
    typedValue* toReturn = newTVArray(0, strcmp(identifier, "string_findOccurances") == 0 ? AT_NUM : AT_UNKNOWN);
    if (strcmp(identifier, "string_findOccurances") == 0) {
        char* toFind = arg1->valueType == TYPE_NUM ? strFromChar(convertNum(arg1->value.numberValue, NUM_CHAR).value.cVal) : strArrToChar(arg1);
        char* buffer = malloc(strlen(toFind)+1);
        buffer[0] = '\0';
        for (int i = 0; i < strlen(n); i++) {
            if (strlen(toFind) == 1) {buffer[0] = n[i]; buffer[1] = '\0';}
            else if (strlen(buffer) < strlen(toFind)) {
                int nn = strlen(buffer); 
                buffer[nn] = n[i]; 
                buffer[nn+1] = '\0';
            }
            else {cycle(buffer, n[i]);}
            if (strcmp(buffer, toFind) == 0) {
                toReturn->value.av.data = srealloc(toReturn->value.av.data, (toReturn->value.av.len+1)*sizeof(typedValue*)); 
                ((typedValue**)toReturn->value.av.data)[toReturn->value.av.len] = numToTV((num){.type = NUM_INT, .value.iVal = i-strlen(toFind)+1});
                toReturn->value.av.len++;
                
                buffer[0] = '\0';
            }
        }
    }
    else if (strcmp(identifier, "string_split") == 0 || strcmp(identifier, "string_splitStr") == 0) {
        char* substr = strcmp(identifier, "string_split") == 0 ? strFromChar(convertNum(arg1->value.numberValue, NUM_CHAR).value.cVal) : strArrToChar(arg1);
        char currentToken[500];
        currentToken[0] = '\0';
        char* previousChars = malloc(strlen(substr)+1);
        previousChars[0] = '\0';
        int startIdx = 0;
        int lastIdx = 0;
        int occurances = 0;
        for (int i = 0; i < strlen(n); i++) {

            if (strlen(previousChars) < strlen(substr)) {
                sprintf(previousChars, "%s%c", previousChars, n[i]);
            }
            else {
                // push everything over one
                cycle(previousChars, n[i]);
            }    
            if (strcmp(previousChars, substr) != 0) {
                int k = strlen(currentToken);
                currentToken[k] = n[i];
                currentToken[k+1] = '\0';
            }
            else {
                // substring from startIdx::1st char of substr
                int firstCharIdx = i-strlen(substr)+1;
                char* str = malloc(i-strlen(substr)+2-startIdx);
                str[0] = '\0';
                strlen(str);
                for (int j = startIdx; j < firstCharIdx; j++) {
                    int nn = strlen(str);
                    str[nn] = n[j];
                    str[nn+1] = '\0';
                }
                startIdx = i+1;
                typedValue* toAppend = newTVArray(strlen(str)+1, AT_CHARARR);
                for (int j = 0; j <= strlen(str); j++) {
                    ((typedValue**)toAppend->value.av.data)[j] = numToTV((num){.type = NUM_CHAR, .value.cVal = j == strlen(str) ? '\0' : str[j]});
                }
                toReturn->value.av.data = srealloc(toReturn->value.av.data, (toReturn->value.av.len+1)*sizeof(typedValue*) ) ;
                ((typedValue**)toReturn->value.av.data)[occurances] = toAppend;
                (toReturn->value.av.len)++;
                occurances++;
                free(str);
            }
        }
        if (startIdx < strlen(n) ) {
            int len = strlen(n);
            typedValue* toAppend = newTVArray(strlen(n)-startIdx+1, AT_CHARARR);
            for (int j = startIdx; j <= strlen(n); j++) {
                ((typedValue**)toAppend->value.av.data)[j-startIdx] = numToTV((num){.type = NUM_CHAR, .value.cVal = j == strlen(n) ? '\0' : n[j]});
            }
            appendTypedValue(toReturn, toAppend);
        }
        free(previousChars);
        free(substr);
    }
    else if (strcmp(identifier, "string_eqSplit") == 0 || strcmp(identifier, "string_nSplit") == 0) {
        // Splits the string into n equal parts. e.x "hello" -> "he", "ll", "o"
        int amt = strcmp(identifier, "string_nSplit") == 0 ? convertNum(arg1->value.numberValue, NUM_INT).value.iVal : strlen(n)/convertNum(arg1->value.numberValue, NUM_INT).value.iVal;
        char* tok = malloc(amt+1); tok[0] = '\0';
        int l = 0;
        for (int i = 0; i < strlen(n); i++) {
            if (strlen(tok) != amt) {
                int len = strlen(tok);
                tok[len] = n[i];
                tok[len+1] = '\0';
            }
            if (strlen(tok) == amt || i >= strlen(n)-1) {
                
                // convert the token into an array of chars, and append it to toReturn.
                typedValue* toAppend = newTVArray(strlen(tok)+1, AT_CHARARR);
                for (int k = 0; k <= strlen(tok); k++) {
                    typedValue* tokChar = numToTV((num){.type = NUM_CHAR, .value.cVal = k == strlen(tok) ? 0 : tok[k]});
                    ((typedValue**)toAppend->value.av.data)[k] = tokChar;
                }
                toReturn->value.av.data = srealloc(toReturn->value.av.data, sizeof(typedValue*)*(toReturn->value.av.len+1) );
                ((typedValue**)toReturn->value.av.data)[l] = toAppend;
                toReturn->value.av.len++;
                l++;
                
                tok[0] = '\0';
            }
        }
        free(tok);
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
}
// C implementation for string_substring()
void sSSubstring(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    typedValue* arg2 = getArray(args,  2);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    if (arg1->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}
    if (arg2->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}

    char* n = strArrToChar(arg0);
    int start = convertNum(arg1->value.numberValue, NUM_INT).value.iVal;
    int end = convertNum(arg2->value.numberValue, NUM_INT).value.iVal;
    if (end < 0 || start < 0) {fatalError(0x30, "Invalid input into a system function. (arg1 & arg2 have to be >=0, and arg1-arg2 >= 0)", -1);}
    if (end-start < 0) {fatalError(0x30, "Invalid input into a system function. (arg1 & arg2 have to be >=0, and arg1-arg2 >= 0)", -1);}
    typedValue* toReturn = newTVArray(end-start, AT_CHARARR);
    for (int i = start; i <= end; i++) {
        typedValue* toAppend = numToTV((num){.type = NUM_CHAR, .value.cVal = n[i-start]});
        ((typedValue**)toReturn->value.av.data)[i-start] = toAppend;
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
    freeTypedValue(arg2);
}
// C implementations for string_leftPad & string_rightPad.
void sSPad(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* str = strArrToChar(arg0);
    typedValue* arg1 = getArray(args,  1);
    if (arg1->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}
    int amt = convertNum(arg1->value.numberValue, NUM_INT).value.iVal;
    if (amt < 0) {fatalError(0x30, "Invalid input into a system function.", -1);}

    typedValue* arg2 = getArray(args,  2);
    if (arg2->valueType != TYPE_NUM) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char toAdd = convertNum(arg2->value.numberValue, NUM_CHAR).value.cVal;

    
    char* paddedResult = malloc(amt+1);
    paddedResult[0] = '\0';
    for (int i = 0; i < amt; i++) {
        paddedResult[i] = toAdd;
        paddedResult[i+1] = '\0';
    }
    char* toConvertToString = malloc(amt+strlen(str)+1);
    toConvertToString[0] = '\0';
    if (strcmp(identifier, "string_leftPad") == 0 ) sprintf(toConvertToString, "%s%s", paddedResult, str);
    else if (strcmp(identifier, "string_rightPad") == 0) sprintf(toConvertToString, "%s%s", str, paddedResult);
    free(paddedResult);
    free(str);
    typedValue* toReturn = newTVArray(arg0->value.av.len+amt, AT_CHARARR);
    for (int i = 0; i <= strlen(toConvertToString); i++) {
        typedValue* toAppend = numToTV((num){.type = NUM_CHAR, .value.cVal = i == strlen(toConvertToString) ? 0 : toConvertToString[i]});
        ((typedValue**)toReturn->value.av.data)[i] = toAppend;
    }

    pushArray(vms->stack,toReturn);
    free(toConvertToString);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
    freeTypedValue(arg2);
}
// C implementation for string_replace
void sSReplace(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* originalStr = strArrToChar(arg0);

    typedValue* arg1 = getArray(args,  1);
    if (arg1->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* toReplace = strArrToChar(arg1);

    typedValue* arg2 = getArray(args,  2);
    if (arg2->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* toReplaceWith = strArrToChar(arg2);

    int* occurances = malloc(sizeof(int)); occurances[0] = 0;
    int occlen = 0;
    char* previousChars = malloc(strlen(toReplace)+1);
    previousChars[0] = '\0';
    // gather the occurances of the string to be replaced.
    for (int i = 0; i < strlen(originalStr); i++) {
        if (strlen(previousChars) < strlen(toReplace)) {
            int l = strlen(previousChars);
            previousChars[l] = originalStr[i];
            previousChars[l+1] = '\0';
        }
        else {
            cycle(previousChars, originalStr[i]);
        }
        if (strcmp(previousChars, toReplace) == 0) {
            occurances = srealloc(occurances, sizeof(int)*(occlen+1) );
            occurances[occlen] = i-strlen(toReplace)+1;
            previousChars[0] = '\0';
            occlen++;
        }
    }
    
    typedValue* toReturn = newTVArray(0, AT_CHARARR);
    free(previousChars);
    for (int i = 0; i < strlen(originalStr); i++) {
        bool occContains = false;
        for (int j = 0; j < occlen; j++) {
            int k = occurances[j];
            if (k == i) {
                occContains = true; break;
            }
        }
        // If it contains it, then push all chars from to-replace, then change i by the length of the string.
        if (occContains) {
            int j = 0;
            while (j < strlen(toReplaceWith)) {
                typedValue* toAppend = numToTV((num){.type = NUM_CHAR, .value.cVal = toReplaceWith[j]});
                toReturn->value.av.data = srealloc(toReturn->value.av.data, (toReturn->value.av.len+1)*sizeof(typedValue*)); 
                ((typedValue**)toReturn->value.av.data)[toReturn->value.av.len] = toAppend;
                toReturn->value.av.len++;
                j++;
            }
            i += strlen(toReplace)-1; 
            continue;
        }
        else {
            typedValue* toAppend = numToTV((num){.value.cVal = originalStr[i], .type = NUM_CHAR});
            toReturn->value.av.data = srealloc(toReturn->value.av.data, sizeof(typedValue*)*(toReturn->value.av.len+1) );
            ((typedValue**)toReturn->value.av.data)[toReturn->value.av.len] = toAppend;
            toReturn->value.av.len++;
        }
    }
    typedValue* nullTerm = numToTV((num){.type = NUM_CHAR, .value.cVal = 0});
    (toReturn->value.av.data) = srealloc(toReturn->value.av.data, sizeof(typedValue*)*(toReturn->value.av.len+1));
    ((typedValue**)toReturn->value.av.data)[toReturn->value.av.len] = nullTerm;
    toReturn->value.av.len++;

    freeTypedValue(arg0);
    freeTypedValue(arg1);
    freeTypedValue(arg2);
    free(originalStr);
    free(toReplace);
    free(toReplaceWith);
    free(occurances);
    pushArray(vms->stack,toReturn);
}
// Enum of valid format specifiers.
typedef enum {
    // Used characters: acdfilnosux
    // Unused Characters: behjkmpqrtvyz
    F_TERM,
    F_CHAR, // %c
    F_SHORT, // %h
    F_INT, // %i
    F_LONG, // %l
    F_FLOAT, // %f
    F_DOUBLE, // %d
    F_LONGDOUBLE, // %D
    F_STRING, // %s
    F_ARRAY, // %a
    F_STRUCT, // %o
} formatSpecifiers;
typedef struct {
    formatSpecifiers type;
    bool isUnsigned : 1; // %u_; e.x %uc or %un. Doesn't work w/ doubles, floats, strings, or long doubles.
    bool isHex : 1; // %X_; e.x %Xc or %Xn. Doesn't work w/ doubles, floats, strings, or long doubles.
    bool isOctal : 1; // %O_; e.x %Oc or %On. Doesn't work w/ doubles, floats, strings, or long doubles.
    bool isCompact : 1; // %G_; e.x %Gc or %Gn. Only works with doubles, floats, or long doubles. Compresses it to smallest string-representation possible.
} format;
// Given a string, gets all formats inside of it. Returns as an array.
format* getFormats(char* n) {
    format* toReturn = malloc(1);
    int formats = 0;
    for (int i = 0; i < strlen(n); i++) {
        char currentChar = n[i];
        if (currentChar == '%' && i < strlen(n)-1) {
            if (n[i+1] == '%') {i++; continue;};
            format toAppend = (format){
                .isCompact = false,
                .isHex = false,
                .isOctal = false,
                .isUnsigned = false,
                .type = F_TERM
            };
            // get next 3 chars
            char nextChars[4]; nextChars[0] = '\0';
            int nlen = strlen(n);
            for (int j = 0; j < 3 && (i + j+1) < nlen; j++) {
                /* check the next character (i+j+1) for terminators/space/symbol */
                if (n[i+j+1] == '%' || n[i+j+1] == ' ' || isSymbolChar(n[i+j+1])) break;
                nextChars[j] = n[i+j+1];
                nextChars[j+1] = '\0';
            }
            for (int j = 0; j < strlen(nextChars); j++) {
                if (nextChars[j] != 'D' && nextChars[j] != 'd' && nextChars[j] != 'O' && nextChars[j] != 'o') {
                    if (nextChars[j] >= 'A' && nextChars[j] <= 'Z') {nextChars[j] ^= ' ';}
                }
                bool doBreak = false;
                switch (nextChars[j]) {
                    case 'g': {toAppend.isCompact = true; break;}
                    case 'u': {toAppend.isUnsigned = true; break;}
                    case 'x': {toAppend.isHex = true; break;}
                    case 'O': {toAppend.isOctal = true; break;}
                    case 'c': {if (toAppend.type == F_TERM) {toAppend.type = F_CHAR;} else {free(toReturn); return NULL;} break;}
                    case 'i': {if (toAppend.type == F_TERM) {toAppend.type = F_INT;} else {free(toReturn); return NULL;} break;}
                    case 'l': {if (toAppend.type == F_TERM) {toAppend.type = F_LONG;} else {free(toReturn); return NULL;} break;}
                    case 'f': {if (toAppend.type == F_TERM) {toAppend.type = F_FLOAT;} else {free(toReturn); return NULL;} break;}
                    case 'd': {if (toAppend.type == F_TERM) {toAppend.type = F_DOUBLE;} else {free(toReturn); return NULL;} break;}
                    case 'D': {if (toAppend.type == F_TERM) {toAppend.type = F_LONGDOUBLE;} else {free(toReturn); return NULL;} break;}
                    case 's': {if (toAppend.type == F_TERM) {toAppend.type = F_STRING;} else {free(toReturn); return NULL;} break;}
                    case 'a': {if (toAppend.type == F_TERM) {toAppend.type = F_ARRAY;} else {free(toReturn); return NULL;} break;}
                    case 'o': {if (toAppend.type == F_TERM) {toAppend.type = F_STRUCT;} else {free(toReturn); return NULL;} break;}
                    case 'h': {if (toAppend.type == F_TERM) {toAppend.type = F_SHORT;} else {free(toReturn); return NULL;} break;}
                }
            }
            if ((toAppend.isUnsigned) && toAppend.type >= F_FLOAT && toAppend.type <= F_LONGDOUBLE) {fatalError(0x37, "Cannot use %u on a float.", -1);}
            
            toReturn = srealloc(toReturn, sizeof(format)*(formats+1));
            toReturn[formats] = toAppend;
            formats++;
        }
    }
    
    format toAppend = (format){
        .isUnsigned = false,
        .isCompact = false,
        .isHex = false,
        .isOctal = false,
        .type = F_TERM
    };
    toReturn = srealloc(toReturn, sizeof(format)*(formats+1));
    toReturn[formats] = toAppend;
    formats++;
    return toReturn;
}
// Converts a format struct into C format.
char* getCFormat(format n) {
    switch (n.type) {
        case F_CHAR: {
            return strdup("%c");
        }
        case F_SHORT: {
            if (n.isHex) return strdup("%hX");
            if (n.isOctal) return strdup("%ho");
            if (n.isUnsigned) return strdup("%hu");
            return strdup("%%hd");
        }
        case F_INT: {
            if (n.isHex) return strdup("%dX");
            if (n.isOctal) return strdup("%do");
            if (n.isUnsigned) return strdup("%du");
            return strdup("%d");
        }
        case F_LONG: {
            if (n.isHex) return strdup("%%llX");
            if (n.isOctal) return strdup("%%llo");
            if (n.isUnsigned) return strdup("%%llu");
            return strdup("%%lld");
        }
        case F_FLOAT: {
            if (n.isHex) return strdup("%a");
            if (n.isCompact) return strdup("%g");
            return strdup("%%f");
        }
        case F_DOUBLE: {
            if (n.isHex) return strdup("%la");
            if (n.isCompact) return strdup("%lg");
            return strdup("%lf");
        }
        case F_LONGDOUBLE: {
            if (n.isHex) return strdup("%La");
            if (n.isCompact) return strdup("%Lg");
            return strdup("%Lf");
        }
        case F_STRING: {
            return strdup("%s");
        }
    }
    return NULL;
}
// Appends a typedValue to an array.
void appendTypedValue(typedValue* arr, typedValue* toAppend) {
    arr->value.av.data = srealloc(arr->value.av.data, sizeof(typedValue*)*(arr->value.av.len+1));
    ((typedValue**)arr->value.av.data)[arr->value.av.len] = toAppend;
    arr->value.av.len++;
}
// C implementation for scan()
void sScan(auFunc) {
    
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    if (arg0->value.av.arrayType != AT_CHARARR) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* formatStr = strArrToChar(arg0);
    format* formatArr = getFormats(formatStr);
    int formatAmt = 0;
    while (formatArr[formatAmt].type != F_TERM) {
        formatAmt++;
    }

    if (formatAmt != args->length-1) {fatalError(0x38, "Number of parameters passed differs from numbers of parameters suggested in format string.", -1);}
    bool isValidInput = false;
    for (int i = 1; i < args->length; i++) {
        format cf = formatArr[i-1];
        typedValue* argN = getArray(args,  i);
        char* hexC = cf.isHex ? "X" : "";
        char* octalC = cf.isOctal ? "o" : "";
        char* unsignedC = cf.isUnsigned ? "u" : "";
        char* compactC = cf.isCompact ? "g" : "";
        switch (cf.type) {
            case F_INT: {
                int k = 0;
                unsigned int l = 0;
                int* toAssign = cf.isUnsigned ? &l : &k;
                char* printSpecifier = getCFormat(cf);
                int m = scanf(printSpecifier,  &k);
                isValidInput = m == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_UINT : NUM_INT, .value.iVal = *toAssign
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_UINT : NUM_INT, .value.iVal = *toAssign
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_CHAR: {
                char k = 0;
                unsigned char l = 0;
                char* toAssign = cf.isUnsigned ? &l : &k;
                char printSpecifier[50];
                sprintf(printSpecifier, "%%c");
                
                isValidInput = scanf(printSpecifier, cf.isUnsigned ? &l : &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = NUM_CHAR, .value.cVal = *toAssign
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = NUM_CHAR, .value.cVal = *toAssign
                    };
                }

                break;
            }
            case F_SHORT: {
                short k = 0;
                unsigned short l = 0;
                short* toAssign = cf.isUnsigned ? &l : &k;
                char* printSpecifier = getCFormat(cf);
                
                isValidInput = scanf(printSpecifier, cf.isUnsigned ? &l : &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_USHORT : NUM_SHORT, .value.sVal = *toAssign
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_USHORT : NUM_SHORT, .value.sVal = *toAssign
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_LONG: {
                int64_t k = 0;
                uint64_t l = 0;
                int64_t* toAssign = cf.isUnsigned ? &l : &k;
                char* printSpecifier = getCFormat(cf);
                
                isValidInput = scanf(printSpecifier, cf.isUnsigned ? &l : &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_ULONG : NUM_LONG, .value.lVal = *toAssign
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = cf.isUnsigned ? NUM_ULONG : NUM_LONG, .value.lVal = *toAssign
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_FLOAT: {
                float k = 0;
                char* printSpecifier = getCFormat(cf);
                
                isValidInput = scanf(printSpecifier, &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = NUM_FLOAT, .value.fVal = k
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = NUM_FLOAT, .value.fVal = k
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_DOUBLE: {
                double k = 0;
                char* printSpecifier = getCFormat(cf);
                
                isValidInput = scanf(printSpecifier, &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = NUM_DOUBLE, .value.dVal = k
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = NUM_DOUBLE, .value.dVal = k
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_LONGDOUBLE: {
                long double k = 0;
                char* printSpecifier = getCFormat(cf);
                
                isValidInput = scanf(printSpecifier, &k) == 1 ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    stackaddr->value.numberValue = (num){
                        .type = NUM_LONGDOUBLE, .value.ldVal = k
                    };
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    stackaddr->value->value.numberValue = (num){
                        .type = NUM_LONGDOUBLE, .value.ldVal = k
                    };
                }
                free(printSpecifier);
                break;
            }
            case F_STRING: {
                char k[500]; k[0] = '\0';
                char printSpecifier[50];
                sprintf(printSpecifier, "%%s");
                
                isValidInput = fgets(k, sizeof(k), stdin) != NULL ? true : false;
                stackPtr* sp = (stackPtr*)argN->ptr;
                arrayValue av = (arrayValue){
                    .arrayType = AT_CHARARR,
                    .data = malloc(sizeof(typedValue*)*(strlen(k)+1)),
                    .len = strlen(k)+1
                };
                for (int i = 0; i <= strlen(k); i++) {
                    typedValue* toApp = poolAlloc(globalPool);
                    *toApp = (typedValue){
                        .ptr = NULL,
                        .value.numberValue = (num){.type = NUM_CHAR, .value.cVal = i == strlen(k) ? 0 : k[i]},
                        .valueType = TYPE_NUM    
                    };
                    av.data[i] = toApp;
                }
                if (sp->isTV) {
                    typedValue* stackaddr = (typedValue*)sp->addr; // TODO: make this memory-safe
                    freeTVArray(stackaddr->value.av);
                    stackaddr->value.av = av;
                }
                else {
                    stackVariable* stackaddr = (stackVariable*)sp->addr;
                    freeTVArray(stackaddr->value->value.av);
                    stackaddr->value->value.av = av;
                }

                break;
            }
        }
    }
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .valueType = TYPE_NUM,
        .value.numberValue = (num){
            .type = NUM_BOOL,
            .value.bVal = (bool)isValidInput
        },
        .ptr = NULL
    };
    pushArray(vms->stack,toReturn);
    free(formatArr);
    free(formatStr);
    for (int i = 0; i < args->length; i++) {
        freeTypedValue(getArray(args,  i));
    }
}
// Returns if a character is alphabetical
bool isAlphabetical(char n) {
    if (n >= 'A' && n <= 'Z') return true;
    if (n >= 'a' && n <= 'z') return true;
    return false;
}
// C implementation for print() & formats().
char* sPrint(auFunc, bool returnResult) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) {fatalError(0x30, "Invalid input into a system function.", -1);}
    char* str = strArrToChar(arg0);
    
    char* toPrint = malloc(1);
    toPrint[0] = '\0';
    int formatNum = 0;
    for (int i = 0; i < strlen(str); i++) {
        char currentChar = str[i];
        if (str[i] == '%' && i < strlen(str)-1) {
            if (str[i+1] == '%') {
                toPrint = srealloc(toPrint, strlen(toPrint)+2);
                int n = strlen(toPrint);
                toPrint[n] = '%';
                toPrint[n+1] = '\0';
                i++;
                continue;
            }
            char nextChars[4]; nextChars[0] = '\0';
            bool previousValidFound = false;
            for (int j = 0; j < 3 && j+i < strlen(str); j++) {
                char current = str[i+j];
                char temp[10]; sprintf(temp, "%s%c", nextChars, current);
                format* n = getFormats(temp);
                if (n == NULL && previousValidFound) break;
                else if (n != NULL) {previousValidFound = true;};
                if ( (str[i+j] == '%' && j > 0) || str[i+j] == ' ' || (!isAlphabetical(str[i+j]) && j > 0) ) break;
                nextChars[j] = str[i+j];
                nextChars[j+1] = '\0';
                
                
            }
            format* f = getFormats(nextChars);
            format cf = f[0];
            typedValue* argN = getArray(args,  formatNum+1);
            formatNum++;
            if (cf.type == F_TERM) {
                free(f);
                continue;
            }
            switch (cf.type) {
                case F_CHAR: {
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+1);
                    int len = strlen(toPrint);
                    toPrint[len] = argN->value.numberValue.value.cVal;
                    toPrint[len+1] = '\0';
                    break;
                }
                case F_SHORT: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isHex) sprintf(printSpecifier, "%%hX");
                    else if (cf.isOctal) sprintf(printSpecifier, "%%ho");
                    else if (cf.isUnsigned) sprintf(printSpecifier, "%%hu");
                    else sprintf(printSpecifier, "%%hd");
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.sVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
                case F_INT: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isHex) sprintf(printSpecifier, "%%X");
                    else if (cf.isOctal) sprintf(printSpecifier, "%%o");
                    else if (cf.isUnsigned) sprintf(printSpecifier, "%%u");
                    else sprintf(printSpecifier, "%%d");
                    
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.iVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
                case F_LONG: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isHex) sprintf(printSpecifier, "%%llX");
                    else if (cf.isOctal) sprintf(printSpecifier, "%%llo");
                    else if (cf.isUnsigned) sprintf(printSpecifier, "%%llu");
                    else sprintf(printSpecifier, "%%lld");
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.lVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
                case F_STRING: {
                    char* n = strArrToChar(argN);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    free(n);
                    break;
                }
                case F_ARRAY: {
                    char* n = typedValueToString(argN);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    free(n);
                    break;
                }
                case F_STRUCT: {
                    char* n = typedValueToString(argN);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    free(n);
                    break;
                }
                case F_FLOAT: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isCompact) sprintf(printSpecifier, "%%g");
                    else sprintf(printSpecifier, "%%f");
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.fVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
                case F_DOUBLE: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isCompact) sprintf(printSpecifier, "%%lg");
                    else sprintf(printSpecifier, "%%lf");
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.dVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
                case F_LONGDOUBLE: {
                    char printSpecifier[50];
                    printSpecifier[0] = '\0';
                    if (cf.isCompact) sprintf(printSpecifier, "%%Lg");
                    else sprintf(printSpecifier, "%%Lf");
                    char n[50]; n[0] = '\0'; snprintf(n, sizeof(n), printSpecifier, argN->value.numberValue.value.ldVal);
                    toPrint = srealloc(toPrint, strlen(toPrint)+1+strlen(n));
                    strcat(toPrint, n);
                    break;
                }
            }
            i += strlen(nextChars)-1;
            free(f);
        }
        else {
            toPrint = srealloc(toPrint, strlen(toPrint)+2);
            int n = strlen(toPrint);
            toPrint[n] = str[i];
            toPrint[n+1] = '\0';
        }
    }
    if (!returnResult && strcmp(identifier, "print") == 0 ) printf("%s", toPrint);
    else if (strcmp(identifier, "formats") == 0) {
        typedValue* toReturn = newTVArray(strlen(toPrint)+1, AT_CHARARR);
        for (int i = 0; i <= strlen(toPrint); i++) {
            toReturn->value.av.data[i] = numToTV((num){.type = NUM_CHAR, .value.cVal = i == strlen(toPrint) ? 0 : toPrint[i]});;
        }
        pushArray(vms->stack,toReturn);
    }
    
    if (!returnResult) free(toPrint);
    free(str); 
    for (int i = 0; i < args->length; i++) {
        freeTypedValue(getArray(args,  i));
    }
    if (returnResult) return toPrint;
}
// C implementation for throw().
void sThrow(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    int code = 0;
    char* message = "";
    int line = -1;
    if (arg0->valueType == TYPE_STRUCT) {
        if (strcmp(arg0->value.so.def->name, "exception") != 0) fatalError(0x30, "", -1);
        code = convertNum(arg0->value.so.fields[0]->value.numberValue, NUM_INT).value.iVal;
        message = strArrToChar(arg0->value.so.fields[1]);
        line = convertNum(arg0->value.so.fields[2]->value.numberValue, NUM_INT).value.iVal;
    }
    else if (arg0->valueType == TYPE_NUM) {
        code = convertNum(arg0->value.numberValue, NUM_INT).value.iVal;
        message = "";
        line = -1;
    }
    else if (arg0->valueType == TYPE_ARRAY) {
        message = strArrToChar(arg0);
        code = -1;
        line = -1;
    }
    fatalError(code, message, line);
    freeTypedValue(arg0);
}
// C implementation for exit().
void sExit(auFunc) {
    exit(0);
}
// C implementation for !opFunc(), which is used for floor division (//) & exponentiation (**).
void opFunc(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    typedValue* arg2 = getArray(args,  2);

    int toCall = convertNum(arg2->value.numberValue, NUM_INT).value.iVal;
    typedValue* toReturn = poolAlloc(globalPool);
    if (toCall == 1 && (arg0->value.numberValue.type < NUM_FLOAT || arg1->value.numberValue.type < NUM_FLOAT) ) fatalError(0x30, "Floor division only works with floats", -1);
    switch (toCall) {
        case 1: {
            long double ld0 = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
            long double ld1 = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
            num ld = (num){.type = NUM_LONGDOUBLE, .value.ldVal=floorl(ld0/ld1)};
            *toReturn = (typedValue){
                .ptr = NULL,
                .valueType = TYPE_NUM,
                .value.numberValue = convertNum(ld, arg1->value.numberValue.type)
            };
            break;
        }
        case 0: {
            long double ld0 = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
            long double ld1 = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
            if (floorl(ld1) != ld1 && ld0 < 0) fatalError(0x17, "", -1);
            num ld = (num){.type = NUM_LONGDOUBLE, .value.ldVal=powl(ld0, ld1)};
            *toReturn = (typedValue){
                .ptr = NULL,
                .valueType = TYPE_NUM,
                .value.numberValue = convertNum(ld, arg0->value.numberValue.type)
            };
            break;
        }
    }
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
    freeTypedValue(arg2);
}
char* getTypeOf(typedValue* tv, bool sizeFormat) {
    if (tv->valueType == TYPE_NUM) {
        switch (tv->value.numberValue.type) {
            case (NUM_BOOL): return strdup("bool");
            case (NUM_CHAR): return sizeFormat ? strdup("int8") : strdup("char");
            case (NUM_SHORT): return sizeFormat ? strdup("int16") : strdup("short");
            case (NUM_INT): return sizeFormat ? strdup("int32") : strdup("int");
            case (NUM_LONG): return sizeFormat ? strdup("int64") : strdup("long");
            case (NUM_UCHAR): return sizeFormat ? strdup("uint8") : strdup("uchar");
            case (NUM_USHORT): return sizeFormat ? strdup("uint16") : strdup("ushort");
            case (NUM_UINT): return sizeFormat ? strdup("uint32") : strdup("uint");
            case (NUM_ULONG): return sizeFormat ? strdup("uint64") : strdup("ulong");
            case (NUM_FLOAT): return sizeFormat ? strdup("double32") : strdup("float");
            case (NUM_DOUBLE): return sizeFormat ? strdup("double64") : strdup("double");
            case (NUM_LONGDOUBLE): return sizeFormat ? strdup("double128") : strdup("longdouble");
        }
    }
    else if (tv->valueType == TYPE_ARRAY) {
        if (tv->value.av.arrayType == AT_CHARARR) return strdup("string");
        // get first element's type and return that & [].
        if (tv->value.av.len == 0) return strdup("array");
        char* firstIdx = getTypeOf(tv->value.av.data[0], sizeFormat);
        char* toReturn = malloc(strlen(firstIdx)+1+2);
        sprintf(toReturn, "%s[]", firstIdx);
        free(firstIdx);
        return toReturn;
    } 
    else if (tv->valueType == TYPE_STRUCT) {
        return strdup(tv->value.so.def->name);
    }
}
void stypeOf(auFunc) {
    typedValue* arg0 = getArray(args, 0);
    bool sizeFormat = false;
    typedValue* arg1 = NULL;
    if (args->length >= 2) {
        arg1 = getArray(args, 1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (arg1->value.numberValue.type != NUM_BOOL) fatalError(0x30, "", -1);
        sizeFormat = arg1->value.numberValue.value.bVal;
    }
    char* toReturn = getTypeOf(arg0, sizeFormat);
    typedValue* tvToReturn = newTVArray(0, AT_CHARARR);
    for (int i = 0; i < strlen(toReturn); i++) {
        appendTypedValue(tvToReturn, numToTV((num){.type = NUM_CHAR, .value.cVal = toReturn[i]}));
    }
    pushArray(vms->stack, tvToReturn);
    freeTypedValue(arg0);
    if (arg1 != NULL) freeTypedValue(arg1);
}

bool isInFunctionRange(char* n, const bcFunction* bcf, char* start, char* end) {
    bool startFound = false;
    bool endFound = false;
    int idx = 0;
    while (startFound == false || endFound == false) {
        bcFunction bcfi = bcf[idx];
        if (strcmp(bcfi.name, start) == 0 ) startFound = true;
        if (strcmp(bcfi.name, end) == 0) endFound = true;
        if (strcmp(bcfi.name, n) == 0 && startFound) {
            return startFound && (!endFound || strcmp(n, end) == 0);
        }
        if (startFound && endFound) return false;
        idx++;
    }
    return false;
}
void systemCall(bcFunction* bcDef, virtualMachineState* vms, int argc) {
    char* identifier = bcDef->name;
    array* args = mallocArray(0);
    for (int i = 0; i < argc; i++) {
        insertArray(args, 0, popArray(vms->stack));
    }
    // Default import
    if (strcmp(identifier, "scani") == 0 || strcmp(identifier, "scand") == 0) sScanT(auFuncCall);
    else if (strcmp(identifier, "printInt") == 0 || strcmp(identifier, "printDec") == 0) sPrintNS(auFuncCall);
    else if (strcmp(identifier, "printString") == 0) sPrintS(auFuncCall);
    else if (strcmp(identifier, "strToInt") == 0 || strcmp(identifier, "strToDec") == 0) sConvertNum(auFuncCall);
    else if (strcmp(identifier, "scaleInt") == 0) sScaleInt(auFuncCall);
    else if (strcmp(identifier, "scaleDec") == 0) sScaleDec(auFuncCall);
    else if (strcmp(identifier, "isNaN") == 0 || strcmp(identifier, "isInf") == 0 || strcmp(identifier, "isNormal") == 0) sFloatBool(auFuncCall);
    else if (strcmp(identifier, "toExp") == 0 || strcmp(identifier, "toSciNo") == 0) sToExp(auFuncCall);
    else if (strcmp(identifier, "array_push") == 0 || strcmp(identifier, "array_append") == 0 || strcmp(identifier, "array_insertElement") == 0) sPush(auFuncCall);
    else if (strcmp(identifier, "array_pop") == 0 || strcmp(identifier, "array_removeElement") == 0) sPop(auFuncCall);
    else if (strcmp(identifier, "array_sort") == 0) sSort(auFuncCall);
    else if (strcmp(identifier, "array_reverse") == 0) sReverse(auFuncCall);
    else if (strcmp(identifier, "newStr") == 0) sNewStr(auFuncCall);
    else if (strcmp(identifier, "string_toUpper") == 0 || strcmp(identifier, "string_toLower") == 0 || strcmp(identifier, "string_swapCase") == 0) sSCase(auFuncCall); 
    else if (strcmp(identifier, "string_split") == 0 || strcmp(identifier, "string_findOccurances") == 0 || strcmp(identifier, "string_nSplit") == 0 || strcmp(identifier, "string_eqSplit") == 0 || strcmp(identifier, "string_splitStr") == 0) sSSplit(auFuncCall);
    else if (strcmp(identifier, "string_substr") == 0) sSSubstring(auFuncCall);
    else if (strcmp(identifier, "string_leftPad") == 0 || strcmp(identifier, "string_rightPad") == 0) sSPad(auFuncCall);
    else if (strcmp(identifier, "string_replace") == 0) sSReplace(auFuncCall);
    else if (strcmp(identifier, "scan") == 0) sScan(auFuncCall);
    else if (strcmp(identifier, "print") == 0 || strcmp(identifier, "formats") == 0) sPrint(auFuncCall, false);
    else if (strcmp(identifier, "throw") == 0) sThrow(auFuncCall);
    else if (strcmp(identifier, "exit") == 0) sExit(auFuncCall);
    else if (strcmp(identifier, "!opFunc") == 0) opFunc(auFuncCall);
    else if (strcmp(identifier, "typeof") == 0) stypeOf(auFuncCall);
    // @math
    else if (strcmp(identifier, "ln") == 0 || strcmp(identifier, "log2") == 0 || strcmp(identifier, "log10") == 0 || strcmp(identifier, "log") == 0) mLog(auFuncCall);
    else if (strcmp(identifier, "hypot") == 0 || strcmp(identifier, "sidel") == 0) mHypot(auFuncCall);
    else if (strcmp(identifier, "dToR") == 0 || strcmp(identifier, "rToD") == 0) mDegreeConversion(auFuncCall);
    else if (isInFunctionRange(identifier, mathFunctions, "sin", "acschd")) mTrig(auFuncCall);
    else if (isInFunctionRange(identifier, mathFunctions, "decToBase", "b64ToDec")) mBases(auFuncCall);
    else if (isInFunctionRange(identifier, mathFunctions, "ceil", "abs")) mDecFuncs(auFuncCall);
    else if (isInFunctionRange(identifier, mathFunctions, "gcd", "array_prod")) mNumberFunctions(auFuncCall);
    else if (strcmp(identifier, "array_mean") == 0 || strcmp(identifier, "array_median") == 0 || strcmp(identifier, "array_quantiles") == 0) mListOp(auFuncCall);
    // @time
    else if (strcmp(identifier, "time") == 0 || strcmp(identifier, "clock") == 0 || strcmp(identifier, "getUTC") == 0 || strcmp(identifier, "toUTC") == 0 || strcmp(identifier, "sleep") == 0) tTime(auFuncCall);
    // @rand
    else if (isInFunctionRange(identifier, randomFunctions, "srand", "array_choose")) rRandom(auFuncCall);
    // @cplx
    else if (isInFunctionRange(identifier, cplxFunctions, "cexp", "clog")) cExponentialFuncs(auFuncCall);
    else if (isInFunctionRange(identifier, cplxFunctions, "csin", "cacoth")) cTrig(auFuncCall);
    // @io
    else if (strcmp(identifier, "fOpen") == 0) fNewFile(auFuncCall);
    else if (strcmp(identifier, "file_close") == 0) fClose(auFuncCall);
    else if (strcmp(identifier, "file_getChar") == 0 || strcmp(identifier, "file_getStr") == 0 || strcmp(identifier, "file_getLine") == 0 || strcmp(identifier, "file_read") == 0) fRead(auFuncCall);
    else if (strcmp(identifier, "file_writef") == 0 || strcmp(identifier, "file_putChar") == 0 || strcmp(identifier, "file_putStr") == 0) fWrite(auFuncCall);
    else if (strcmp(identifier, "file_rename") == 0 || strcmp(identifier, "file_delete") == 0) fModify(auFuncCall);
    else if (strcmp(identifier, "file_size") == 0) fSize(auFuncCall);
    else if (isInFunctionRange(identifier, ioFunctions, "createDirectory", "getFiles")) fDirectory(auFuncCall);
}