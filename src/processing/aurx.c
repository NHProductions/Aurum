/*
--- AurX.c ---
This file is responsible for translating bytecode to and from the AurX format, which is then saved as a file.

*/

#include "bytecoder.h"
#include "vm.h"
#include "aurx.h"
#include <io.h>
#include "../misc/winInclude.h"
#include "../systemLibraries/sysFunctions.h"
// Reads a byte and returns it as a bool.
bool readBool(FILE* f) {
    bool toReturn;
    fread(&toReturn, 1, 1, f);
    return toReturn;
}
// Reads two bytes and returns it as a short.
short readShort(FILE* f) {
    short toReturn;
    fread(&toReturn, 2, 1, f);
    return toReturn;
}
// Reads 4 bytes and returns it as an int
int readInt(FILE* f) {
    int toReturn;
    fread(&toReturn, sizeof(int), 1, f);
    return toReturn;
}
// Reads 1 byte and returns it as a char.
char readChar(FILE* f) {
    char toReturn;
    fread(&toReturn, sizeof(char), 1, f);
    return toReturn;
}
// Reads several bytes and returns them as a string. The 1st byte(s) is the length of the string.
char* readString(FILE* f, int size) {
    int n;
    if (size == sizeof(char)) n = readChar(f);
    else if (size == sizeof(short)) n = readShort(f);
    else if (size == sizeof(int)) n = readInt(f);

    char* toReturn = malloc(n+1);
    fread(toReturn, sizeof(char), n, f);
    toReturn[n] = '\0';
    return toReturn;
}
/*
BcFunction format:
[Char: Length of name][char* name]
[char: argc][isSystem]
[Char: Length of returnStruct. If 0, it's NULL][Return struct; if returnStruct is NULL, then this is just another 0]
*/
void writeBCFunction(FILE* f, bcFunction* bcf) {
    // Adds length
    putc(strlen(bcf->name), f);
    // Adds the name
    int len = strlen(bcf->name);
    fputs(bcf->name, f);
    // Adds argc
    putc(bcf->argc, f);
    putc(bcf->isSystem, f);
    if (bcf->returnStruct == NULL) {putc(0, f); return;}
    putc(strlen(bcf->returnStruct), f);
    fputs(bcf->returnStruct, f);
}
void readBCFuncs(byteCode* c, FILE* f) {
    short n;
    fread(&n, sizeof(short), 1, f);
    for (int i = 0; i < n; i++) {
        bcFunction* toAppend = malloc(sizeof(bcFunction));
        
        char* name = readString(f, sizeof(char));
        toAppend->name = name;

        char argc = readChar(f);
        toAppend->argc = argc;

        bool isSystem = readBool(f);
        toAppend->isSystem = isSystem;
        char* returnStruct = readString(f, sizeof(char));
        if (strlen(returnStruct) == 0) {
            free(returnStruct);
            toAppend->returnStruct = NULL;
        }
        else toAppend->returnStruct = returnStruct;

        appendArray(c->functionIdentifiers, toAppend);
    }
}
/*
[Char: length of name][char* name]
[char type][isArray, isConst]
[char: Length of return struct. If 0, it's NULL]
[char: return struct; if it's NULL, then this is 0]
*/
void writeVMV(FILE* f, vmVariable* vmv) {
    putc(strlen(vmv->name), f);
    fputs(vmv->name, f);
    putc(vmv->t, f);
    putc(vmv->isArray, f);
    putc(vmv->isConst, f);
    if (vmv->structType == NULL) {putc(0, f); return;}
    putc(strlen(vmv->structType), f);
    fputs(vmv->structType, f);
}
void readVMV(byteCode* c, FILE* f) {
    short amt = readShort(f);
    for (int i = 0; i < amt; i++) {
        vmVariable* vmv = malloc(sizeof(vmVariable));
        char* name = readString(f, sizeof(char));
        vmv->name = name;
        vmv->t = readChar(f);
        vmv->isArray = readBool(f);
        vmv->isConst = readBool(f);
        char* structType = readString(f, sizeof(char));
        if (strlen(structType) == 0) {
            free(structType);
            vmv->structType = NULL;
        } else vmv->structType = structType;
        appendArray(c->globals, vmv);
    }
}
/*
Num: [type][num]
Array: [char: type][char: arrayType][int: length][... typedValues]
*/
void writeTV(FILE* f, typedValue* tv) {
    if (tv->valueType == TYPE_NUM) {
        putc(TYPE_NUM, f);
        fwrite(&tv->value.numberValue, sizeof(num), 1, f);
    }
    else if (tv->valueType == TYPE_ARRAY) {
        putc(TYPE_ARRAY, f);
        putc(tv->value.av.arrayType, f);
        fwrite(&tv->value.av.len, sizeof(int), 1, f);
        for (int i = 0; i < tv->value.av.len; i++) {
            writeTV(f, (tv->value.av.data)[i]);
        }
    }
}
typedValue* readTV(FILE* f) {
    typedValue* tv = malloc(sizeof(typedValue));
    tv->ptr = NULL;
    tv->valueType = readChar(f);
    if (tv->valueType == TYPE_NUM) {
        num n;
        fread(&n, sizeof(num), 1, f);
        tv->value.numberValue = n;
    }
    else if (tv->valueType == TYPE_ARRAY) {
        char type = readChar(f);
        int len = readInt(f);
        tv->value.av = (arrayValue){
            .arrayType = type,
            .len = len,
            .data = malloc(len <= 0 ? 1 : len*sizeof(typedValue*)),
        };
        for (int i = 0; i < tv->value.av.len; i++) {
            tv->value.av.data[i] = readTV(f);
        }
    }
    return tv;
}
void readAllTV(byteCode* c, FILE* f) {
    int amt = readInt(f);
    for (int i = 0; i < amt; i++) {
        typedValue* valueRead = readTV(f);
        appendArray(c->constants, valueRead);
    }
}
/*
structDefinitions:
structType format:
Struct: [1 bit: isPDT, isArray, isConst][char nameLength][char name]
Nonstruct: [1 bit: isPDT, isArray, isConst][char pdtType]
structDef format:
[Amount of definitions][Length of name][name][Amount of fields][... length of field, field chars][structTypes]
*/
void writeStructType(FILE* f, structTypes st) {
    putc(st.isPDT, f);
    putc(st.isConst, f);
    putc(st.isArray, f);
    if (st.isPDT) putc(st.type.pdtType, f);
    else {putc(strlen(st.type.name), f); fputs(st.type.name, f); }
}
void writeSD(FILE* f, structDefinition* sd) {
    putc(strlen(sd->name), f);
    fputs(sd->name, f);
    putc(sd->fields->length, f);
    for (int i = 0; i < sd->fields->length; i++) {
        char* n = ((structField*)getArray(sd->fields, i))->name;
        putc(strlen(n), f);
        fputs(n, f);
    }
    for (int i = 0; i < sd->fields->length; i++) {
        writeStructType(f, *(structTypes*)getArray(sd->fieldTypes, i));
    }
}
void readSD(byteCode* c, FILE* f) {
    short amt = readShort(f);
    for (int i = 0; i < amt; i++) {
        structDefinition* sd = malloc(sizeof(structDefinition));
        sd->name = readString(f, sizeof(char));
        sd->fields = mallocArray(0);
        sd->fieldTypes = mallocArray(0);
        char fieldsAmt = readChar(f);
        for (int j = 0; j < fieldsAmt; j++) {
            structField* toApp = malloc(sizeof(structField));
            toApp->name = readString(f, sizeof(char));
            appendArray(sd->fields, toApp);
        }
        for (int j = 0; j < fieldsAmt; j++) {
            structTypes* st = malloc(sizeof(structTypes));
            *st = (structTypes){
                .isPDT = readBool(f),
                .isConst = readBool(f),
                .isArray = readBool(f)
            };
            if (st->isPDT) st->type.pdtType = readChar(f);
            else st->type.name = readString(f, sizeof(char));
            appendArray(sd->fieldTypes, st);
        }
        appendArray(c->structDefs, sd);
    }
}
/*
[amount of chunks]
[Amount of instructions in chunk]
[char: instruction][short* args]
*/
void writeChunk(FILE* f, chunk* c) {
    fwrite(&c->instructions->length, sizeof(int), 1, f);
    putc(strlen(c->name), f);
    fputs(c->name, f);
    for (int i = 0; i < c->instructions->length; i++) {
        instruction* inst = getArray(c->instructions, i);
        fwrite(&inst->code, sizeof(char), 1, f);
        for (int j = 0; j < inst->argc; j++) {fwrite(&inst->args[j], sizeof(int), 1, f);}
    }
}
void readChunks(byteCode* c, FILE* f) {
    short chunksAmt = readShort(f);
    for (int i = 0; i < chunksAmt; i++) {
        chunk* toAppend = malloc(sizeof(chunk));
        toAppend->instructions = mallocArray(0);
        int instructionsAmt = readInt(f);
        toAppend->name = readString(f, sizeof(char));
        for (int j = 0; j < instructionsAmt; j++) {
            instruction* inst = malloc(sizeof(instruction));
            inst->code = readChar(f);
            if (inst->code >= NONOP_NULLARYSTART && inst->code < NONOP_UNARYSTART) {inst->argc = 0; inst->args = NULL; appendArray(toAppend->instructions, inst); continue;}
            if (inst->code >= NONOP_UNARYSTART && inst->code < NONOP_BINARYSTART) {inst->argc = 1; inst->args = malloc(sizeof(int));}
            else {inst->argc = 2; inst->args = malloc(sizeof(int)*2);}
            
            for (int k = 0; k < inst->argc; k++) {inst->args[k] = readInt(f);}
            appendArray(toAppend->instructions, inst);
        }
        appendArray(c->chunks, toAppend);
    }
}
// Converts bytecode to AurX and saves it into toSaveAs.
void toAurX(byteCode* c, char* toSaveAs) {
    if (isDebug) printBytecode(*c, false, true);
    FILE* f = fopen(toSaveAs, "wb");
    if (f == NULL) {
        printf("Unable to save file");
        exit(1);
    }
    fwrite(&c->functionIdentifiers->length, sizeof(short), 1, f); if (isDebug) printf("%d", c->functionIdentifiers->length);
    for (int i = 0; i < c->functionIdentifiers->length; i++) {
        writeBCFunction(f, getArray(c->functionIdentifiers, i));
    }
    fwrite(&c->globals->length, sizeof(short), 1, f);
    for (int i = 0; i < c->globals->length; i++) {
        writeVMV(f, getArray(c->globals, i));
    }
    fwrite(&c->constants->length, sizeof(int), 1, f);
    for (int i = 0; i < c->constants->length; i++) {
        writeTV(f, getArray(c->constants, i));
    }
    fwrite(&c->structDefs->length, sizeof(short), 1, f);
    for (int i = 0; i < c->structDefs->length; i++) {
        writeSD(f, getArray(c->structDefs, i));
    }
    fwrite(&c->chunks->length, sizeof(short), 1, f);
    for (int i = 0; i < c->chunks->length; i++) {
        writeChunk(f, getArray(c->chunks, i));
    }
    fclose(f);
}
// Reads a file and makes a byteCode out of it.
void fromAurX(bool isValid, char* toRead) {
    if (!isValid) return;
    FILE* f = fopen(toRead, "rb");
    byteCode* bc = malloc(sizeof(byteCode));
    *bc = (byteCode){
        .chunks = mallocArray(0),
        .constants = mallocArray(0),
        .functionIdentifiers = mallocArray(0),
        .globals = mallocArray(0),
        .structDefs = mallocArray(0)
    };
    readBCFuncs(bc, f);
    readVMV(bc, f);
    readAllTV(bc, f);
    readSD(bc, f);
    readChunks(bc, f);
    fclose(f);
    if (isDebug) printBytecode(*bc, false, true);
    executeBytecode(bc);
    exit(1);
}