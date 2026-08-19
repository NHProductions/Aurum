/*
--- parser.h ---
this is the headerfile for parser.c, which is responsible for parsing.
Inside, there's definitions that parser.c & other files use.


typedef enum varSpecifiers - Enum for identifier specifiers (const)
typedef enum ASTType - Enum containing valid ASTNode types.
typedef union ASTValue - Union containing an ASTNode's value.
typedef struct ASTNode - Struct containing an ASTNode's data.
typedef enum primDatatype - Enum containg some primitive data types (all datatypes that aren't structs)
typedef struct variable - parser variables, used to check if a identifier exists.
typedef struct function - parser functions, used to check if a identifier exists.
typedef struct scope - parser scope, used to store variables & functions.

Files to check next: bytecoder.c
*/

#ifndef PARSER_H
#define PARSER_H

#include "parser.h"
#include "lexer.h"
#include "../dataStorage/num.h"


#include <stdio.h>
#include <stdint.h>

// Normally, as structs are declared they get recognized by the parser, but for standard-library functions, that doesn't happen as the parser doesn't see their struct declarations, so this is an override for that process to get it to recognize it.
static const char* systemStructDecl[] = {
    "exception",
    "fnptr",
    "floatConversionResult",
    "UTCTime",
    "cplxNum",
    "vector2",
    "vector3",
    "vectorN",
    "mtrx",
    "file",
    "directorySearch",
    "window",
    "wmsg",
    ""
};

typedef enum {
    VS_CONST
} varSpecifiers;
typedef enum {
    AST_NONE,
    AST_NUMBER,
    AST_IMPORT,
    AST_LIBRARY,
    AST_IDENTIFIER,
    AST_OPERATOR,
    AST_STRING,
    AST_CONDITIONAL,
    AST_VARDECLARATION,
    AST_ASSIGNMENT,
    AST_BLOCK,
    AST_IF,
    AST_ELSE,
    AST_IDENTIFIERSPECIFIER,
    AST_ELIF,
    AST_WHILE,
    AST_FOR,
    AST_FOREACH,
    AST_IN,
    AST_CONTINUE,
    AST_BREAK,
    AST_FUNCDEL,
    AST_FUNCCALL,
    AST_RETURN,
    AST_BLOCKEND,
    AST_ARRAYLITERAL,
    AST_ARRAYASSIGNMENT,
    AST_ARRAYACCESS,
    AST_EXPREND,
    AST_PRGM,
    AST_ARROW,
    AST_VOID,
    AST_PARAM,
    AST_COMMA,
    AST_DATATYPE,
    AST_COMPLEXDATATYPE,
    AST_SWITCH,
    AST_CASE,
    AST_STRUCT,
    AST_STRUCTACCESS,
    AST_STRUCTASSIGNMENT,
    AST_STRUCTBLOCK,
    AST_DOT

} ASTType;
typedef union {
    double numberVal;
    num numVal;
    char* nameVal;
    char* opVal;
    char* anyVal;
} ASTValue;
typedef struct ASTNode {
    ASTValue value;
    struct ASTNode* left;
    struct ASTNode* right;
    List* children;
    ASTType type : 8;
} ASTNode;
typedef enum {
    PDT_NULL,
    PDT_VOID,
    PDT_STRING,
    PDT_AUTO,

    // Num types:
    PDT_BOOL,
    PDT_CHAR,
    PDT_UCHAR,
    PDT_SHORT,
    PDT_USHORT,
    PDT_INT,
    PDT_UINT,
    PDT_LONG,
    PDT_ULONG,
    PDT_FLOAT,
    PDT_DOUBLE,
    PDT_LONGDOUBLE
} primDatatype;

typedef union {
    char* name;
    primDatatype t;
} customTypeIdentifierUnion;
typedef struct {
    customTypeIdentifierUnion value;
    bool isArray;
} customType;
typedef struct {
    char* name;
    customType type;
    bool isConst;
} variable;
typedef struct {
    char* name;
} function;
typedef struct {
    List variables;
    List functions;
    List customTypes;
} scope;
ASTNode* newASTNode();
ASTNode* newNumberNode(double a);
ASTNode* newIdentifierNode(char* name);
ASTNode* newOperatorNode(char* op, ASTNode* Left, ASTNode* Right);
ASTNode* parseTokenList(List* n);

void printAST(ASTNode* node, int depth);
void freeAST(ASTNode* node);
#endif
