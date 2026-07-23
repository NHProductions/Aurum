#ifndef MATHFUNC_H
#define MATHFUNC_H
#include "bytecoder.h"
#include "vm.h"
#include "sysFunctions.h"
void mBases(virtualMachineState* vms, char* identifier, List* args);
void mLog(auFunc);
void mHypot(auFunc);
void mTrig(auFunc);
void mDegreeConversion(auFunc);
void mDecFuncs(auFunc);
void mNumberFunctions(auFunc);
void mListOp(auFunc);
static const char* mathDefinitions = "\n\
const float M_PI8 = 3.1415926f;\n\
const double M_PI16 = 3.141592653589793d;\n\
const double128 M_PI32 = 3.1415926535897932384626433832795ld;\n\
const double M_PI = M_PI16;\n\
const double128 M_E32 = 2.7182818284590452353602874713526ld;\n\
const double M_E16 = 2.7182818284590452353602874713526d;\n\
const float M_E8 = 2.7182818f;\n\
const double M_E = M_E16;\n\
const float M_TAU8 = 6.2831853f;\n\
const double M_TAU16 = 6.283185307179586d;\n\
const double128 M_TAU32 = 6.2831853071795864769252867665590ld;\n\
const double M_TAU = M_TAU16;\n\
const float M_SQRT28 = 1.4142135f;\n\
const double M_SQRT216 = 1.414213562373095d;\n\
const double128 M_SQRT232 = 1.4142135623730950488016887242096ld;\n\
const float M_SQRT2 = M_SQRT216;\n\
const float M_GRAT8 = 1.6180339f;\n\
const double M_GRAT16 = 1.618033988749894d;\n\
const double128 M_GRAT32 = 1.6180339887498948482045868343656ld;\n\
const double M_GRAT = M_GRAT16;\n\
const float M_SSUBNORMALF = 2f**-149f;\n\
const double M_SSUBNORMALD = 2d**-1074d;\n\
const double128 M_SUBNORMALLD = 2ld**-16495ld;\n\
const float M_LSUBNORMALF = (2f-(2f**-23f))*(2f**-126f);\n\
const double M_LSUBNORMALD = (2d-(2d**-52d))*(2d**-1022d);\n\
const double128 M_LSUBNORMALLD = (2ld-(2ld**-112ld))*(2ld**-16382ld);\n\
const float M_EPSILONF = (2f**-23f);\n\
const double M_EPSILOND = (2f**-52f);\n\
const double128 M_EPSILONLD = (2f**-112f);\n\
const float M_FMAX = (2f-(2f**-23f))*(2f**127f);\n\
const double M_DMAX = (2d-(2d**-52d))*(2d**1023d);\n\
const double128 M_LDMAX = (2ld-(2ld**-112ld))*(2ld**16383ld);\n\
const float M_FMIN = -M_FMAX;\n\
const double M_DMIN = -M_DMAX;\n\
const double128 M_LDMIN = -M_LDMAX;\n\
const char M_MAX8 = 127c;\n\
const char M_MAXC = 127c;\n\
const short M_MAX16 = 16383s;\n\
const short M_MAXS = 16383s;\n\
const int M_MAX32 = 0x7fffffffi;\n\
const int M_MAXI = 0x7fffffffi;\n\
const long M_MAX64 = 0x7fffffffffffffffl;\n\
const long M_MAXL = 0x7fffffffffffffffl;\n\
const char M_MIN8 = -127c;\n\
const char M_MINC = -127c;\n\
const short M_MIN16 = -16383s;\n\
const short M_MINS = -16383s;\n\
const int M_MIN32 = -0x7fffffffi;\n\
const int M_MINI = -0x7fffffffi;\n\
const long M_MIN64 = -0x7fffffffffffffffl;\n\
const long M_MINL = -0x7fffffffffffffffl;\n\
const char M_MAX8U = 255uc;\n\
const char M_MAXUC = 255uc;\n\
const short M_MAX16U = 32767us;\n\
const short M_MAXUS = 32767us;\n\
const int M_MAX32U = 4294967295ui;\n\
const int M_MAXUI = 4294967295ui;\n\
const long M_MAX64U = 18446744073709551615ul;\n\
const long M_MAXUL = 18446744073709551615ul;\n\
";
static const bcFunction mathFunctions[] = {
    // Base conversions:
    {.name = "decToBase", .argc = 3, .returnStruct = NULL}, // X
    {.name = "baseToDec", .argc = 3, .returnStruct = NULL}, // X
    {.name = "decToBin", .argc = 1, .returnStruct = NULL}, // X
    {.name = "decToOct", .argc = 1, .returnStruct = NULL}, // X
    {.name = "decToDoz", .argc = 1, .returnStruct = NULL}, // X
    {.name = "decToHex", .argc = 1, .returnStruct = NULL}, // X
    {.name = "decToB64", .argc = 1, .returnStruct = NULL}, // X
    {.name = "binToDec", .argc = 1, .returnStruct = NULL}, // X
    {.name = "octToDec", .argc = 1, .returnStruct = NULL}, // X
    {.name = "dozToDec", .argc = 1, .returnStruct = NULL}, // X
    {.name = "hexToDec", .argc = 1, .returnStruct = NULL}, // X
    {.name = "b64ToDec", .argc = 1, .returnStruct = NULL}, // X
    // Exponents
    {.name = "ln", .argc = 1, .returnStruct = NULL}, // X
    {.name = "log2", .argc = 1, .returnStruct = NULL}, // X
    {.name = "log10", .argc = 1, .returnStruct = NULL}, // X
    {.name = "log", .argc = 2, .returnStruct = NULL}, // X
    // Geometry:
    {.name = "hypot", .argc = 2, .returnStruct = NULL}, // X
    {.name = "sidel", .argc = 2, .returnStruct = NULL}, // X
    // Basic Trig functions
    {.name = "sin", .argc = 1, .returnStruct = NULL}, // X
    {.name = "cos", .argc = 1, .returnStruct = NULL}, // X 
    {.name = "tan", .argc = 1, .returnStruct = NULL},// X
    {.name = "cot", .argc = 1, .returnStruct = NULL},// X
    {.name = "sec", .argc = 1, .returnStruct = NULL},// X
    {.name = "csc", .argc = 1, .returnStruct = NULL},// X
    {.name = "asin", .argc = 1, .returnStruct = NULL},// X
    {.name = "acos", .argc = 1, .returnStruct = NULL},// X
    {.name = "atan", .argc = 1, .returnStruct = NULL},// X
    {.name = "acot", .argc = 1, .returnStruct = NULL},// X
    {.name = "asec", .argc = 1, .returnStruct = NULL},// X
    {.name = "acsc", .argc = 1, .returnStruct = NULL},// X
    {.name = "sinh", .argc = 1, .returnStruct = NULL},// X
    {.name = "cosh", .argc = 1, .returnStruct = NULL},// X
    {.name = "tanh", .argc = 1, .returnStruct = NULL},// X
    {.name = "coth", .argc = 1, .returnStruct = NULL},// X
    {.name = "sech", .argc = 1, .returnStruct = NULL},// X
    {.name = "csch", .argc = 1, .returnStruct = NULL},// X
    {.name = "asinh", .argc = 1, .returnStruct = NULL},// X
    {.name = "acosh", .argc = 1, .returnStruct = NULL},// X
    {.name = "atanh", .argc = 1, .returnStruct = NULL},// X
    {.name = "acoth", .argc = 1, .returnStruct = NULL},// X
    {.name = "asech", .argc = 1, .returnStruct = NULL},// X
    {.name = "acsch", .argc = 1, .returnStruct = NULL},// X
    {.name = "sind", .argc = 1, .returnStruct = NULL},// X
    {.name = "cosd", .argc = 1, .returnStruct = NULL},// X
    {.name = "tand", .argc = 1, .returnStruct = NULL},// X
    {.name = "cotd", .argc = 1, .returnStruct = NULL},// X
    {.name = "secd", .argc = 1, .returnStruct = NULL},// X
    {.name = "cscd", .argc = 1, .returnStruct = NULL},// X
    {.name = "asind", .argc = 1, .returnStruct = NULL},// X
    {.name = "acosd", .argc = 1, .returnStruct = NULL},// X
    {.name = "atand", .argc = 1, .returnStruct = NULL},// X
    {.name = "acotd", .argc = 1, .returnStruct = NULL},// X
    {.name = "asecd", .argc = 1, .returnStruct = NULL},// X
    {.name = "acscd", .argc = 1, .returnStruct = NULL},// X
    {.name = "sinhd", .argc = 1, .returnStruct = NULL},// X
    {.name = "coshd", .argc = 1, .returnStruct = NULL},// X
    {.name = "tanhd", .argc = 1, .returnStruct = NULL},// X
    {.name = "cothd", .argc = 1, .returnStruct = NULL},// X
    {.name = "sechd", .argc = 1, .returnStruct = NULL},// X
    {.name = "cschd", .argc = 1, .returnStruct = NULL},// X
    {.name = "asinhd", .argc = 1, .returnStruct = NULL},// X
    {.name = "acoshd", .argc = 1, .returnStruct = NULL},// X
    {.name = "atanhd", .argc = 1, .returnStruct = NULL},// X
    {.name = "acothd", .argc = 1, .returnStruct = NULL},// X
    {.name = "asechd", .argc = 1, .returnStruct = NULL},// X
    {.name = "acschd", .argc = 1, .returnStruct = NULL},// X
    
    {.name = "dToR", .argc = 1, .returnStruct = NULL}, // X
    {.name = "rToD", .argc = 1, .returnStruct = NULL}, // X
    // Numbers:
    {.name = "ceil", .argc = 1, .returnStruct = NULL},// X
    {.name = "floor", .argc = 1, .returnStruct = NULL},// X
    {.name = "trunc", .argc = 1, .returnStruct = NULL},// X
    {.name = "round", .argc = 2, .returnStruct = NULL},// X
    {.name = "dPart", .argc = 1, .returnStruct = NULL},// X
    {.name = "sign", .argc = 1, .returnStruct = NULL},// X
    {.name = "abs", .argc = 1, .returnStruct = NULL},// X

    {.name = "gcd", .argc = 2, .returnStruct = NULL}, // X
    {.name = "lcm", .argc = 2, .returnStruct = NULL}, // X
    {.name = "isPrime", .argc = 1, .returnStruct = NULL}, // X
    {.name = "primeFac", .argc = 1, .returnStruct = "array"}, // X
    {.name = "array_sum", .argc = 1, .returnStruct = NULL}, // X
    {.name = "array_prod", .argc = 1, .returnStruct = NULL}, // X

    // Statistics:
    {.name = "array_mean", .argc = 1, .returnStruct = NULL}, // X
    {.name = "array_median", .argc = 1, .returnStruct = NULL}, // X
    {.name = "array_quantiles", .argc = 1, .returnStruct = "array"}, // X
    {.name = "", .argc = -1} // Terminator
};
#endif