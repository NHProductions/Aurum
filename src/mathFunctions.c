
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include "sysFunctions.h"
#include "mathFunctions.h"
#include "bytecoder.h"
#include "Tundora.h"
#include "parser.h"
#include "vm.h"
#include "bytecoder.h"
#include "num.h"
#include "winInclude.h"

char* defaultCharset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/\\!@#$%^&*_-=;:'\"<>,?'€ƒ†‡Š‹ŒÆŽš›œžŸ¡¢£¤¥¦§©ÇÐÑ×ØÞßæçð÷øþäëïöü±µ";
char* b64Charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
// Converts a base-10 long double n to another base. Uses defaultCharset or b64Charset.
char* convertBase(long double n, int base, char* charset) {
    char* chars = "";
    if (charset != NULL) chars = charset;
    else if (base != 64) chars = defaultCharset;
    else sprintf(chars, "%s", b64Charset);
    
    char toReturn[256]; toReturn[0] = '\0';
    long double remainder = n;
    int index = 0;
    int64_t nn = (int64_t)remainder;
    while (remainder > 0) {
        int64_t charidx = (int64_t)remainder % (int64_t)base;
        toReturn[index] = chars[charidx];
        remainder = floorl(remainder/base);
        index++;
    }
    toReturn[index] = '\0';
    char* t = malloc(strlen(toReturn)+1);
    t[0] = '\0';
    for (int i = strlen(toReturn)-1; i >= 0; i--) {
        t[strlen(toReturn)-i-1] = toReturn[i];
    }
    t[strlen(toReturn)] = '\0';
    
    return t;
}
// Removes fraction from a number.
long double removeFraction(long double n, int precision) {
    long double frac = n > 0 ? fabs(n - (int64_t)n) : n;
    long double toReturn = frac;
    int times = 0;
    while (frac != 0 && times <= precision) {
        toReturn *= 10;
        frac /= 10;
        times++;
    }
    return (long double)(toReturn);
}
char* convertBaseFraction(long double n, int base, char* charset) {
    char toReturn[500]; toReturn[0] = '\0';
    long double currentNum = n;
    int maxPrecision = LDBL_DIG + 2; 
    while (currentNum > 0.0L && strlen(toReturn) < maxPrecision && strlen(toReturn) < 498)
    {
        long double shifted = currentNum*base;
        long double sIntPart;
        long double sFracPart = modfl(shifted, &sIntPart);
        
        int idx = (int)sIntPart;
        int len = strlen(toReturn);
        toReturn[len] = charset[idx];
        toReturn[len+1] = '\0';
        currentNum = sFracPart;
    }
    return strdup(toReturn);
}
// Converts base-10 long double to base n, but also works with floats.
char* convertBasef(long double n, int base, char* charsetA) {
    bool isNeg = n < 0;
    char* charset = charsetA;
    if (charset == NULL && base != 64) charset = defaultCharset;
    else if (base == 64) charset = b64Charset;

    long double whole = fabsl(trunc(n)); // -2.3 -> 2
    char precision[50];
    sprintf(precision, "%Lf", n);
    int afterDot = -1;
    int zeroesAmt = 0;
    for (int i = 0; i < strlen(precision); i++) {
        if (precision[i] == '.') afterDot++;
        if (afterDot >= 0) afterDot++;
        if (precision[i] == '0' && afterDot >= 0) zeroesAmt++;
        if (zeroesAmt >= 1) break;
    }
    long double intpart;
    
    long double frac = modfl(n, &intpart);
    char* wholePart = convertBase(whole, base, charset);

    char* fracPart = convertBaseFraction(frac, base, charset);
    char* toReturn = malloc(strlen(wholePart)+strlen(fracPart)+1+(strlen(fracPart) > 0)+isNeg);
    char* dot = strlen(fracPart) > 0 ? "." : "";
    if (isNeg) sprintf(toReturn, "-%s%s%s", wholePart, dot, fracPart);
    else sprintf(toReturn, "%s%s%s", wholePart, dot, fracPart);

    free(wholePart);
    free(fracPart);
    
    return toReturn;
}
// Finds the index of a char inside of a string.
int findChar(char* charset, char n) {
    for (int i = 0; i < strlen(charset); i++) {
        if (charset[i] == n) return i;
    }
    return -1;
}
// Converts from a base back to base 10.
long double baseToDec(char* n, int base, char* charsetA) {
    char* charset = charsetA;
    if (charset == NULL && base != 64) charset = defaultCharset;
    else if (base == 64) charset = b64Charset;

    long double toReturn = 0;
    int commaIdx = findChar(n, '.');
    long double wholePart = 0;
    long double fracPart = 0;
    int pow = 0;
    bool commafound = commaIdx != -1;
    if (commaIdx == -1) commaIdx = strlen(n);
    for (int i = commaIdx-1; i >= 0; i--) {
        toReturn += (findChar(charset, n[i])*powl(base, pow));
        pow++;
    }
    if (!commafound) return toReturn;
    pow = -1;
    for (int i = commaIdx+1; i < strlen(n); i++) {
        toReturn += findChar(charset, n[i])*powl(base, pow);
        pow--;
    }
    return toReturn;
}
// C implementation for baseToDec(), binToDec(), octToDec(), dozToDec(), hexToDec(), b64ToDec(), decToBase(), decToBin(), decToOct(), decToDoz(), decToHex(), and decToB64()
void mBases(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (strcmp(identifier, "baseToDec") == 0 || strcmp(identifier, "binToDec") == 0 || strcmp(identifier, "octToDec") == 0 || strcmp(identifier, "dozToDec") == 0 || strcmp(identifier, "hexToDec") == 0 || strcmp(identifier, "b64ToDec") == 0 ) {
        typedValue* arg1 = NULL;
        if (args->length >= 2) {arg1 = getArray(args,  1); if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);}
        typedValue* arg2 = NULL;
        if (args->length >= 3) arg2 = getArray(args,  2);
        if (arg2 != NULL) {if (arg2->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);}
        
        if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        char* charset = arg2 == NULL ? NULL : strArrToChar(arg2);
        int base = arg1 != NULL ? convertNum(arg1->value.numberValue, NUM_INT).value.iVal : 0;
        if (base == 0) {
            if (strcmp(identifier, "binToDec") == 0) base = 2;
            if (strcmp(identifier, "octToDec") == 0) base = 8;
            if (strcmp(identifier, "dozToDec") == 0) base = 12;
            if (strcmp(identifier, "hexToDec") == 0) base = 16;
            if (strcmp(identifier, "b64ToDec") == 0) base = 64;
        }
        if (base < 2) fatalError(0x30, "", -1);
        
        char* toConvert = strArrToChar(arg0);

        long double res = baseToDec(toConvert, base, charset);

        typedValue* toReturn = numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = res}, arg1 == NULL ? NUM_LONGDOUBLE : arg1->value.numberValue.type));
        pushArray(vms->stack,toReturn);

        if (arg2 != NULL) free(charset);
        free(toConvert);
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
    else if (strcmp(identifier, "decToBase") == 0 || strcmp(identifier, "decToBin") == 0 || strcmp(identifier, "decToOct") == 0 || strcmp(identifier, "decToDoz") == 0 || strcmp(identifier, "decToHex") == 0 || strcmp(identifier, "decToB64") == 0 ) {
        typedValue* arg1 = NULL;
        if (args->length >= 2) {arg1 = getArray(args,  1); if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);}
        typedValue* arg2 = NULL;
        if (args->length >= 3) arg2 = getArray(args,  2);
        if (arg2 != NULL) {if (arg2->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);}
        
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        char* charset = arg2 == NULL ? NULL : strArrToChar(arg2);
        int base = arg1 != NULL ? convertNum(arg1->value.numberValue, NUM_INT).value.iVal : 0;
        if (base == 0) {
            if (strcmp(identifier, "decToBin") == 0) base = 2;
            if (strcmp(identifier, "decToOct") == 0) base = 8;
            if (strcmp(identifier, "decToDoz") == 0) base = 12;
            if (strcmp(identifier, "decToHex") == 0) base = 16;
            if (strcmp(identifier, "decToB64") == 0) base = 64;
        }
        if (base < 2) fatalError(0x30, "", -1);
        
        long double toConvert = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;

        char* res = convertBasef(toConvert, base, charset);

        typedValue* toReturn = newTVArray(strlen(res)+1, AT_CHARARR);
        for (int i = 0; i <= strlen(res); i++) {
            typedValue* toApp = numToTV((num){.type = NUM_CHAR, .value.cVal = i == strlen(res) ? 0 : res[i]});
            toReturn->value.av.data[i] = toApp;
        };
        pushArray(vms->stack,toReturn);

        if (arg2 != NULL) free(charset);
        freeTypedValue(arg0);
        freeTypedValue(arg1);
        free(res);
    }

}
// C implementation for log, log10, log2, & ln.
void mLog(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    long double base = 0.0L;
    long double numn = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    typedValue* arg1 = NULL;
    if (strcmp(identifier, "log") == 0) {
        arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        base = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    }
    else if (strcmp(identifier, "ln") == 0) base = M_E;
    else if (strcmp(identifier, "log10") == 0) base = 10.0L;
    else if (strcmp(identifier, "log2") == 0) base = 2.0L;
    if (numn <= 0 || base <= 0 || base == 1) fatalError(0x30, "Either first or second argument is outside the domain of log(). Try using the complexNum struct", -1); // Function is outside the domain of the log() function.
    long double result = logl(numn)/logl(base); // log base change property

    typedValue* toReturn = numToTV(convertNum((num){.value.ldVal = result,.type = NUM_LONGDOUBLE}, arg0->value.numberValue.type));
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    if (arg1 != NULL) freeTypedValue(arg1);

}
// C implementation for hypot() & sidel().
void mHypot(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    typedValue* arg1 = getArray(args,  1);
    if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    long double a = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double b = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    if ( (a <= 0 || b <= 0 && strcmp(identifier, "hypot") == 0) ) fatalError(0x30, "Cannot find the hypotenuse of a triangle with negative sides.", -1);
    if ( (a > b) && strcmp(identifier, "sidel") == 0) fatalError(0x30, "Cannot find the sidelength of a triangle with a given sidelength greater than its hypotenuse", -1);
    // a^2 + b^2 = c^2
    // c = sqrt(a^2 + b^2)
    
    // a^2 + b^2 = c^2
    // b^2 = c^2 - a^2
    // b = sqrt(c^2 - a^2)
    long double result = strcmp(identifier, "hypot") == 0 ? sqrtl((a*a) + (b*b)) : sqrtl((b*b) - (a*a));

    typedValue* toReturn = numToTV(convertNum((num){.value.ldVal = result, .type = NUM_LONGDOUBLE}, arg0->value.numberValue.type));
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
    freeTypedValue(arg1);
}
// Converts from degrees to radians & radians to degrees.
long double dToR(long double d) {
    return d*(M_PI/180);
}
long double rToD(long double r) {
    return r*(180/M_PI);
}
// Returns if a ld is >= min && <= max.
bool inclusiveRangel(long double n, long double min, long double max) {
    return (n >= min && n <= max);
}
// Implementation for trig functions.
void mTrig(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    if (arg0->value.numberValue.type < NUM_FLOAT) fatalError(0x30, "Only floats, doubles, or long doubles are valid for trig functions.",-1);
    long double n = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double result = 0.0L;
    bool isDegrees = false;
    char idx[50]; idx[0] = '\0';
    for (int i = 0; i < strlen(identifier); i++) {
        if (identifier[i] == 'd') {n = dToR(n); isDegrees = true; continue;}
        
        int len = strlen(idx);
        idx[len] = identifier[i];
        idx[len+1] = '\0';
    }

    if (strcmp(idx, "sin") == 0) {
        result = sinl(n);
    }
    else if (strcmp(idx, "cos") == 0) {
        result = cosl(n);
    }
    else if (strcmp(idx, "tan") == 0) {
        if (cosl(n) == 0) fatalError(0x30, "Cannot pass an odd multiple of pi/2 into tan(x)", -1);
        result = tanl(n);
    }
    else if (strcmp(idx, "cot") == 0) {
        if (sinl(n) == 0) fatalError(0x30, "Cannot pass an odd multiple of pi/2 into tan(x)", -1);
        result = 1/tanl(n);
    }
    else if (strcmp(idx, "sec") == 0) {
        if (cosl(n) == 0) fatalError(0x30, "Cannot pass an odd multiple of pi/2 into sec(x)", -1);
        result = 1/cosl(n);
    }
    else if (strcmp(idx, "csc") == 0) {
        if (sinl(n) == 0) fatalError(0x30, "Cannot pass an odd multiple of pi into tan(x)", -1);
        result = 1/sinl(n);
    }
    else if (strcmp(idx, "asin") == 0) {
        if (!inclusiveRangel(n, -1, 1)) fatalError(0x30, "asin()'s domain is [-1, 1] inclusive.",-1);
        result = asinl(n);
    }
    else if (strcmp(idx, "acos") == 0) {
        if (!inclusiveRangel(n, -1, 1)) fatalError(0x30, "acos()'s domain is [-1, 1] inclusive.",-1);
        result = acosl(n);
    }
    else if (strcmp(idx, "atan") == 0) {
        result = atanl(n);
    }
    else if (strcmp(idx, "acot") == 0) {
        if (n == 0) result = M_PI/2;
        else if (n > 0) {
            result = atanl(1/n);
        }
        else {
            result = atanl(1/n) + M_PI;
        }
    }
    else if (strcmp(idx, "asec") == 0) {
        if (inclusiveRangel(n, -1, 1) && n != -1 && n != 1) fatalError(0x30, "asec()'s domain is (-inf, -1), (1, inf) inclusive", -1);
        result = acosl(1/n);
    }
    else if (strcmp(idx, "acsc") == 0) {
        if (inclusiveRangel(n, -1, 1) && n != -1 && n != 1) fatalError(0x30, "acsc()'s domain is (-inf, -1), (1, inf) inclusive.", -1);
        result = asinl(1/n);
    }
    else if (strcmp(idx, "sinh") == 0) {
        result = sinhl(n);
    }
    else if (strcmp(idx, "cosh") == 0) {
        result = coshl(n);
    }
    else if (strcmp(idx, "tanh") == 0) {
        result = tanhl(n);
    }
    else if (strcmp(idx, "coth") == 0) {
        if (tanhl(n) == 0) fatalError(0x30, "Cannot pass zero into coth(x)", -1);
        else result = 1/tanhl(n);
    }
    else if (strcmp(idx, "sech") == 0) {
        result = 1/coshl(n);
    }
    else if (strcmp(idx, "csch") == 0) {
        if (sinhl(n) == 0) fatalError(0x30, "Cannot pass zero into csch(x)", -1);
        result = 1/sinhl(n);
    }
    else if (strcmp(idx, "asinh") == 0) {
        result = asinhl(n);
    }
    else if (strcmp(idx, "acosh") == 0) {
        if (n < 1) fatalError(0x30, "arccosh()'s domain is [1, inf) exclusive", -1);
        result = acoshl(n);
    }
    else if (strcmp(idx, "atanh") == 0) {
        result = atanhl(n);
    }
    else if (strcmp(idx, "acoth") == 0) {
        if (n < 1 && n > -1) fatalError(0x30, "acoth()'s domain is (-inf, -1)|(1, inf) inclusive", -1);
        result = atanhl(1/n);
    }
    else if (strcmp(idx, "asech") == 0) {
        if (!inclusiveRangel(n, 0, 1)) fatalError(0x30, "asech()'s domain is [-1, 1] inclusive.", -1);
        if (n == 0) fatalError(0x30, "asech(0) is undefined", -1);
        result = acoshl(1/n);
    }
    else if (strcmp(idx, "acsch") == 0) {
        if (n == 0) fatalError(0x30, "acsch(0) is undefined.", -1);
        result = asinhl(1/n);
    }

    typedValue* toReturn = numToTV(convertNum((num){.value.ldVal = result,.type = NUM_LONGDOUBLE}, arg0->value.numberValue.type));
    pushArray(vms->stack,toReturn);
    freeTypedValue(arg0);
}
// C implementation for dToR() and rToD().
void mDegreeConversion(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    long double n = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double toReturn = 0;
    if (strcmp(identifier, "dToR") == 0) toReturn = dToR(n);
    if (strcmp(identifier, "rToD") == 0) toReturn = rToD(n);
    int typeToReturn = arg0->value.numberValue.type >= NUM_FLOAT ? arg0->value.numberValue.type : NUM_FLOAT;
    typedValue* toReturnV = numToTV(convertNum((num){.value.ldVal = toReturn,.type = NUM_LONGDOUBLE}, typeToReturn));
    pushArray(vms->stack,toReturnV);
    freeTypedValue(arg0);
}
// C implementations for ceil, floor, trunc, dPart, round, abs, & sign
void mDecFuncs(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    long double n = convertNum(arg0->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
    long double toReturn = 0;
    if (strcmp(identifier, "ceil") == 0) toReturn = ceill(n);
    if (strcmp(identifier, "floor") == 0) toReturn = floorl(n);
    if (strcmp(identifier, "trunc") == 0) toReturn = truncl(n);
    if (strcmp(identifier, "dPart") == 0) {
        long double sign = n < 0 ? -1 : 1;
        long double precision = floorl(log10l(abs(n)));
        long double roundedPart = roundl(n/powl(10, -precision))*powl(10,-precision);
        toReturn = n-roundedPart;
    }
    if (strcmp(identifier, "round") == 0) {
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        long double precision = convertNum(arg1->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
        long double m = powl(10, -precision);
        toReturn = roundl(n/m)*m;
        freeTypedValue(arg1);
    }
    if (strcmp(identifier, "abs") == 0) toReturn = fabsl(n);
    if (strcmp(identifier, "sign") == 0) toReturn = n < 0 ? -1 : n > 0 ? 1 : 0;
    
    typedValue* toReturnV = numToTV(convertNum((num){.value.ldVal = toReturn,.type = NUM_LONGDOUBLE}, arg0->value.numberValue.type));
    pushArray(vms->stack,toReturnV);
    freeTypedValue(arg0);
}
// gcd & lcm formulas.
int64_t gcd(int aa, int bb) {
    // W euclid
    if (bb == 0) return __max(aa, -aa);
    return gcd(bb, aa % bb);
}
int64_t lcm(int64_t aa, int64_t bb) {
    return abs(aa*bb)/gcd(aa,bb);
}
typedef struct {
    int64_t* data;
    int len;
} llarr;
// Returns if a number is prime or not.
bool isPrime(int64_t aa) {
    if (aa == 1 || aa == 2 || aa == 3) return true;
    if (aa % 2 == 0) return false;
    for (int i = 3; i < aa/2; i += 2) {
        if (aa % i == 0) return false;
    }
    return true;
}
// Computes prime factorization, returning it as a ll array.
void primeFactorization(int64_t aa, llarr* ll) {
    // 212 -> 2*106 -> 2*2*53
    int64_t a = aa;
    int i = 2;
    while (a > 1) {
        if (isPrime(i) && a % i == 0) {
            a /= i;
            ll = srealloc(ll, (sizeof(int64_t) * (ll->len+1) ) );
            ll->data[ll->len] = i;
            ll->len++;
            if (a == 1) break;
            if (a % i == 0) i--;
        }
        i++;
    }
}
// C implementation for lcm, gcd, isPrime, primeFac, array_sum, & array_prod.
void mNumberFunctions(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (strcmp(identifier, "lcm") == 0 || strcmp(identifier, "gcd") == 0) {
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (arg0->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "gcd & lcm only work with ints.", -1);
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (arg1->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "gcd & lcm only work with ints.", -1);
        int64_t a = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        int64_t b = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
        int64_t result = 0.0L;
        if (strcmp(identifier, "lcm") == 0) result = lcm(a, b);
        else result = gcd(a, b);

        typedValue* toReturn = poolAlloc(globalPool);
        *toReturn = (typedValue){
            .ptr = NULL,
            .value.numberValue = convertNum((num){
               .value.lVal = result,
               .type = NUM_LONG
            }, arg0->value.numberValue.type),
            .valueType = TYPE_NUM
        };
        pushArray(vms->stack,toReturn);
        freeTypedValue(arg0);
        freeTypedValue(arg1);
    }
    else if (strcmp(identifier, "isPrime") == 0) {
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (arg0->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "isPrime() only works with integers.", -1);
        int64_t a = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        typedValue* toReturn = numToTV((num){.type = NUM_BOOL, .value.bVal=isPrime(a)});
        pushArray(vms->stack,toReturn);
        freeTypedValue(arg0);
    }
    else if (strcmp(identifier, "primeFac") == 0) {
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        if (arg0->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "isPrime() only works with integers.", -1);
        int64_t a = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        llarr p = (llarr){.data = malloc(1), .len = 0};
        primeFactorization(a, &p);
        typedValue* toReturn = poolAlloc(globalPool);
        *toReturn = (typedValue){
            .ptr = NULL,
            .valueType = TYPE_ARRAY,
            .value.av = (arrayValue){
                .data = malloc(sizeof(typedValue*)*(p.len)),
                .arrayType = AT_NUM,
                .len = p.len
            }
        } ;
        for (int i = 0; i < p.len; i++) {
            typedValue* toAdd = numToTV(convertNum((num){.value.lVal = p.data[i],.type = NUM_LONG}, arg0->value.numberValue.type));
            toReturn->value.av.data[i] = toAdd;
        }
        pushArray(vms->stack,toReturn);
        freeTypedValue(arg0);
        free(p.data);
    }
    else if (strcmp(identifier, "array_sum") == 0 || strcmp(identifier, "array_prod") == 0) {
        if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        long double result = strcmp(identifier, "array_sum") == 0 ? 0 : 1;
        int type = NUM_INT;
        for (int i = 0; i < arg0->value.av.len; i++) {
            typedValue* tv = arg0->value.av.data[i];
            if (tv->valueType != TYPE_NUM) fatalError(0x30, "Cannot take the sum or product of an array with a nonnumerical element.", -1);
            type = tv->value.numberValue.type;
            long double arrN = convertNum(tv->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
            if (strcmp(identifier, "array_sum") == 0) {
                result += arrN;
            }
            else {
                result *= arrN;
            }
        }
        typedValue* toReturn = numToTV(convertNum((num){.value.ldVal = result, .type = NUM_LONGDOUBLE}, type));
        pushArray(vms->stack,toReturn);
        freeTypedValue(arg0);
    }
}
// Computes the quantiles of a list.
long double* quantiles(long double* list, int len, int amt) {
    long double* toReturn = malloc(sizeof(long double)*(amt-1)); // amt=4 -> return 3 nums
    for (int i = 1; i < amt; i++) {
        long double r = ((long double)i/amt)*(long double)(len-1);
        if (r == ceill(r)) {
            toReturn[i-1] = list[(int)r];
        }
        else {
            long double weight = r - floorl(r);
            toReturn[i-1] = list[(int)floorl(r)] + weight * (list[(int)ceill(r)] - list[(int)floorl(r)]);
        }
    }
    return toReturn;
}
// Computes median of a list
long double median(long double* list, int len) {
    if (len % 2 == 0) {
        return (list[(int)floor(len/2)]+list[(int)ceil(len/2)])/2;
    }
    else {
        return list[len/2];
    }
}
// Computes mean of a list
long double mean(long double* list, int len) {
    long double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += list[i];
    }
    return sum/len;
}
// Compares two LDs (used in sorting)
int cmpLD(const void* av, const void* bv) {
    long double* a = (long double*)av;
    long double* b = (long double*)bv;
    if (*a > *b) return 1;
    else if (*a < *b) return -1;
    return 0;
}
// C implementation for array_mean, array_median, & array_quantiles.
void mListOp(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
    if (arg0->value.av.arrayType != AT_NUM) fatalError(0x30, "", -1);

    
    int len = arg0->value.av.len;
    long double* arr = malloc(sizeof(long double)*len);
    int type = -NUM_FLOAT;
    for (int i = 0; i < len; i++) {
        typedValue* arg0N = arg0->value.av.data[i];
        if (arg0N->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        arr[i] = convertNum(arg0N->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
        type = arg0N->value.numberValue.type > type ? arg0N->value.numberValue.type : type;
    }
    if (strcmp(identifier, "array_mean") == 0 || strcmp(identifier, "array_median") == 0) {
        long double result = strcmp(identifier, "array_median") == 0 ? median(arr, len) : mean(arr, len);
        pushArray(vms->stack,numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal = result}, type) ) );
    }
    else {
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        int q = convertNum(arg1->value.numberValue, NUM_INT).value.iVal;
        qsort(arr, len, sizeof(long double), cmpLD);
        long double* res = quantiles(arr, len, q);
        typedValue* toReturn = newTVArray(q-1, AT_NUM);
        for (int i = 0; i < q-1; i++) {
            toReturn->value.av.data[i] = numToTV(convertNum((num){.type = NUM_LONGDOUBLE, .value.ldVal=res[i]}, type));
        }
        pushArray(vms->stack,toReturn);
    }

    free(arr);
    freeTypedValue(arg0);
}