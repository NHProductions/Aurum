#ifndef TIMEFUNC_H
#include "sysFunctions.h"
#include "../processing/bytecoder.h"
void tTime(auFunc);
static const bcFunction timeFunctions[] = {
    {.name = "time", .argc = 0, .returnStruct = NULL},
    {.name = "clock", .argc = 0, .returnStruct = NULL},
    {.name = "toUTC", .argc = 1, .returnStruct = "UTCTime"},
    {.name = "getUTC", .argc = 0, .returnStruct = "UTCTime"},
    {.name = "timeld", .argc = 0, .returnStruct = NULL},
    {.name = "sleep", .argc = 1, .returnStruct = NULL},
    {.name = "", .argc = -1}
};
static const char* timeDefinitions = "\n\
struct UTCTime {\n\
int ms;\n\
int second;\n\
int minute;\n\
int hour;\n\
int mday;\n\
int month;\n\
int year;\n\
int wday;\n\
int yday;\n\
bool isdst;\n\
};";
#endif