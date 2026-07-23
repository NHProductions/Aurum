/*
--- timeFunctions.c ---
This contains functions used in @time.

*/

#include "timeFunctions.h"
#include <time.h>

int64_t getMS() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC)) {
        int64_t ms = ((int64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
        return ms;
    }
    return 0;
}
int getMSint() {
    struct timespec ts;
    if (timespec_get(&ts, TIME_UTC)) {
        // Isolate the nanoseconds and convert them to a 0-999 millisecond value
        return (int)(ts.tv_nsec / 1000000);
    }
    return 0;
}
// C implementations for time(), clock(), toUTC(), & sleep().
void tTime(auFunc) {
    if (strcmp(identifier, "time") == 0) {List_InsertElement(vms->stack, 0, numToTV((num){.type = NUM_LONG, .value.lVal = (int64_t)time(NULL)})); return;}
    if (strcmp(identifier, "clock") == 0) {List_InsertElement(vms->stack, 0, numToTV((num){.type = NUM_LONG, .value.lVal = (int64_t)clock()})); return;}
    if (strcmp(identifier, "toUTC") == 0) {
        typedValue* arg0 = List_GetElement(args, 0)->data;
        if (arg0->valueType != TYPE_NUM) {fatalError(0x30, "", -1);}
        if (arg0->value.numberValue.type >= NUM_FLOAT) fatalError(0x30, "", -1);
        int64_t t = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        time_t conv = (time_t)t;
        struct tm *utc = gmtime(&conv);
        if (utc == NULL) {
            fatalError(0x30, "Time conversion failed.", -1);
        }
        typedValue* toReturn = malloc(sizeof(typedValue));
        *toReturn = (typedValue){
            .ptr = NULL,
            .value.so.def = getStructDefViaName("UTCTime", *vms->bc),
            .value.so.fields = malloc(sizeof(typedValue*)*10),
            .valueType = TYPE_STRUCT,
        };
        toReturn->value.so.fields[0] = numToTV((num){.type = NUM_INT, .value.iVal = 0});
        toReturn->value.so.fields[1] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_sec});
        toReturn->value.so.fields[2] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_min});
        toReturn->value.so.fields[3] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_hour});
        toReturn->value.so.fields[4] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_mday});
        toReturn->value.so.fields[5] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_mon+1});
        toReturn->value.so.fields[6] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_year+1900});
        toReturn->value.so.fields[7] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_wday});
        toReturn->value.so.fields[8] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_yday});
        toReturn->value.so.fields[9] = numToTV((num){.type = NUM_BOOL, .value.iVal = utc->tm_isdst != 0});
        for (int i = 0; i < 9; i++) {
            stackPtr* sp = malloc(sizeof(stackPtr));
            sp->addr = toReturn;
            sp->isTV = true;
            sp->idx = i;
            toReturn->value.so.fields[i]->ptr = sp;
        }
        free(arg0);
        List_InsertElement(vms->stack, 0, toReturn);
    }
    if (strcmp(identifier, "getUTC") == 0) {
        time_t conv = time(NULL);
        struct tm *utc = gmtime(&conv);
        if (utc == NULL) {
            fatalError(0x30, "Time conversion failed.", -1);
        }
        typedValue* toReturn = malloc(sizeof(typedValue));
        *toReturn = (typedValue){
            .ptr = NULL,
            .value.so.def = getStructDefViaName("UTCTime", *vms->bc),
            .value.so.fields = malloc(sizeof(typedValue*)*10),
            .valueType = TYPE_STRUCT,
        };
        toReturn->value.so.fields[0] = numToTV((num){.type = NUM_INT, .value.iVal = getMSint()});
        toReturn->value.so.fields[1] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_sec});
        toReturn->value.so.fields[2] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_min});
        toReturn->value.so.fields[3] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_hour});
        toReturn->value.so.fields[4] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_mday});
        toReturn->value.so.fields[5] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_mon+1});
        toReturn->value.so.fields[6] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_year+1900});
        toReturn->value.so.fields[7] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_wday});
        toReturn->value.so.fields[8] = numToTV((num){.type = NUM_INT, .value.iVal = utc->tm_yday});
        toReturn->value.so.fields[9] = numToTV((num){.type = NUM_BOOL, .value.iVal = utc->tm_isdst != 0});
        for (int i = 0; i < 9; i++) {
            stackPtr* sp = malloc(sizeof(stackPtr));
            sp->addr = toReturn;
            sp->isTV = true;
            sp->idx = i;
            toReturn->value.so.fields[i]->ptr = sp;
        }
        List_InsertElement(vms->stack, 0, toReturn);
    }
    if (strcmp(identifier, "sleep") == 0) {
        typedValue* arg0 = List_GetElement(args, 0)->data;
        if (arg0->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        int64_t ms = convertNum(arg0->value.numberValue, NUM_LONG).value.lVal;
        if (ms < 0) fatalError(0x30, "", -1);
        int64_t initMS = getMS();
        while (getMS() < ms+initMS) {}
        free(arg0);
    }
}