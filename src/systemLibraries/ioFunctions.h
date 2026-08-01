#ifndef IOFUNC_H
#define IOFUNC_H
#include "../processing/bytecoder.h"
#include "sysFunctions.h"

void fDirectory(auFunc);
void fNewFile(auFunc);
void fClose(auFunc);
void fRead(auFunc);
void fWrite(auFunc);
void fSize(auFunc);
void fModify(auFunc);

static const bcFunction ioFunctions[] = {
    {.name = "fOpen", .argc = 1, .returnStruct = "file"}, // X
    {.name = "file_close", .argc = 1, .returnStruct = NULL}, // X
    {.name = "file_getChar", .argc = 1, .returnStruct = NULL}, // X
    {.name = "file_getStr", .argc = 2, .returnStruct = "string"}, // X
    {.name = "file_getLine", .argc = 2, .returnStruct = "string"}, // X
    {.name = "file_writef", .argc = 2, .returnStruct = NULL}, // X
    {.name = "file_putChar", .argc = 2, .returnStruct = NULL}, // X
    {.name = "file_putStr", .argc = 2, .returnStruct = NULL}, // X
    {.name = "file_read", .argc = 1, .returnStruct = "string"}, // X
    {.name = "file_delete", .argc = 1, .returnStruct = NULL}, // X
    {.name = "file_rename", .argc = 2, .returnStruct = NULL}, // X
    {.name = "file_size", .argc = 1, .returnStruct = NULL},  // X

    {.name = "createDirectory", .argc = 1, .returnStruct = NULL},
    {.name = "deleteDirectory", .argc = 1, .returnStruct = NULL},
    {.name = "renameDirectory", .argc = 2, .returnStruct = NULL},
    {.name = "getFiles", .argc = 1, .returnStruct = "directorySearch"},
    {.name = "", .argc = -1}
};

static const char* ioDefinitions = "\n\
const int FM_READ = 1i;\n\
const int FM_WRITE = 2i;\n\
const int FM_APPEND = 3i;\n\
const int FM_READWRITE = 4i;\n\
const int FM_READOVERWRITE = 5i;\n\
const int FM_READAPPEND = 6i;\n\
const int EOF = -1i;\n\
struct file {\n\
    long ptr; \n\
    long pos; \n\
    int mode; \n\
    string name;\n\
}\n\
struct directorySearch {\n\
    string[] files;\n\
    string[] childDirs;\n\
}\n\
";

#endif