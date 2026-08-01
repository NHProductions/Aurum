/*
--- complexFunctions.c ---
This file contains implementation for @cplx functions.
*/


#include <complex.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complexFunctions.h"

// Converts a typedValue to a complex long double.
long double complex tvToCLD(typedValue* tv) {
    if (tv->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp(tv->value.so.def->name, "cplxNum") != 0) fatalError(0x30, "", -1);
    long double a = convertNum(tv->value.so.fields[0]->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double b = convertNum(tv->value.so.fields[1]->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    return a+b*I;
}
// same as tvToCLD() but the other way around.
typedValue* cldToTV(long double complex cld, virtualMachineState* vms) {
    typedValue* a = numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal=creall(cld)});
    typedValue* bi = numToTV((num){.type = NUM_LONGDOUBLE, .value.ldVal=cimagl(cld)});
    typedValue* structToReturn = poolAlloc(globalPool);
    stackPtr* aptr = malloc(sizeof(stackPtr)); stackPtr* bptr = malloc(sizeof(stackPtr));
    *aptr = *bptr = (stackPtr){
        .addr = structToReturn,
        .idx = -1,
        .isTV = true
    };
    a->ptr = aptr; bi->ptr = bptr;
    *structToReturn = (typedValue){
        .ptr = NULL,
        .value.so = (struct structObject){
            .def = getStructDefViaName("cplxNum", *vms->bc),
            .fields = malloc(sizeof(typedValue*)*2)
        },
        .valueType = TYPE_STRUCT
    };
    structToReturn->value.so.fields[0] = a;
    structToReturn->value.so.fields[1] = bi;
    return structToReturn;
}
// C implementation for cexp(), ctxp(), cln(), clog2(), clog10(), cpow(), & clog().
void cExponentialFuncs(auFunc) {

    typedValue* arg0 = getArray(args, 0);
    long double complex cld0 = tvToCLD(arg0);
    long double complex result = 0+0*I;
    if (strcmp(identifier, "cexp") == 0) {
        result = cexpl(cld0);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "ctxp") == 0) {
        result = cpowl(2+0*I, cld0);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "cln") == 0) {
        result = clogl(cld0);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "clog2") == 0) {
        result = clogl(cld0)/clogl(2+0*I);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "clog10") == 0) {
        result = clogl(cld0)/clogl(10+0*I);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
    }
    if (strcmp(identifier, "cpow") == 0) {
        typedValue* arg1 = getArray(args, 1);
        long double complex cld1 = tvToCLD(arg1);
        result = cpowl(cld0, cld1);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
    if (strcmp(identifier, "clog") == 0) {
        typedValue* arg1 = getArray(args, 1);
        long double complex cld1 = tvToCLD(arg1);
        result = clogl(cld0)/clogl(cld1);
        pushArray(vms->stack, cldToTV(result, vms));
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
}
// C implementation for complex trig functions
void cTrig(auFunc) {
    typedValue* arg0 = getArray(args, 0);
    long double complex n = tvToCLD(arg0);
    long double complex result = 0+0*I;
    char* idx = identifier;
    if (strcmp(idx, "csin") == 0) {
        result = csinl(n);
    }
    else if (strcmp(idx, "ccos") == 0) {
        result = ccosl(n);
    }
    else if (strcmp(idx, "ctan") == 0) {
        if (ccosl(n) == 0.0L) fatalError(0x30, "ctan is undefined where ccos(n)=0+0i", -1);
        result = ctanl(n);
    }
    else if (strcmp(idx, "ccot") == 0) {
        if (csinl(n) == 0.0L) fatalError(0x30, "ccot is undefined where csin(n)=0+0i", -1);
        result = 1.0L / ctanl(n);
    }
    else if (strcmp(idx, "csec") == 0) {
        if (ccosl(n) == 0.0L) fatalError(0x30, "csec is undefined where ccos(n)=0+0i", -1);
        result = 1.0L / ccosl(n);
    }
    else if (strcmp(idx, "ccsc") == 0) {
        if (csinl(n) == 0.0L) fatalError(0x30, "ccsc is undefined where ccsc(n)=0+0i", -1);
        result = 1.0L / csinl(n);
    }
    else if (strcmp(idx, "casin") == 0) {
        result = casinl(n);
    }
    else if (strcmp(idx, "cacos") == 0) {
        result = cacosl(n);
    }
    else if (strcmp(idx, "catan") == 0) {
        result = catanl(n);
    }
    else if (strcmp(idx, "cacot") == 0) {
        if (n == 0.0L) fatalError(0x30, "cacot(0) is undefined", -1);
        result = (0.5L * I) * clogl((n - I) / (n + I));
    }
    else if (strcmp(idx, "casec") == 0) {
        if (n == 0.0L) fatalError(0x30, "casec(0) is undefined.", -1);
        result = cacosl(1.0L / n);
    }
    else if (strcmp(idx, "cacsc") == 0) {
        if (n == 0.0L) fatalError(0x30, "cacsc(0) is undefined.", -1);
        result = casinl(1.0L / n);
    }
    else if (strcmp(idx, "csinh") == 0) {
        result = csinhl(n);
    }
    else if (strcmp(idx, "ccosh") == 0) {
        result = ccoshl(n);
    }
    else if (strcmp(idx, "ctanh") == 0) {
        result = ctanhl(n);
    }
    else if (strcmp(idx, "ccoth") == 0) {
        if (csinhl(n) == 0.0L) fatalError(0x30, "ccoth is undefined where csinh(n)=0+0i", -1);
        result = 1.0L / ctanhl(n);
    }
    else if (strcmp(idx, "csech") == 0) {
        if (ccoshl(n) == 0.0L)
            fatalError(0x30, "csech is undefined where cosh(n) = 0+0i", -1);
        result = 1.0L / ccoshl(n);
    }
    else if (strcmp(idx, "ccsch") == 0) {
        if (csinhl(n) == 0.0L) fatalError(0x30, "ccsch is undefined where sinh(z) = 0+0i", -1);
        result = 1.0L / csinhl(n);
    }
    else if (strcmp(idx, "casinh") == 0) {
        result = casinhl(n);
    }
    else if (strcmp(idx, "cacosh") == 0) {
        result = cacoshl(n);
    }
    else if (strcmp(idx, "catanh") == 0) {
        if (n == 1.0L || n == -1.0L) fatalError(0x30, "catanh is undefined at +1 & -1", -1);
        result = catanhl(n);
    }
    else if (strcmp(idx, "cacoth") == 0) {
        if (n == 0.0L) fatalError(0x30, "cacoth(0) is undefined.", -1);
        if (n == 1.0L || n == -1.0L) fatalError(0x30, "cacoth is undefined at +1 & -1", -1);
        result = catanhl(1.0L / n);
    }
    else if (strcmp(idx, "casech") == 0) {
        if (n == 0.0L) fatalError(0x30, "casech(0) is undefined.", -1);
        result = cacoshl(1.0L / n);
    }
    else if (strcmp(idx, "cacsch") == 0) {
        if (n == 0.0L) fatalError(0x30, "acsch(0) is undefined.", -1);
        result = casinhl(1.0L / n);
    }

    pushArray(vms->stack, cldToTV(result, vms));
    freeTypedValue(arg0);
}