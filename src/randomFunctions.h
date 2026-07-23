#ifndef RANDOMFUNC_H
#define RANDOMFUNC_H
#include "sysFunctions.h"
#include "bytecoder.h"

void rRandom(auFunc);
static const bcFunction randomFunctions[] = {
    {.name = "srand", .argc = 1, .returnStruct = NULL}, // X
    {.name = "rand", .argc = 0, .returnStruct = NULL}, // X
    {.name = "randL", .argc = 2, .returnStruct = NULL}, // X
    {.name = "randLD", .argc = 2, .returnStruct = NULL}, // X
    {.name = "factorial", .argc = 1, .returnStruct = NULL}, // X
    {.name = "gamma", .argc = 1, .returnStruct = NULL}, // X
    {.name = "lgamma", .argc = 1, .returnStruct = NULL}, // X
    {.name = "permutations", .argc = 3, .returnStruct = NULL}, // X
    {.name = "combinations", .argc = 3, .returnStruct = NULL}, // X
    {.name = "array_choose", .argc = 0, .returnStruct = NULL}, // X
    {.name = "", .argc = -1}
};


#endif