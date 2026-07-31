/*
--- bytecoder.h ---
This file contains definitions used in bytecoder.c & vm.c, and standard-library function declarations.
*/


#ifndef BYTECODER_H
#define BYTECODER_H

#include "lexer.h"
#include "parser.h"
#include "Tundora.h"
#include "num.h"
#include "array.h"
extern char* toExec;
extern char* projectFolder;
// struct that is exported from toBytecode().
typedef struct {
    array* globals;
    array* constants;
    array* functionIdentifiers;
    array* chunks;
    array* structDefs;
} byteCode;
// opcode enums
typedef enum {
    OP_LOAD_CONST=1,
    OP_LOAD_GLOBAL,
    OP_STORE_GLOBAL,

    OP_LOAD_LOCAL,
    OP_STORE_LOCAL,

    OP_CLEAR_STACK,

    OP_LOAD_IDX,
    OP_STORE_IDX,
    OP_ARRAY_LEN,
    OP_BUILD_ARRAY,

    OP_NEW_STRUCT,
    OP_GET_FIELD,
    OP_SET_FIELD,

    OP_ADD, // +
    OP_SUB, // a-b
    OP_MUL, // *
    OP_DIV,  // /
    OP_NEG, // -a
    OP_MOD,

    OP_BXOR, // ^
    OP_BAND, // &
    OP_BNOT, // ~
    OP_BOR, // |
    OP_LSHIFT,
    OP_RSHIFT,

    OP_GTHAN, // >
    OP_LTHAN, // <
    OP_LEQTHAN, // <=
    OP_GEQTHAN, // >=
    OP_EQUALS, // ==
    OP_OR, // ||
    OP_AND, // &&
    OP_NOT, // !
    OP_XOR,

    OP_CALL,
    OP_RETURN,

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_HALT
} Opcode;
// typedValue types
typedef enum {
    TYPE_NULL,
    TYPE_NUM,
    TYPE_ARRAY,
    TYPE_STRUCT
} type;
// Array value types.
typedef enum {
    AT_NUM=0,
    AT_CHARARR=1,
    AT_UNKNOWN
} arrayTypes;
typedef struct arrayValue {
    struct typedValue** data;
    int len;
    arrayTypes arrayType;
} arrayValue;
typedef struct {
    char* name;
} structField;
typedef struct {
    char* name;
    array* fields;
    array* fieldTypes;
} structDefinition;
typedef struct typedValue {
    union {
        num numberValue;
        arrayValue av;
        struct structObject {
            structDefinition* def;
            struct typedValue** fields;
        } so;
    } value;
    void* ptr;
    type valueType;
} typedValue;
typedef struct {
    union {
        char* name;
        int pdtType;
    } type;
    bool isPDT : 1;
    bool isArray : 1;
    bool isConst : 1;
} structTypes;
typedef struct {
    char* name;
    char* structType;
    type t;
    bool isArray : 1;
    bool isConst : 1;
} vmVariable;
structDefinition* getStructDefViaName(char* name, byteCode c);
typedef struct {
    int* args;
    char argc;
    char code;
} instruction;
typedef struct {
    array* instructions;
    char* name;
} chunk;
typedef struct {
    char* name;
    char* returnStruct;
    char argc;
    bool isSystem : 1;
} bcFunction;
void freeAST(ASTNode* p);
char* readTextFile(const char* filename);
// Default function insertion
static const bcFunction defaultFunctions[] = {
    // Console functions:
    {.name = "!opFunc", .argc = 1, .returnStruct = NULL}, // X
    {.name = "printInt", .argc = 2, .returnStruct = NULL}, // X
    {.name = "printDec", .argc = 3, .returnStruct = NULL}, // X
    {.name = "printString", .argc = 1, .returnStruct = NULL},  // X
    {.name = "scan", .argc = 1, .returnStruct = NULL}, // X
    {.name = "scani", .argc = 1, .returnStruct = NULL}, // X
    {.name = "scand", .argc = 1, .returnStruct = NULL}, // X
    {.name = "print", .argc = 2, .returnStruct = NULL}, 
    // Conversions
    {.name = "strToInt", .argc = 1, .returnStruct = NULL}, // X
    {.name = "strToDec", .argc = 1, .returnStruct = NULL}, // X
    {.name = "scaleInt", .argc = 2, .returnStruct = NULL}, //X
    {.name = "scaleDec", .argc = 2, .returnStruct = NULL}, // X
    // Float functions
    {.name = "isInf", .argc = 1, .returnStruct = NULL}, // X
    {.name = "isNaN", .argc = 1, .returnStruct = NULL}, // X
    {.name = "isNormal", .argc = 1, .returnStruct = NULL}, // X
    {.name = "toExp", .argc = 2, .returnStruct = "floatConversion"}, // X
    {.name = "toSciNo", .argc = 1, .returnStruct =  "floatConversion"}, // X
    // Array functions
    {.name = "array_push", .argc = 2, .returnStruct = NULL}, // X
    {.name = "array_append", .argc = 2, .returnStruct = NULL}, // X
    {.name = "array_pop", .argc = 2, .returnStruct = NULL }, // X
    {.name = "array_removeElement", .argc = 3, .returnStruct = NULL}, // X
    {.name = "array_sort", .argc = 2, .returnStruct = NULL}, // X
    {.name = "array_reverse", .argc = 1, .returnStruct = NULL}, // X
    {.name = "array_insertElement", .argc = 3, .returnStruct = NULL}, // X
    // String functions
    {.name = "newStr", .argc = 1, .returnStruct = "string"}, // X
    {.name = "string_toUpper", .argc = 2, .returnStruct = "string"}, // X
    {.name = "string_toLower", .argc = 2, .returnStruct = "string"}, // X
    {.name = "string_swapCase", .argc = 2, .returnStruct = "string"}, // X
    {.name = "string_findOccurances", .argc = 2, .returnStruct = "array"}, // X
    {.name = "string_split", .argc = 2, .returnStruct = "array"}, // X
    {.name = "string_splitStr", .argc = 2, .returnStruct = "array"}, // X
    {.name = "string_substr", .argc = 3, .returnStruct = "string"}, // X
    {.name = "string_leftPad", .argc = 3, .returnStruct = "string"}, // X
    {.name = "string_rightPad", .argc = 3, .returnStruct = "string"}, // X
    {.name = "string_replace", .argc = 3, .returnStruct = "string"}, // X
    {.name = "string_eqSplit", .argc = 2, .returnStruct = "array"}, // X
    {.name = "string_nSplit", .argc = 2, .returnStruct = "array"}, // X
    {.name = "formats", .argc = 2, .returnStruct = "string"},
    // Error-Handling
    {.name = "throw", .argc = 1, .returnStruct = NULL},
    {.name = "exit", .argc = 0, .returnStruct = NULL},

    {.name = "typeof", .argc = 1, .returnStruct = "string"},
    {.name = "", .argc = -1} // Terminator
};

void printBytecode(byteCode c, bool numsOnly, bool numberInstructions);

byteCode* convertToBytecode(ASTNode* prgm);


// Default insertion
static const char* defaultDefinitions = "\n\
struct exception {\n\
    int code;\n\
    string message;\n\
    int line;\n\
    function exception(int code, string message, int line) -> exception {\n\
        this.code = code; \n\
        this.message = message; \n\
        this.line = line;\n\
        return this;\n\
    }\n\
}\n\
struct floatConversion {\n\
    double[] arrayConversion;\n\
    string stringConversion;\n\
    function floatConversion(double[] arr, string str) -> floatConversion {\n\
        this.arrayConversion = arr;\n\
        this.stringConversion = str;\n\
        return this;\n\
    }\n\
}\n\
;"; 
#endif;
