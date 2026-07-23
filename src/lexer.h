#ifndef LEXER_H
#define LEXER_H
#include "Tundora.h"
/*
--- Lexer.h ---
This file contains definitions for tokens.
auOperators is a list of valid operator tokens
auGrouping is a list of valid grouping tokens
auDelimiter is a list of valid delimiters (it's just ;)
auKeywords is a list of valid keyword tokens
auComma is a list of comma-like tokens (tokens that need non-keyword characters on both sides)

tokenType is a enum containing valid types for tokens.
tokenFound is a struct containing a token, a tokenType, and an amount to skip (as when the lexer encounters a token, it jumps forward by that token's length)
token is the struct that's returned by the lexer.
*/

static const char* auOperators[] = {
    "!=",
    "==", // Assignment
    "+","+=", // Addition & Addition-Assignment
    "-","-=", // Subtraction & Subtraction-Assignment
    "/","/=", // Division
    "//","//=", // Floor-Division
    "*","*=", // Multiplication
    "**","**=", // Exponentiation
    "^","^^","^=", // XOR
    "&","&=","&&", // AND
    "|","|=","||", // OR
    "!", // Logical NOT
    "~", // Bitwise NOT
    "++", // Increment
    "--", // Decrement
    "%", "%=", // Modulo
    ">", // Greater Than
    "<", // Less than
    ">=", "=>", // Greater than or Equal to
    "<=", "=<", // Less than or equal to
    "<<", ">>", ">>=", "<<=", // Bitwise Shift
    "#", "=", // Equality
}; 
#define OP_LEN 41
static const char* auGrouping[] = {
    "(", ")", "[", "]", "{", "}"
};
#define GR_LEN 6
static const char* auDelimiter[] = {
    ";"
};
#define DE_LEN 1
static const char* auKeywords[] = {
    "bool", "char", "short", "int", "long", "float", "double", "longdouble", // Number types (names)
    "int8", "int16", "int32", "int64", "double32", "double64", "double128", // Number types (precision format)
    "uint8", "uint16", "uint24", "uint32", "uint64", // Unsigned number types (precision format)
    "uchar", "ushort", "uint", "ulong", "ulonglong", // Unsigned number types (names)

    "string", "NaN", "inf",
    "const","auto", // Variable Modifiers
    "true", "false", // Booleans
    "if", "else","elif", "switch", "case", // Conditionals
    "for", "while", "break", "continue", // Loops
    "return", "->", "function", "void", // Functions
    "struct", // objects
    "import", // imports
};
#define KW_LEN 47
static const char* auComma[] = {
    ",", "."
};
#define CO_LEN 2
typedef enum {
    T_NONE=0,
    T_IDENTIFIER,
    T_OPERATOR,
    T_STRING,
    T_CHAR,
    T_NUMBER,
    T_KEYWORD,
    T_DELIMITER,
    T_GROUPING,
    T_LIBRARY

} tokenType;
struct tokenFound {
    char* token;
    int amtToSkip;
    tokenType type;
};
typedef struct token {
    tokenType type;
    char* value;
} token;

bool isIdentifierChar(char c);
bool isValidChar(char n);
bool isSymbolChar(char c);
List* tokenize(char* n);
void printTokenList(List* lst);
#endif