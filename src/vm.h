#ifndef VM_H
#define VM_H
#include "bytecoder.h"


void freeTypedValue(typedValue* tv);
void freeTVArray(arrayValue av);
typedef struct {
    char* name;
    type t;
    char* structType;
    bool isArray;
    typedValue* value;
} stackVariable;
typedef struct {
    void* addr;
    bool isTV;
    int idx;
} stackPtr;
void executeBytecode(byteCode* bc);
typedValue* deepcopyTypedValue(typedValue* tv);
typedef struct {
    List* stack; // List of typed values (e.x anonymous arrays/numbers/structs ONLY)
    List* globals;
    byteCode* bc;
} virtualMachineState;
#endif