/*
--- randomFunctions.c ---
This contains functions used in @rand.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sysFunctions.h"
#include "../processing/vm.h"
// Generates a random 64 bit long.
int64_t rand64() {
    return ((int64_t)rand() << 45) | 
           ((int64_t)rand() << 30) | 
           ((int64_t)rand() << 15) | 
           ((int64_t)rand());
}
// Implementations for srand, rand, randL, randLD, factorial, gamma, lgamma, permutations, combinations, & array_choose.
void rRandom(auFunc) {
    if (strcmp(identifier, "srand") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        srand(convertNum(arg0->value.numberValue, NUM_LONG).value.lVal);
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "rand") == 0) pushArray(vms->stack,numToTV((num){.type = NUM_INT, .value.iVal = rand()}));
    if (strcmp(identifier, "randL") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);

        int64_t min = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        int64_t max = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
        if (max < min) fatalError(0x30, "min has to be less than max.", -1);
        pushArray(vms->stack,numToTV((num){.type = NUM_LONG, .value.lVal = (rand64() % (max - min + 1)) + min}));
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
    if (strcmp(identifier, "randLD") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);

        long double min = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
        long double max = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;

        pushArray(vms->stack,numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal = min + ((long double)rand() / RAND_MAX) * (max - min)}));
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
    if (strcmp(identifier, "factorial") == 0 || strcmp(identifier, "gamma") == 0 || strcmp(identifier, "lgamma") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (strcmp(identifier, "factorial") == 0 && arg0->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "arg0 has to be a positive integer. Try using the gamma function instead.", -1);
        long double n = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal+1; // Not sure why this needs to have +1 for it to work, probably smth to do w/ conversions, but if it works it works ig.
        if (n < 0 && strcmp(identifier, "factorial") == 0) fatalError(0x30, "arg0 has to be a positive integer. Try using the gamma function instead", -1);
        if (strcmp(identifier, "factorial") == 0) pushArray(vms->stack,numToTV((num){.type = NUM_LONG, .value.lVal = (int64_t)tgammal(n)}));
        if (strcmp(identifier, "gamma") == 0 ) pushArray(vms->stack,numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal = tgammal(n)}));
        if (strcmp(identifier, "lgamma") == 0 ) pushArray(vms->stack,numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal = lgammal(n)}));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "permutations") == 0 || strcmp(identifier, "combinations") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        typedValue* arg2 = getArray(args,  2);
        if (arg2->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        long double n = floorl(convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal);
        long double r = floorl(convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal);
        bool doRepetition = convertNum(arg2->value.numberValue, NUM_BOOL).value.bVal;
        bool isPermut = strcmp(identifier, "permutations") == 0;
        if (isPermut && doRepetition) pushArray(vms->stack,numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = powl((long double)n, (long double)r)}, arg0->value.numberValue.type)));
        if (isPermut && !doRepetition) pushArray(vms->stack,numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = (tgammal(n+1))/(tgammal(1+n-r))}, arg0->value.numberValue.type)));
        if (!isPermut && doRepetition) pushArray(vms->stack,numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = (tgammal(n+r)/(tgammal(r)*tgammal(n)))}, arg0->value.numberValue.type)));
        if (!isPermut && !doRepetition) pushArray(vms->stack,numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = (tgammal(n+1))/(tgammal(r+1)*tgammal(n-r+1))}, arg0->value.numberValue.type)));
        
        freeTypedValue(arg0);
        freeTypedValue(arg1);
        freeTypedValue(arg2);
    }
    if (strcmp(identifier, "array_choose") == 0) {
        typedValue* arg0 = getArray(args,  0);
        if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        int64_t randChoice = (rand64() % (arg0->value.av.len));
        typedValue* toApp = arg0->value.av.data[randChoice];
        free(toApp->ptr);
        toApp->ptr = NULL;
        pushArray(vms->stack,toApp);
        //free(arg0); 
    }
}
