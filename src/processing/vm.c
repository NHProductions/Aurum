/*
--- vm.c ---
After having converted the AST into bytecode, this actually runs the code.

Files to check next: sysFunctions.c 
*/

// Standard library imports
#include <limits.h>
#include <windows.h>
#include <direct.h>
// Aurum imports
#include "lexer.h"
#include "parser.h"
#include "../dataStorage/Tundora.h"
#include "vm.h"
#include "bytecoder.h"
#include "../systemLibraries/sysFunctions.h"
#include "../misc/winInclude.h"
#include "../dataStorage/pool.h"
/*
Copies a typedValue by continuously recursing.
*/
void freeTypedValue(typedValue* tv);
pool* globalPool = NULL;
bool isRunning = false;
int windowsOpen = 0;
typedValue* deepcopyTypedValue(typedValue* tv) {
    typedValue* toReturn = poolAlloc(globalPool);
    if (toReturn == NULL) {printf("Memory allocation error"); exit(1);}
    *toReturn = (typedValue){
        .ptr = NULL,
        .valueType = tv->valueType,
        .value.numberValue = (num){.type = NUM_BOOL, .value.bVal = false}
    };
    if (tv->ptr != NULL) {
        toReturn->ptr = malloc(sizeof(stackPtr));
        *(stackPtr*)toReturn->ptr = (stackPtr){
            .addr = ((stackPtr*)tv->ptr)->addr,
            .isTV = true,
            .idx = ((stackPtr*)tv->ptr)->idx
        };
    }
    else toReturn->ptr = NULL;

    switch (tv->valueType) {
        case TYPE_NUM: {
            toReturn->value.numberValue = (num){
                .type = tv->value.numberValue.type,
                .value = tv->value.numberValue.value
            };
            return toReturn;
            break;
        }
        case TYPE_ARRAY: {
            toReturn->value.av = (arrayValue){
                .arrayType = tv->value.av.arrayType,
                .len = tv->value.av.len,
                .data = NULL
            };
            toReturn->value.av.data = malloc(sizeof(typedValue*)*toReturn->value.av.len);
            
            for (int i = 0; i < toReturn->value.av.len; i++) {
                typedValue* toCpy = (tv->value.av.data)[i];
                (toReturn->value.av.data)[i] = deepcopyTypedValue(toCpy);
            }
            return toReturn;
            break;
        }
        case TYPE_STRUCT: {
            toReturn->value.so.def = tv->value.so.def; // definitions can remain.
            int fieldCount = tv->value.so.def->fields->length;
            typedValue** srcFields = (typedValue**)tv->value.so.fields;
            typedValue** dstFields = malloc(sizeof(typedValue*) * fieldCount);
            for (int i = 0; i < fieldCount; i++) {
                dstFields[i] = deepcopyTypedValue(srcFields[i]);
            }
            toReturn->value.so.fields = dstFields;
            return toReturn;
            break;
        }
    }
}
char* charToStr(char n) {
    char* toReturn = malloc(2);
    sprintf(toReturn, "%c", n);
    return toReturn;
}
void printStack(virtualMachineState* vms);
// Does binary/unary operation using num.c.
void doVMSOp(virtualMachineState* vms, char* op, bool isBinary) {

    if (vms->stack->length == 0 || (isBinary && vms->stack->length == 1) ) {
        if (isDebug) printf("Stack illegal length for addition!");
        if (isDebug) printStack(vms);
        fatalError(0x50, "", -1);
    }
    // Grabs A & B.
    int AType = TYPE_NULL;
    int BType = TYPE_NULL;
    typedValue* B = NULL;
    typedValue* A = NULL;
    B = (typedValue*)popArray(vms->stack);
    BType = B->valueType;
    if (isBinary) { A = (typedValue*)popArray(vms->stack); AType = A->valueType;}


    // If all inputs are numbers, do the operation & push it into the stack.
    if ( (AType == TYPE_NUM || !isBinary) && BType == TYPE_NUM) {
        long double bld = convertNum(B->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
        if (bld == 0 && strcmp(op, "/") == 0) {
            fatalError(0x16, "Cannot divide by zero.", -1);
        }
        typedValue* toAdd = malloc(sizeof(typedValue));
        *toAdd = (typedValue){
            .value.numberValue = (num){.value.bVal = 0, .value = NUM_BOOL},
            .valueType = TYPE_NUM,
            .ptr = NULL,
        };
        toAdd->value.numberValue = isBinary ? doBinaryOperation(op, A->value.numberValue, B->value.numberValue) : doUnaryOperation(op, B->value.numberValue);
        if (A != NULL) freeTypedValue(A);
        freeTypedValue(B); // todo: implement custom freeTypedValue() function.
        pushArray(vms->stack, toAdd);
    }
    // String equality
    else if (strcmp(op, "==") == 0 && AType == TYPE_ARRAY && BType == TYPE_ARRAY) {
        if (A->value.av.arrayType == AT_CHARARR && B->value.av.arrayType == AT_CHARARR) {
            char* Astr = strArrToChar(A);
            char* Bstr = strArrToChar(B);
            bool equals = strcmp(Astr, Bstr) == 0 ? true : false;
            pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value.bVal = equals}));
            free(Astr);
            free(Bstr);
            freeTypedValue(A);
            freeTypedValue(B);
        }
    }
    // String concat
    else if (strcmp(op, "+") == 0 && AType == TYPE_ARRAY && (BType == TYPE_ARRAY || BType == TYPE_NUM)) {
        if (A->value.av.arrayType == AT_CHARARR) {
            char* Astr = strArrToChar(A);
            char* Bstr = BType == TYPE_NUM ? charToStr(B->value.numberValue.value.cVal) : strArrToChar(B);
            char* toAppend = malloc(strlen(Astr)+strlen(Bstr)+1);
            sprintf(toAppend, "%s%s", Astr, Bstr);
            
            typedValue* toApp = newTVArray(0, AT_CHARARR);
            for (int i = 0; i < strlen(toAppend); i++) {
                appendTypedValue(toApp, numToTV((num){.type = NUM_CHAR, .value.cVal = toAppend[i]}));
            }
            appendTypedValue(toApp, numToTV((num){.type = NUM_CHAR, .value.cVal = 0}));
            pushArray(vms->stack, toApp);
            free(toAppend);
            free(Astr);
            free(Bstr);
            freeTypedValue(A);
            freeTypedValue(B);
        }
    }
}
// Executes lines.
void executeLine(virtualMachineState* vms, int chunkIdx, int* line, array* locals, bool* returnLn) {

    chunk* currentChunk = (chunk*)getArray(vms->bc->chunks, chunkIdx);
    instruction* currentInstruction = (instruction*)getArray(currentChunk->instructions, *line);
    //printStack(vms);
    switch (currentInstruction->code) {
        // Pushes constants[args[0]] into the stack.
        case OP_LOAD_CONST: {
            typedValue* tocpy = getArray(vms->bc->constants, currentInstruction->args[0]);
            typedValue* tva = deepcopyTypedValue(tocpy);
            if (tva->valueType == TYPE_NULL) {
                if (isDebug) printf("Null"); 
                fatalError(0x30, "", -1);
            }
            pushArray(vms->stack, tva);
            break;
        }
        // All of the arithmetic operations just call doVMSOp(), which pops the values from the stack & does the operation.
        case OP_ADD: {doVMSOp(vms, "+", true);break;}
        case OP_SUB: {doVMSOp(vms, "-", true);break;}
        case OP_MUL: {doVMSOp(vms, "*", true);break;}
        case OP_MOD: {doVMSOp(vms, "%", true);break;}
        case OP_BXOR: {doVMSOp(vms, "^", true);break;}
        case OP_BAND: {doVMSOp(vms, "&", true);break;}
        case OP_BOR: {doVMSOp(vms, "|", true);break;}
        case OP_LSHIFT: {doVMSOp(vms, "<<", true);break;}
        case OP_RSHIFT: {doVMSOp(vms, ">>", true);break;}
        case OP_GTHAN: {doVMSOp(vms, ">", true);break;}
        case OP_GEQTHAN: {doVMSOp(vms, ">=", true);break;}
        case OP_LTHAN: {doVMSOp(vms, "<", true);break;}
        case OP_LEQTHAN: {doVMSOp(vms, "<=", true);break;}
        case OP_EQUALS: {doVMSOp(vms, "==", true);break;}
        case OP_XOR: {doVMSOp(vms, "^^", true);break;}
        case OP_AND: {doVMSOp(vms, "&&", true);break;}
        case OP_OR: {doVMSOp(vms, "||", true);break;}
        case OP_DIV: {doVMSOp(vms, "/", true);break;}
        case OP_BNOT: {doVMSOp(vms, "~", false);break;}
        case OP_NOT: {doVMSOp(vms, "!", false);break;}
        case OP_NEG: {doVMSOp(vms, "-", false);break;}
        // Loads globals[args[0]] onto the stack.
        case OP_LOAD_GLOBAL: {
            stackVariable* sv = (stackVariable*)getArray(vms->globals, currentInstruction->args[0]);
            typedValue* A = deepcopyTypedValue(sv->value);
            A->ptr = malloc(sizeof(stackPtr));
            *((stackPtr*)A->ptr) = (stackPtr){
                .addr = sv,
                .isTV = false,
                .idx = -1,
            };
            pushArray(vms->stack, A);
            break;
        }
        // Pops a value from the stack and stores it into globals[args[0]].
        case OP_STORE_GLOBAL: {
            // [2] [var:Undefined] -> [] [var:2]
            typedValue* A = ((typedValue*)popArray(vms->stack));
            stackVariable* toSet = (stackVariable*)getArray(vms->globals, currentInstruction->args[0]);
            freeTypedValue(toSet->value);
            toSet->value = deepcopyTypedValue(A);
            freeTypedValue(A);
        
            break;
        }
        
        // Pops a value from the stack and stores it into locals[args[0]].
        case OP_STORE_LOCAL: {
            typedValue* A = ((typedValue*)popArray(vms->stack));
            stackVariable* toSet = NULL;
            // If it already exists, then set the value of toSet to the variable that needs set. If not, add it to locals.:
            if (currentInstruction->args[0] < locals->length) toSet = (stackVariable*)getArray(locals, currentInstruction->args[0]);
            if (currentInstruction->args[0] >= locals->length-1) {
                while (currentInstruction->args[0] >= locals->length-1) {
                    stackVariable* sv = malloc(sizeof(stackVariable));
                    *sv = (stackVariable){
                        .isArray = false,
                        .structType = NULL,
                        .value = NULL
                    };
                    appendArray(locals, sv);
                }
            }
            if (toSet == NULL) {
                toSet = malloc(sizeof(stackVariable));
                *toSet = (stackVariable){
                    .isArray = A->valueType == TYPE_ARRAY,
                    .structType = "",
                    .t = A->valueType,
                    .value = NULL,
                };
                toSet->structType = A->valueType == TYPE_STRUCT ? A->value.so.def->name : strdup("");
                freeTypedValue(((stackVariable*)getArray(locals, currentInstruction->args[0]))->value);
                replaceArray(locals, currentInstruction->args[0], toSet);
                
            }
            if (toSet->value != NULL) {
                freeTypedValue(toSet->value);
            }
            toSet->value = deepcopyTypedValue(A);
            freeTypedValue(A);
            break;
        }
        // Loads locals[args[0]] onto the stack.
        case OP_LOAD_LOCAL: {
            stackVariable* sv = (stackVariable*)getArray(locals, currentInstruction->args[0]);
            typedValue* A = deepcopyTypedValue(sv->value);
            A->ptr = malloc(sizeof(stackPtr));
            *((stackPtr*)A->ptr) = (stackPtr){
                .addr = sv,
                .isTV = false,
                .idx = -1
            };
            pushArray(vms->stack, A);
            break;
        }
        // Jumps to line args[0].
        case OP_JUMP: {
            *line = currentInstruction->args[0];
            break;
        }
        // Pops a value from the stack. If it's zero, jump to line args[0].
        case OP_JUMP_IF_FALSE: {
            typedValue* A = (typedValue*)popArray(vms->stack);
            if (A->valueType == TYPE_NUM) {
                long double ld = convertNum(A->value.numberValue, NUM_LONGDOUBLE).value.ldVal;
                if (ld == 0) {
                    *line = currentInstruction->args[0];
                }
            }
            freeTypedValue(A);
            break;
        }
        // Exits the program (not really used lol)
        case OP_HALT: {
            exit(0);
            break;
        }
        /*
        Pops args[1] values from the stack
        If the functions[args[0]] is a system function, use systemCall(), which calls the C implementation for that function.
        If it's not, then iterate through the chunk index given, executing the lines.
        */
        case OP_FPTR: {
            short toMake = (short)currentInstruction->args[0];
            typedValue* toApp = poolAlloc(globalPool);
            *toApp = (typedValue){
                .ptr = NULL,
                .value.so = (struct structObject){
                    .def = getStructDefViaName("fnptr", *vms->bc),
                    .fields = malloc(sizeof(typedValue*))  
                },
                .valueType = TYPE_STRUCT
            };
            toApp->value.so.fields[0] = numToTV((num){.type = NUM_SHORT, .value.sVal = toMake});
            pushArray(vms->stack, toApp);
            break;
        }
        case OP_DCALL: {
            typedValue* A = ((typedValue*)popArray(vms->stack));
            if (A->valueType != TYPE_STRUCT) {fatalError(0x30, "Unable to do a dynamic call with a non-callable object", -1);}
            short toCall = A->value.so.fields[0]->value.numberValue.value.sVal;
            bcFunction* bcf = (bcFunction*)getArray(vms->bc->functionIdentifiers, toCall);
            if (bcf->isSystem) {
                systemCall(bcf, vms, currentInstruction->args[0], locals);
            }
            else {
                array* toSend = mallocArray(0);
                for (int i = 0; i < currentInstruction->args[0]; i++) {
                    stackVariable* sv = malloc(sizeof(stackVariable));
                    typedValue* tv = popArray(vms->stack);
                    *sv = (stackVariable){
                        .isArray = tv->valueType == TYPE_ARRAY,
                        .name = strdup(""),
                        .structType = tv->valueType == TYPE_STRUCT ? tv->value.so.def->name : strdup(""),
                        .t = tv->valueType,
                        .value = tv
                    };
                    insertArray(toSend, 0, sv);
                }
                chunk* toCall = NULL;
                int cidx;
                for (int i = 0; i < vms->bc->chunks->length; i++) {
                    chunk* c = getArray(vms->bc->chunks, i);
                    if (strcmp(c->name, bcf->name) == 0) {
                        toCall = c;
                        cidx = i;
                        break;
                    }
                }
                if (toCall == NULL) {fatalError(0x2E, "Unknown function", -1);}
                bool doBreak = false;
                for (int i = 0; i < toCall->instructions->length && !doBreak; i++) {
                    int prevIVal = i; 
                    executeLine(vms, cidx, &i, toSend, &doBreak);
                    if (i != prevIVal) {i--;}
                }

            }
            
            freeTypedValue(A);
            break;


        }
        case OP_CALL: {
            bcFunction* bcf = (bcFunction*)getArray(vms->bc->functionIdentifiers, currentInstruction->args[0]);
            if (bcf->isSystem) {
                systemCall(bcf, vms, currentInstruction->args[1], locals);
            }
            else {
                array* toSend = mallocArray(0);
                for (int i = 0; i < currentInstruction->args[1]; i++) {
                    stackVariable* sv = malloc(sizeof(stackVariable));
                    typedValue* tv = popArray(vms->stack);
                    *sv = (stackVariable){
                        .isArray = tv->valueType == TYPE_ARRAY,
                        .name = strdup(""),
                        .structType = tv->valueType == TYPE_STRUCT ? tv->value.so.def->name : strdup(""),
                        .t = tv->valueType,
                        .value = tv
                    };
                    insertArray(toSend, 0, sv);
                }
                chunk* toCall = NULL;
                int cidx;
                for (int i = 0; i < vms->bc->chunks->length; i++) {
                    chunk* c = getArray(vms->bc->chunks, i);
                    if (strcmp(c->name, bcf->name) == 0) {
                        toCall = c;
                        cidx = i;
                        break;
                    }
                }
                if (toCall == NULL) {fatalError(0x2E, "Unknown function", -1);}
                bool doBreak = false;
                for (int i = 0; i < toCall->instructions->length && !doBreak; i++) {
                    int prevIVal = i; 
                    executeLine(vms, cidx, &i, toSend, &doBreak);
                    if (i != prevIVal) {i--;}
                }

            }
            break;
        }
        // sets the return flag to true.
        case OP_RETURN: {
            if (chunkIdx == 1) *line = -1;
            *returnLn = true; // early break
            break;
        }
        // Pops args[0] values from the stack, and uses it to build an array. Pushes that array onto the stack.
        case OP_BUILD_ARRAY: {
            int amt = currentInstruction->args[0];
            typedValue* toAppend = malloc(sizeof(typedValue));
            *toAppend = (typedValue){
                .valueType = TYPE_ARRAY,
                .value.av = (arrayValue){
                    .len = amt,
                    .data = malloc(amt*sizeof(typedValue*)),
                    .arrayType = AT_UNKNOWN
                }
            };
            for (int i = amt; i > 0; i-- ) {
                typedValue* ta = popArray(vms->stack);
                if (ta->valueType == TYPE_NUM) {
                    toAppend->value.av.arrayType = AT_NUM;
                }
                toAppend->value.av.data[i-1] = ta;
            }
            pushArray(vms->stack, toAppend);
            break;
        }
        // # operator; returns the length of an array (or string)
        case OP_ARRAY_LEN: {
            typedValue* tv = malloc(sizeof(typedValue));
            typedValue* arr = popArray(vms->stack);
            
            *tv = (typedValue){
                .valueType = TYPE_NUM,
                .value.numberValue = (num){
                    .type = NUM_UINT,
                    .value.uiVal = arr->value.av.len
                }
            };
            freeTypedValue(arr);
            pushArray(vms->stack, tv);
            break;
        }
        // Pops two values from the stack. The 1st value popped is the index, and 2nd value is the array.
        // Load arr[idx] onto the stack. Free idx & arr.
        case OP_LOAD_IDX: {
            typedValue* idx = popArray(vms->stack);
            typedValue* arr = popArray(vms->stack);
            if (idx->valueType != TYPE_NUM || arr->valueType != TYPE_ARRAY) {fatalError(0x22, "Invalid Array access", -1);}
            uint64_t cc = convertNum(idx->value.numberValue, NUM_ULONG).value.ulVal;
            if (cc >= arr->value.av.len) fatalError(0x22, "Invalid array access", -1);
            typedValue* toCpy = (arr->value.av.data)[cc];
            typedValue* toApp = deepcopyTypedValue(toCpy);
            if (toCpy->valueType == TYPE_NULL) {
                if (isDebug) printStack(vms);
                fatalError(0x22, "Invalid array access (toCpy is null?)", -1);}
            if (toApp->ptr != NULL) free(toApp->ptr);
            toApp->ptr = malloc(sizeof(stackPtr));
            stackPtr* sp = (stackPtr*)arr->ptr;
            if (!sp) {

            }
            else if (!sp->isTV) {
                stackVariable* sv = (stackVariable*)sp->addr;
                *((stackPtr*)toApp->ptr) = (stackPtr){
                    .addr = sv,
                    .isTV = false,
                    .idx = cc
                };
            }
            else {
                typedValue* tv = ((typedValue*)sp->addr)->value.av.data[cc];
                *((stackPtr*)toApp->ptr) = (stackPtr){
                    .addr = tv,
                    .isTV = tv->valueType == TYPE_ARRAY,
                    .idx = cc
                };
            }
            pushArray(vms->stack, toApp);
            freeTypedValue(idx);
            freeTypedValue(arr);
            break;
        }
        // Pops three values from the stack. The 1st is the value to store, 2nd is the index, 3rd is the array.
        // Stores toStore into array[idx]
        case OP_STORE_IDX: {
            typedValue* toStore = popArray(vms->stack);
            typedValue* idx = popArray(vms->stack);
            typedValue* array = popArray(vms->stack);
            
            if (idx->valueType != TYPE_NUM) {fatalError(0x2F, "Invalid index assignment", -1);}
            if (array->ptr == NULL) {if (isDebug) {printf("ARRAY NULL");} break;};
            
            stackPtr* sp = (stackPtr*)array->ptr;
            uint64_t ullidx = convertNum(idx->value.numberValue, NUM_ULONG).value.ulVal;
            typedValue* targetContainer = NULL;
            if (!sp->isTV) {
                stackVariable* memLoc = (stackVariable*)sp->addr;
                targetContainer = sp->idx >= 0 ? ((typedValue**)memLoc->value->value.av.data)[sp->idx] : memLoc->value;
            }
            else {
                typedValue* memLoc = (typedValue*)sp->addr;
                targetContainer = sp->idx >= 0 ? ((typedValue**)memLoc->value.av.data)[sp->idx] : memLoc;
            }
            if (targetContainer == NULL || targetContainer->valueType != TYPE_ARRAY) {fatalError(0x2F, "Invalid index assignment", -1);}
            ((typedValue**)targetContainer->value.av.data)[ullidx] = toStore;
            freeTypedValue(idx);
            freeTypedValue(array);
            break;
        }
        // Allocates a new struct
        case OP_NEW_STRUCT: {
            structDefinition* sd = getArray(vms->bc->structDefs, currentInstruction->args[0]);
            typedValue* toSet = malloc(sizeof(typedValue));
            *toSet = (typedValue){
                .ptr = NULL,
                .valueType = TYPE_STRUCT,
                .value.so = (struct structObject){
                    .def = sd,
                    .fields = malloc(sd->fields->length*sizeof(typedValue*))
                }
            };
            for (int i = 0; i < toSet->value.so.def->fields->length; i++) {
                typedValue* f = malloc(sizeof(typedValue));
                *f = (typedValue){
                    .ptr = NULL,
                    .value.numberValue = (num){.type = NUM_CHAR, .value.cVal = 0},
                    .valueType = TYPE_NUM
                };
                (toSet->value.so.fields)[i] = f;
            }
            pushArray(vms->stack, toSet);
            break;
        }
        // Pops two values from the stack. The 1st is the value to store, and the 2nd is the struct to store it in.
        // sets struct.args[0] to val.
        case OP_SET_FIELD: {
            typedValue* val = popArray(vms->stack);
            typedValue* structSet = popArray(vms->stack);

            if (structSet->ptr == NULL) {
                //free(((typedValue**)structSet->value.so.fields)[currentInstruction->args[0]]);
                typedValue** n = (typedValue**)structSet->value.so.fields;
                n[currentInstruction->args[0]] = deepcopyTypedValue(val); 
            }
            else {
                stackPtr* sp = (stackPtr*)structSet->ptr;
                if (!sp->isTV) {
                    stackVariable* memLoc = (stackVariable*)sp->addr;
                    //free(((typedValue**)memLoc->value->value.so.fields)[currentInstruction->args[0]]);
                    ((typedValue**)memLoc->value->value.so.fields)[currentInstruction->args[0]] = deepcopyTypedValue(val);
                    //free(array);
                    freeTypedValue(val);
                    freeTypedValue(structSet);
                }
                else {
                    typedValue* memLoc = (typedValue*)sp->addr;
                    //free(((typedValue**)memLoc->value.so.fields)[currentInstruction->args[0]]);
                    ((typedValue**)memLoc->value.so.fields)[currentInstruction->args[0]] = deepcopyTypedValue(val);
                    //free(array);
                    freeTypedValue(val);
                    freeTypedValue(structSet);
                }
            }
            break;
        }
        // Pops one value from the stack, and pushes tv.args[0].
        case OP_GET_FIELD: {
            typedValue* tv = popArray(vms->stack);
            typedValue* toReturn = deepcopyTypedValue(((typedValue**)tv->value.so.fields)[currentInstruction->args[0]]);
            toReturn->ptr = malloc(sizeof(stackPtr));
            if (tv->ptr == NULL) {
                *(stackPtr*)(toReturn->ptr) = (stackPtr){
                    .addr = ((typedValue**)tv->value.so.fields)[currentInstruction->args[0]],
                    .idx = -2,
                    .isTV = true
                };
            }
            else {
                stackPtr* tvptr = (stackPtr*)tv->ptr;
                if (tvptr->isTV) *(stackPtr*)(toReturn->ptr) = (stackPtr){
                    .addr = ((typedValue**)((typedValue*)tvptr->addr)->value.so.fields)[currentInstruction->args[0]],
                    .isTV = true,
                    .idx = -1
                };
                else *(stackPtr*)(toReturn->ptr) = (stackPtr){
                    .addr = ((typedValue**)((stackVariable*)tvptr->addr)->value->value.so.fields)[currentInstruction->args[0]],
                    .isTV = true,
                    .idx = -1
                };
            }
            
            pushArray(vms->stack, toReturn);
            freeTypedValue(tv);
            break;
        }
    }
    
}
// Prints the stack
void printStack(virtualMachineState* vms) {
    printf("\n[");
    for (int i = 0; i < vms->stack->length; i++) {
        typedValue* tv = (typedValue*)getArray(vms->stack, i);
        if (tv->valueType == TYPE_NUM) {
            if (tv->value.numberValue.type == NUM_CHAR) {
                printf("%d(%c)", tv->value.numberValue.value.cVal, tv->value.numberValue.value.cVal);
            }
            else if (tv->value.numberValue.type >= NUM_FLOAT) {
                printf("%Lf", convertNum(tv->value.numberValue, NUM_LONGDOUBLE).value.ldVal);
            }
            else if (tv->value.numberValue.type != NUM_BOOL) {
                int64_t n = convertNum(tv->value.numberValue, NUM_LONG).value.lVal;
                printf("%lld", n);
            }
            else {
                printf("%d", tv->value.numberValue.value.bVal);
            }
            if (i != vms->stack->length-1) printf(", ");
        }
        else if (tv->valueType == TYPE_ARRAY) {
            if (tv->value.av.arrayType != AT_UNKNOWN) {
                printf("[");
                for (int j = 0; j < tv->value.av.len; j++) {
                    typedValue* jn = ((typedValue**)tv->value.av.data)[j];
                    if (jn == NULL) continue;
                    if (jn->value.numberValue.type == NUM_CHAR) {
                        printf("%c", jn->value.numberValue.value.cVal);
                    }
                    else if (jn->value.numberValue.type != NUM_BOOL) {
                        printf("%Lf, ", convertNum(jn->value.numberValue, NUM_LONGDOUBLE).value.ldVal);
                    }
                    else {
                        printf(jn->value.numberValue.value.bVal ? "True, " : "False,");
                    }
                }
                printf("], ");
            }
            else {
                printf("[ARRAY %d]", tv->value.av.len);
            }
        }
        if (i == vms->stack->length-1) printf("]\n");
    }
    if (vms->stack->length == 0) printf("]\n");
}
// Finds a function's chunkidx.
int findFuncChunkIdx(byteCode* bc, char* name) {
    for (int i = 0; i < bc->chunks->length; i++) {
        chunk* c = (chunk*)getArray(bc->chunks, i);
        if (strcmp(c->name, name) == 0) {
            return i;
        }
    }
    if (isDebug) printf("\nCan't find function %s", name);
    return -1;
}
void freeTVArray(arrayValue av);
void freeTypedValue(typedValue* tv) {
    
    if (tv == NULL) return;

    if (tv->valueType == TYPE_ARRAY) {
        for (int i = 0; i < tv->value.av.len; i++) {
            freeTypedValue(tv->value.av.data[i]);
        }
        free(tv->value.av.data);
    }
    else if (tv->valueType == TYPE_STRUCT) {
        for (int i = 0; i < tv->value.so.def->fields->length; i++) {
            freeTypedValue(tv->value.so.fields[i]);
        }
        free(tv->value.so.fields); 
    }
    poolFree(globalPool, tv);
}
void freeTVArray(arrayValue av) {
    for (int i = 0; i < av.len; i++) {
        freeTypedValue(av.data[i]);
    }
    free(av.data);
}
// Executes the bytecode
void executeBytecode(byteCode* bc) {
    chdir(projectFolder);
    // Initializes the virtualMachineState.
    virtualMachineState* vms = malloc(sizeof(virtualMachineState));
    *vms = (virtualMachineState){
        .globals = mallocArray(0),
        .stack = mallocArray(0),
        .bc = bc
    };
    // adds bc.globals to vms.globals.
    for (int i = 0; i < vms->bc->globals->length; i++) {
        
        vmVariable* toCpy = (vmVariable*)getArray(vms->bc->globals, i);
        stackVariable* sv = malloc(sizeof(stackVariable));
        *sv = (stackVariable){
            .isArray = toCpy->isArray,
            .name = toCpy->name,
            .structType = toCpy->structType,
            .t = toCpy->t,
            .value = malloc(sizeof(typedValue))
        };
        *sv->value = (typedValue){.valueType = TYPE_NUM, .value.numberValue.value.bVal = 0, .ptr = NULL};
        appendArray(vms->globals, sv);
    }
    // Executes premain
    isRunning = true;
    int premainIdx = findFuncChunkIdx(bc, "premain");
    chunk* premainChunk = (chunk*)getArray(bc->chunks, premainIdx);
    array* locals = mallocArray(0);
    bool ptr = false;
    for (int i = 0; i < premainChunk->instructions->length; i++) {
        executeLine(vms, premainIdx, &i, locals, &ptr);
    } 
    // executes main
    int mainIdx = findFuncChunkIdx(bc, "main");
    chunk* mainChunk = (chunk*)getArray(bc->chunks, mainIdx);
    for (int i = 0; i < mainChunk->instructions->length; i++) {
        int prevIVal = i;
        executeLine(vms, mainIdx, &i, locals, &ptr);
        if (i < 0) break;
        if (i != prevIVal) {i--;}
        
    } 
    isRunning = true;
    if (isDebug) printStack(vms);
    if (isDebug) {printf("\n%d/%d", globalPool->currentTotal, globalPool->capacity/sizeof(typedValue));}
}
