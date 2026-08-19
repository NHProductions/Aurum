/*
--- ioFunctions.c ---
Includes implementations for @io.
*/

#include "ioFunctions.h"
// Enum of valid file modes
typedef enum {
    FM_R=1, // r - Read
    FM_W=2, // w - Overwrites
    FM_A=3, // a - Append (no read)
    FM_RP=4, // r+ - Read/Write
    FM_WP=5, // w+ - Read/Overwrites
    FM_AP=6 // a+ - Read/Append
} fileMode;
// Converts fileMode into a C file mode.
char* fmToStr(fileMode fm) {
    switch (fm) {
        case FM_R: {return "r";}
        case FM_W: {return "w";}
        case FM_A: {return "a";}
        case FM_RP: {return "r+";}
        case FM_WP: {return "w+";}
        case FM_AP: {return "a+";}
    }
    return "";
}
// Implementation for fopen(); Opens a file, and returns it as a file struct (see ioFunctions.h)
void fNewFile(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
    if (arg0->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
    char* pathToOpen = strArrToChar(arg0);

    typedValue* arg1 = getArray(args,  1);
    if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    int mode = convertNum(arg1->value.numberValue, NUM_INT).value.iVal;
    FILE* n = NULL;

    int64_t ptr = 0;
    int64_t pos = 0;
    n = fopen(pathToOpen, fmToStr(mode));
    if (ferror(n)) {
        int code = errno;
        if (code == ENOENT) fatalError(0x42, "No such file or directory", -1);
        if (code == EEXIST) fatalError(0x43, "File already exists", -1);
        if (code == EISDIR) fatalError(0x44, "Path is a directory", -1);
        if (code == ENAMETOOLONG) fatalError(0x45, "File name too long", -1);
        if (code == ELOOP) fatalError(0x46, "Too many symbolic links", -1);
        if (code == EACCES) fatalError(0x47, "Permission denied", -1);
        if (code == EROFS) fatalError(0x48, "Attempted to open a file with writing permission on a read-only file system", -1);
        if (code == ETXTBSY) fatalError(0x49, "Text file busy", -1);
        if (code == EMFILE) fatalError(0x4A, "Too many open files", -1);
        if (code == ENFILE) fatalError(0x4B, "File table overflow (Too many files exist)", -1);
        if (code == ENOMEM) fatalError(0x4C, "Cannot allocate memory to file", -1);
        if (code == ENOSPC) fatalError(0x4D, "No space left on device", -1);
        if (code == EINVAL) fatalError(0x4E, "Invalid mode", -1);
        if (code == EOVERFLOW) fatalError(0x4F, "File too large", -1);
    }

    ptr = (int64_t)n;
    if (n != NULL) pos = ftell(n);
    else pos = 0;
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .valueType = TYPE_STRUCT,
        .ptr = NULL,
        .value.so = (struct structObject){
            .def = getStructDefViaName("file", *vms->bc),
            .fields = malloc(sizeof(typedValue*)*4)
        }
    };
    toReturn->value.so.fields[0] = numToTV((num){.type = NUM_LONG, .value.lVal = ptr});
    toReturn->value.so.fields[1] = numToTV((num){.type = NUM_LONG, .value.lVal = pos});
    toReturn->value.so.fields[2] = numToTV((num){.type = NUM_INT, .value.iVal = mode});
    toReturn->value.so.fields[3] = arg0;
    pushArray(vms->stack, toReturn);
    freeTypedValue(arg1);
    free(pathToOpen);
}
// Implementation for file_close().
void fClose(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp("file", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
    FILE* n = (FILE*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
    fclose(n);
    stackPtr* sp = (stackPtr*)arg0->ptr;
    if (sp->isTV) {
        typedValue* sptv = (typedValue*)sp->addr;
        if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[0]->value.numberValue.value.lVal = 0.0L;
        else sptv->value.numberValue.value.iVal = 0.0L;
    }
    if (!sp->isTV) {
        stackVariable* sptv = (stackVariable*)sp->addr;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[0]->value.numberValue.value.lVal = 0.0L;
        else sptv->value->value.numberValue.value.iVal = 0.0L;
    }
    freeTypedValue(arg0);
}
// Implementation for file_read(), file_getChar(), file_getLine, and file_getStr
void fRead(auFunc) {

    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp(arg0->value.so.def->name, "file") != 0) fatalError(0x30, "", -1);
    FILE* f = (FILE*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
    int64_t newPos = convertNum(arg0->value.so.fields[1]->value.numberValue, NUM_LONG).value.lVal;
    if (strcmp(identifier, "file_getChar") == 0) {
        int c = fgetc(f);
        pushArray(vms->stack, numToTV((num){.type = NUM_INT, .value.iVal = c}));
        if (c != -1) newPos++;
    }
    if (strcmp(identifier, "file_getLine") == 0 || strcmp(identifier, "file_read") == 0 || strcmp(identifier, "file_getStr") == 0 ) {
        char buffer[1024]; buffer[0] = '\0';
        size_t read = fread(buffer, 1, 1023, f);
        buffer[read] = '\0';
        typedValue* toReturn = newTVArray(0, AT_CHARARR);
        int64_t numToRead = INT64_MAX;
        if (strcmp(identifier, "file_getStr") == 0) {
            typedValue* arg1 = getArray(args,  1);
            if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
            numToRead = convertNum(arg1->value.numberValue, NUM_LONG).value.lVal;
            freeTypedValue(arg1);
        }
        for (int i = 0; i < strlen(buffer) && i < numToRead; i++) {
            if (strcmp(identifier, "file_getLine") == 0 && buffer[i] == '\n') break;
            appendTypedValue(toReturn, numToTV((num){.type = NUM_CHAR, .value.cVal = buffer[i]}));
        }
        appendTypedValue(toReturn, numToTV((num){.type = NUM_CHAR, .value.cVal = 0}));
        newPos = newPos+toReturn->value.av.len-1;
        pushArray(vms->stack, toReturn);
    }

    stackPtr* sp = (stackPtr*)arg0->ptr;
    fseek(f, newPos, SEEK_SET);
    if (sp->isTV) {
        typedValue* sptv = (typedValue*)sp->addr;
        if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[1]->value.numberValue.value.lVal = (int64_t)newPos;
        else sptv->value.numberValue.value.iVal = (int64_t)newPos;
    }
    if (!sp->isTV) {
        stackVariable* sptv = (stackVariable*)sp->addr;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[1]->value.numberValue.value.lVal = (int64_t)newPos;
        else sptv->value->value.numberValue.value.iVal = (int64_t)newPos;
    }
    freeTypedValue(arg0);
}
// C implementation for file_writef, file_putChar, & file_putStr()
void fWrite(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp(arg0->value.so.def->name, "file") != 0) fatalError(0x30, "", -1);
    FILE* f = (FILE*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
    int64_t newPos = convertNum(arg0->value.so.fields[1]->value.numberValue, NUM_LONG).value.lVal;
    if (strcmp(identifier, "file_writef") == 0) {
        array* toSendArgs = mallocArray(0);
        for (int i = 1; i < args->length; i++) {
            appendArray(toSendArgs, getArray(args, i));
        }
        char* toWrite = sPrint(vms, "print", toSendArgs, locals, true);
        fwrite(toWrite, 1, strlen(toWrite), f);
        newPos += strlen(toWrite);
        free(toWrite);
    }
    if (strcmp(identifier, "file_putChar") == 0) {
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        char toWrite = convertNum(arg1->value.numberValue, NUM_CHAR).value.cVal;
        fputc(toWrite, f);
        newPos++;
    }
    if (strcmp(identifier, "file_putStr") == 0) {
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg1->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
        char* toWrite = strArrToChar(arg1);
        fputs(toWrite, f);
        newPos += strlen(toWrite);
    } 

    stackPtr* sp = (stackPtr*)arg0->ptr;
    fseek(f, newPos, SEEK_SET);
    if (sp->isTV) {
        typedValue* sptv = (typedValue*)sp->addr;
        if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[1]->value.numberValue.value.lVal = (int64_t)newPos;
        else sptv->value.numberValue.value.iVal = (int64_t)newPos;
    }
    if (!sp->isTV) {
        stackVariable* sptv = (stackVariable*)sp->addr;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[1]->value.numberValue.value.lVal = (int64_t)newPos;
        else sptv->value->value.numberValue.value.iVal = (int64_t)newPos;
    }
    freeTypedValue(arg0);

}
// C implementation for file_size().
void fSize(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp(arg0->value.so.def->name, "file") != 0) fatalError(0x30, "", -1);
    FILE* f = (FILE*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
    int64_t newPos = convertNum(arg0->value.so.fields[1]->value.numberValue, NUM_LONG).value.lVal;
    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    fseek(f, newPos, SEEK_SET);
    pushArray(vms->stack, numToTV((num){.type = NUM_LONG, .value.lVal = size}));
    freeTypedValue(arg0);
}
// C implementation for file_delete & file_rename
void fModify(auFunc) {
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
    if (strcmp(arg0->value.so.def->name, "file") != 0) fatalError(0x30, "", -1);
    FILE* f = (FILE*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
    int64_t newPos = convertNum(arg0->value.so.fields[1]->value.numberValue, NUM_LONG).value.lVal;
    int64_t newPtr = (int64_t)f;
    char* name = strArrToChar(arg0->value.so.fields[3]);
    typedValue* nameTV = arg0->value.so.fields[3];
    if (strcmp(identifier, "file_delete") == 0) {
        fclose(f);
        remove(name);
        free(name);
        newPtr = 0.0L;
    }
    if (strcmp(identifier, "file_rename") == 0) {
        fclose(f);
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg1->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
        char* newName = strArrToChar(arg1);
        rename(name, newName);
        free(name);
        char* mode = fmToStr(convertNum(arg0->value.so.fields[2]->value.numberValue, NUM_LONG).value.lVal);
        f = fopen(newName, mode);
        free(newName);
        nameTV = arg1;
        newPtr = (int64_t)f;
    }

    stackPtr* sp = (stackPtr*)arg0->ptr;
    fseek(f, newPos, SEEK_SET);
    if (sp->isTV) {
            typedValue* sptv = (typedValue*)sp->addr;
            if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[0]->value.numberValue.value.lVal = newPtr;
            if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[1]->value.numberValue.value.lVal = 0;
            if (sptv->valueType == TYPE_STRUCT) sptv->value.so.fields[3] = nameTV;
    }
    if (!sp->isTV) {
        stackVariable* sptv = (stackVariable*)sp->addr;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[0]->value.numberValue.value.lVal = newPtr;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[1]->value.numberValue.value.lVal = 0;
        if (sptv->value->valueType == TYPE_STRUCT) sptv->value->value.so.fields[3] = nameTV;
        
    }
    freeTypedValue(arg0);
}
// C implementations for createDirectory, deleteDirectory, renameDirectory, & getFiles
void fDirectory(auFunc) {
    /*
    {.name = "createDirectory", .argc = 1, .returnStruct = NULL},
    {.name = "deleteDirectory", .argc = 1, .returnStruct = NULL},
    {.name = "renameDirectory", .argc = 1, .returnStruct = NULL},
    {.name = "getFiles", .argc = 1, .returnStruct = NULL},
    */
    typedValue* arg0 = getArray(args,  0);
    if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
    if (arg0->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
    char* path = strArrToChar(arg0);

    if (strcmp(identifier, "createDirectory") == 0) {
        if (CreateDirectory(path, NULL)) {

        }
        free(path);
    }
    if (strcmp(identifier, "deleteDirectory") == 0) {
        _rmdir(path);
        free(path);        
    }
    if (strcmp(identifier, "renameDirectory") == 0) {
        typedValue* arg1 = getArray(args,  1);
        if (arg1->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg1->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
        char* renamed = strArrToChar(arg1);
        rename(path, renamed);
        free(path);
        free(renamed);
    }
    if (strcmp(identifier, "getFiles") == 0) {
        typedValue* files = newTVArray(0, AT_UNKNOWN);
        typedValue* dirs = newTVArray(0, AT_UNKNOWN);
        WIN32_FIND_DATA f;
        char* buffer = malloc(strlen(path)+4); buffer[0] = '\0';
        sprintf(buffer, "%s\\\\*", path);
        HANDLE hfind = FindFirstFile(buffer, &f);
        do {
            char* toAdd = malloc(strlen(f.cFileName)+1); toAdd[0] = '\0';
            sprintf(toAdd, "%s", f.cFileName);
            if (toAdd[0] == '.') {continue;}
            // convert toAdd into a typedValue, then append it to either files or dirs depending on the file's type.
            typedValue* toApp = newTVArray(0, AT_CHARARR);
            for (int i = 0; i < strlen(toAdd); i++) {appendTypedValue(toApp, numToTV((num){.type = NUM_CHAR, .value.cVal = toAdd[i]}));}
            appendTypedValue(toApp, numToTV((num){.type = NUM_CHAR, .value.cVal = 0}));
            if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) appendTypedValue(dirs, toApp);
            else appendTypedValue(files, toApp);
            
            free(toAdd);

        } while (FindNextFile(hfind, &f) != 0);
        typedValue* toReturn = poolAlloc(globalPool);
        *toReturn = (typedValue){
            .ptr = NULL,
            .value.so = (struct structObject){
                .def = getStructDefViaName("directorySearch", *vms->bc),
                .fields = malloc(sizeof(typedValue*)*2)
            },
            .valueType = TYPE_STRUCT
        };
        toReturn->value.so.fields[0] = files;
        toReturn->value.so.fields[1] = dirs;
        pushArray(vms->stack, toReturn);
        free(path);
    }
    freeTypedValue(arg0);
}

