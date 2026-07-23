/*
--- num.c ---
This file contains functions for dealing with number-unions
For reference, here's the num struct:
typedef struct {
    numberTypes type;
    union {
        char cVal; // 1 byte
        unsigned char ucVal; // 1 byte
        short sVal; // 2 bytes
        unsigned short usVal; // 2 bytes
        int iVal; // 3 bytes
        unsigned int uiVal; // 3 bytes
        int64_t lVal; // 8 bytes (64 bits)
        uint64_t ulVal; // 8 bytes (64 bits)
        float fVal; // 4 bytes (32 bits)
        double dVal; // 8 bytes (64 bits)
        long double ldVal; // 16 bytes (128 bits)
        bool bVal; // 1 byte
    } value;
} num;
Essentially, it's just a union with all valid number values, & these functions make it easier to deal with num structs so that I don't need to have 200 if statements dictating what to do for each number type.
Additionally, as you might've saw in main.c, the language checks to see if all types are their correct size.

Files to check next: num.h, parser.c, bytecoder.c
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "num.h"
#include "winInclude.h"
#include "lexer.h"
// Converts a number to a int64_t (long long).
int64_t toLL(num n) {
    switch (n.type) {
        case NUM_CHAR: {return (int64_t)n.value.cVal;}
        case NUM_SHORT: {return (int64_t)n.value.sVal;}
        case NUM_INT: {return (int64_t)n.value.iVal;}
        case NUM_LONG: {return (int64_t)n.value.lVal;}
        case NUM_UCHAR: {return (int64_t)n.value.ucVal;}
        case NUM_USHORT: {return (int64_t)n.value.usVal;}
        case NUM_UINT: {return (int64_t)n.value.uiVal;}
        case NUM_ULONG: {return (int64_t)n.value.ulVal;}
        case NUM_FLOAT: {return (int64_t)n.value.fVal;}
        case NUM_DOUBLE: {return (int64_t)n.value.dVal;}
        case NUM_LONGDOUBLE: {return (int64_t)n.value.ldVal;}
        case NUM_BOOL: {return (int64_t)n.value.bVal;}
        default: {return (int64_t)0;}
    }
    return 0;
}
// Converts a number to a uint64_t
uint64_t toULL(num n) {
    switch (n.type) {
        case NUM_CHAR: {return (uint64_t)n.value.cVal;}
        case NUM_SHORT: {return (uint64_t)n.value.sVal;}
        case NUM_INT: {return (uint64_t)n.value.iVal;}
        case NUM_LONG: {return (uint64_t)n.value.lVal;}
        case NUM_UCHAR: {return (uint64_t)n.value.ucVal;}
        case NUM_USHORT: {return (uint64_t)n.value.usVal;}
        case NUM_UINT: {return (uint64_t)n.value.uiVal;}
        case NUM_ULONG: {return (uint64_t)n.value.ulVal;}
        case NUM_FLOAT: {return (uint64_t)n.value.fVal;}
        case NUM_DOUBLE: {return (uint64_t)n.value.dVal;}
        case NUM_LONGDOUBLE: {return (uint64_t)n.value.ldVal;}
        case NUM_BOOL: {return (uint64_t)n.value.bVal;}
    }
    return 0;
}
// Converts a number to a long double
long double toLD(num n) {
    switch (n.type) {
        case NUM_CHAR: {
            char val = n.value.cVal;
            return (long double)n.value.cVal;
        }
        case NUM_SHORT: {
            short val = n.value.sVal;
            return (long double)n.value.sVal;
        }
        case NUM_INT: {
            return (long double)n.value.iVal;
        }
        case NUM_LONG: {
            long val = n.value.lVal;
            return (long double)n.value.lVal;
        }
        case NUM_UCHAR: {
            unsigned char val = n.value.ucVal;
            return (long double)n.value.ucVal;
        }
        case NUM_USHORT: {
            unsigned short val = n.value.usVal;
            return (long double)n.value.usVal;
        }
        case NUM_UINT: {
            unsigned int val = n.value.uiVal;
            return (long double)n.value.uiVal;
        }
        case NUM_ULONG: {
            unsigned long val = n.value.ulVal;
            return (long double)n.value.ulVal;
        }
        case NUM_FLOAT: {
            float val = n.value.fVal;
            return (long double)n.value.fVal;
        }
        case NUM_DOUBLE: {
            double val = n.value.dVal;
            return (long double)n.value.dVal;
        }
        case NUM_LONGDOUBLE: {
            return n.value.ldVal;
        }
        case NUM_BOOL: {return (long double)n.value.bVal == 0 ? 0.0L : 1;}
    }
    printf("Unknown num"); exit(1);
    return 0;
}
// Converts a number type to a string
char* nTypeToStr(num n) {
    switch (n.type) {
        case NUM_CHAR: {return "CHAR";}
        case NUM_SHORT: {return "SHORT";}
        case NUM_INT: {return "INT";}
        case NUM_LONG: {return "LONG";}
        case NUM_FLOAT: {return "FLOAT";}
        case NUM_DOUBLE: {return "DOUBLE";}
        case NUM_LONGDOUBLE: {return "LONGDOUBLE";}
        case NUM_UCHAR: {return "UCHAR";}
        case NUM_USHORT: {return "USHORT";}
        case NUM_UINT: {return "UINT";}
        case NUM_ULONG: {return "ULONG";}
        case NUM_BOOL: {return "BOOL";}
    }
    printf("UNKNOWN NUM: %d", n.type);
    return "UNKNOWN NUMBER";
}
// Essentially converts a number into a TYPE(VALUE) format.
// e.x 2i would be #INT(2).
// For booleans, it just uses true/false.
void sprintfNum(num n, char* t, bool includeType) {
    char* strPart = includeType ? nTypeToStr(n) : "";
    int abs = n.type < 0 ? -n.type : n.type;
    if (abs < NUM_FLOAT && abs != 0) {
        if (n.type < 0) {
            sprintf(t, "#%s(%llu)", strPart, toULL(n));
        }
        else {
            sprintf(t, "#%s(%lld)", strPart, toLL(n));
        }
    }
    else if (n.type >= NUM_FLOAT) {
        sprintf(t, "#%s(%Lf)", strPart, toLD(n));
    }
    else {
        sprintf(t, "#%s(%s)", strPart, n.value.bVal != 0 ? "True" : "False");
    }
}
// Counts the letters in a string (Used for strToNum to count how many letters it should process)
int countLetters(char* s) {
    int toReturn = 0;
    for (int i = 0; i < strlen(s); i++) {
        char n = s[i];
        if ((n >= 'A' && n <= 'Z') || (n >= 'a' && n <= 'z')) toReturn++;
    }
    return toReturn;
}
// Converts a token whose type is T_NUMBER into a num struct. (Btw, base-literals were already handled in lexer.c, so 0xFFld is being processed as "127ld")
num strToNum(token t) {
    num toReturn = (num){
        .type = 0,
        .value.sVal = 0
    };
    int letters = countLetters(t.value);
    // If there's no letters, guess the type of the number (if it has decimal, it's a float, if not, then int)
    if (letters == 0) {
        bool hasDecimal = false;
        char* endptr;
        for (int i = 0; i < strlen(t.value); i++) {if (t.value[i] == '.') {hasDecimal = true; break;} }
        if (hasDecimal) return (num){.type = NUM_FLOAT, .value.fVal = strtof(t.value, &endptr)};
        else return (num){.type = NUM_INT, .value.iVal=(int)strtof(t.value, &endptr)};
    }
    // Allocate a buffer, and populate it with all number characters in the token.
    // When a letter is encountered, change the type of toReturn to fit it.
    char nums[500];
    nums[0] = '\0';
    int numLen = 0;
    for (int i = 0; i < strlen(t.value); i++) {
        char currentChar = t.value[i];
        if ((currentChar >= '0' && currentChar <= '9') || currentChar == '.' || currentChar == '-') {
            nums[numLen] = currentChar;
            nums[numLen+1] = '\0';
            numLen++;
        }
        else {
            switch (currentChar) {
                case 'c': {toReturn.type = NUM_CHAR;break;}
                case 'u': {toReturn.type *= -1; break;} // If you haven't noticed, in the numberType enum, all unsigned types are the negative versions of their signed counterpart. (e.x int=3, uint=-3), making it easier to convert from signed->unsigned.
                case 's': {toReturn.type = NUM_SHORT; break;}
                case 'i': {toReturn.type = NUM_INT; break;}
                case 'l': {
                    // If a 'd' has been encountered, make the type into NUM_LONGDOUBLE, if not, make it NUM_LONG.
                    int isU = toReturn.type < 0 ? -1 : 1;
                    switch (toReturn.type < 0 ? -toReturn.type : toReturn.type) {
                        default: {toReturn.type = isU*NUM_LONG; break;}
                        case (NUM_DOUBLE): {toReturn.type = NUM_LONGDOUBLE; break;}
                    }
                    break;
                }
                case 'f': {toReturn.type = NUM_FLOAT; break;}
                case 'd': {
                    // If a 'l' has been encountered, make it NUM_LONGDOUBLE, otherwise make it NUM_DOUBLE
                    toReturn.type = toReturn.type != NUM_LONG ? NUM_DOUBLE : NUM_LONGDOUBLE; break;
                }
                default: {break;}
            }
        }
    }
    long double val = 0.0L;
    char* endptr = "";
    // Finally, convert the number buffer back into a number, keeping the new type.
    val = strtold(nums, &endptr);
    switch (toReturn.type) {
        case NUM_CHAR: {toReturn.value.cVal = (char)val;break;}
        case NUM_UCHAR: {toReturn.value.ucVal = (unsigned char)val;break;}
        case NUM_SHORT: {toReturn.value.sVal = (short)val;break;}
        case NUM_USHORT: {toReturn.value.usVal = (unsigned short)val;break;}
        case NUM_INT: {toReturn.value.iVal = (int)val;break;}
        case NUM_UINT: {toReturn.value.uiVal = (unsigned int)val;break;}
        case NUM_LONG: {toReturn.value.lVal = (long)val;break;}
        case NUM_ULONG: {toReturn.value.ulVal = (unsigned long)val;break;}
        case NUM_FLOAT: {toReturn.value.fVal = (float)val;break;}
        case NUM_DOUBLE: {toReturn.value.dVal = (double)val;break;}
        case NUM_LONGDOUBLE: {toReturn.value.ldVal = (long double)val;break;}
        case NUM_BOOL: {toReturn.value.bVal = (bool)val; break;}
        default: {break;}
    }
    return toReturn;


}
// Converts a number a into a different type, keeping the same value.
num convertNum(num a, numberTypes n) {
    // initialize ll, dll, & ull, which will be used depending on the target type.
    // Might be better to initialize them in the switch statement.
    int64_t ll = toLL(a);
    long double dll = toLD(a);
    uint64_t ull = toULL(a);
    // Convert the bool into the range [0,1], as oftentimes booleans have values other than 0 or 1 in C.
    if (a.type == NUM_BOOL && a.value.bVal) {ll = 1LL; dll = 1; ull=1ULL;} 
    else if (a.type == NUM_BOOL) {ll = 0LL; dll = 0; ull = 0ULL;}
    // Finally, return the new number.
    switch (n) {
        case NUM_BOOL: {return (num){.type = n, .value.bVal = (bool)ll};}
        case NUM_CHAR: {return (num){.type = n, .value.cVal = (char)ll};}
        case NUM_SHORT: {return (num){.type = n, .value.sVal = (short)ll};}
        case NUM_INT: {return (num){.type = n, .value.iVal = (int)ll};}
        case NUM_LONG: {return (num){.type = n, .value.lVal = (int64_t)ll};}
        case NUM_FLOAT: {return (num){.type = n, .value.fVal = (float)dll};}
        case NUM_DOUBLE: {return (num){.type = n, .value.dVal = (double)dll};}
        case NUM_LONGDOUBLE: {return (num){.type = n, .value.ldVal = (long double)dll};}
        case NUM_UCHAR: {return (num){.type = n, .value.ucVal = (unsigned char)ull};}
        case NUM_USHORT: {return (num){.type = n, .value.usVal = (unsigned short)ull};}
        case NUM_UINT: {return (num){.type = n, .value.uiVal = (unsigned int)ull};}
        case NUM_ULONG: {return (num){.type = n, .value.ulVal = (uint64_t)ull};}
    }
    return (num){.type = NUM_BOOL, .value.bVal = false};
}
// Makes a new number when given a existing num, and a type.
num newNum(num n, int type) {
    int64_t l = toLL(n);
    uint64_t ull = toULL(n);
    long double ld = toLD(n);
    switch (type) {
        case NUM_CHAR: {
            return (num){.type = NUM_CHAR, .value.cVal = (char)l};
        }
        case NUM_SHORT: {
            return (num){.type = NUM_SHORT, .value.sVal = (short)l};
        }
        case NUM_INT: {
            return (num){.type = NUM_INT, .value.iVal = (int)l};
        }
        case NUM_LONG: {
            return (num){.type = NUM_LONG, .value.lVal = (int64_t)l};
        }
        case NUM_FLOAT: {
            return (num){.type = NUM_FLOAT, .value.fVal = (float)ld};
        }
        case NUM_DOUBLE: {
            return (num){.type = NUM_DOUBLE, .value.dVal = (double)ld};
        }
        case NUM_LONGDOUBLE: {
            return (num){.type = NUM_DOUBLE, .value.ldVal = ld};
        }
        case NUM_UCHAR: {
            return (num){.type = NUM_UCHAR, .value.ucVal = (unsigned char)l};
        }
        case NUM_USHORT: {
            return (num){.type = NUM_USHORT, .value.usVal = (unsigned short)l};
        }
        case NUM_UINT: {
            return (num){.type = NUM_UINT, .value.uiVal = (unsigned int)l};
        }
        case NUM_ULONG: {
            return (num){.type = NUM_ULONG, .value.ulVal = (uint64_t)l};
        }
    }
    fatalError(0x11, "Invalid type for new number.", -1);
}
#define absol(a) (a < 0 ? -a : a)
// Converts two numbers into the same type for operations.
pair convertNums(num a, num b) {
    pair toReturn = (pair){.a = (num){.type = NUM_BOOL, .value.bVal = 0}, .b = (num){.type = NUM_BOOL, .value.bVal = 0}}; // initialize a return struct.
    // If sign(a.type) == sign(b.type), set d to sign(a.type).
    int d = 0; 
    if ((a.type > 0 && b.type > 0) || (a.type < 0 && b.type < 0) ) {
        if (a.type > 0) d = 1;
        else d = -1;
    }
    // If a & b are already the same type, just return them, as they're already valid.
    if (a.type == b.type) {
        return (pair){
            .a = a,
            .b = b
        };
    }
    // if d isn't zero (a & b have the same signage), then convert it to whichever one has the higher type. (ld <- d <- f <- l <- i <- s <- c)
    if (d != 0) {
        if (a.type == NUM_LONGDOUBLE || b.type == NUM_LONGDOUBLE) {
            return (pair){
                .a = newNum(a, NUM_LONGDOUBLE),
                .b = newNum(b, NUM_LONGDOUBLE)
            };
        }
        else if (a.type == NUM_DOUBLE || b.type == NUM_DOUBLE) {
            return (pair){
                .a = newNum(a, NUM_DOUBLE),
                .b = newNum(b, NUM_DOUBLE)
            };
        }
        else if (a.type == NUM_FLOAT || b.type == NUM_FLOAT) {
            return (pair){
                .a = newNum(a, NUM_FLOAT),
                .b = newNum(b, NUM_FLOAT)
            };
        }
        else if (a.type == d*NUM_LONG || b.type == d*NUM_LONG) {
            return (pair){
                .a = newNum(a, d*NUM_LONG),
                .b = newNum(b, d*NUM_LONG)
            };
        }
        else if (a.type == d*NUM_INT || b.type == d*NUM_INT) {
            return (pair){
                .a = newNum(a, d*NUM_INT),
                .b = newNum(b, d*NUM_INT)
            };
        }
        else if (a.type == d*NUM_SHORT || b.type == d*NUM_SHORT) {
            return (pair){
                .a = newNum(a, d*NUM_SHORT),
                .b = newNum(b, d*NUM_SHORT)
            };
        }
        else if (a.type == d*NUM_CHAR || b.type == d*NUM_CHAR) {
            return (pair){
                .a = newNum(a, d*NUM_CHAR),
                .b = newNum(b, d*NUM_CHAR)
            };
        }
    }
    // If both are non-float numbers, and one of the numbers is unsigned, return the next highest type.
    else if (absol(a.type) < NUM_FLOAT && absol(b.type) < NUM_FLOAT && ((a.type > 0 && b.type < 0) || (a.type < 0 && b.type > 0))) {
        int maxType = a.type > b.type ? a.type : b.type;
        return (pair){
            .a = newNum(a, maxType+1),
            .b = newNum(b, maxType+1)
        };
    }
    // If one of them is bools, return the other one..
    else if (a.type == NUM_BOOL || b.type == NUM_BOOL) {
        int newType = a.type == NUM_BOOL ? b.type : a.type;
        return (pair){
            .a = newNum(a, newType),
            .b = newNum(b, newType)
        };
    }
    // Otherwise, just throw an error and exit the program.
    fatalError(12, "Invalid conversion for a number.", -1);
    return (pair){.a = (num){.type = NUM_SHORT, .value = 0}, .b = (num){.type = NUM_SHORT, .value = 0}};
}
// powUInt() & powInt() are used in doBinaryOperation, for number literals (e.x 2i**5i). For things involving identifiers, !opFunc() is used (see systemFunctions.c)
uint64_t powUInt(uint64_t A, uint64_t B) {
    uint64_t toReturn = 1;
    for (uint64_t i = B; i > 0; i--) {
        toReturn *= A;
    }
    return toReturn;
}
int64_t powInt(int64_t A, int64_t B) {
    if (B < 0) {fatalError(0x13, "Cannot raise an integer by a <0 number. Convert it to a decimal.", -1);}
    int64_t toReturn = 1;
    for (int64_t i = B; i > 0; i--) {
        toReturn *= A;
    }
    return toReturn;
}
// Does binary operations by converting both numbers to the same type, then doing that operation.
// I'm sure there's a more compact way to do this lol
num doBinaryOperation(char* op, num a, num b) {
    
    pair n = convertNums(a, b);
    long double dfA = toLD(n.a);
    long double dfB = toLD(n.b);
    int64_t llA = toLL(n.a);
    int64_t llB = toLL(n.b);
    uint64_t ullA = toULL(n.a);
    uint64_t ullB = toULL(n.b);
    // Binary arithmetic operators:
    if (strcmp(op, "-") == 0) {
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA-(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA-(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA-(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA-(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA-(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA-(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA-(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA-(unsigned long)ullB};
        // Decimal Types
        if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)dfA-(float)dfB};
        if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)dfA-(double)dfB};
        if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)dfA-(long double)dfB};
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = (bool)llA-(bool)llB};
    }
    if (strcmp(op, "+") == 0) {
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA+(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA+(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA+(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA+(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA+(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA+(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA+(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA+(unsigned long)ullB};
        // Decimal Types
        if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)dfA+(float)dfB};
        if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)dfA+(double)dfB};
        if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)dfA+(long double)dfB};
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = (bool)llA+(bool)llB};
    }
    if (strcmp(op, "*") == 0) {
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA*(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA*(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA*(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA*(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA*(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA*(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA*(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA*(unsigned long)ullB};
        // Decimal Types
        if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)dfA*(float)dfB};
        if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)dfA*(double)dfB};
        if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)dfA*(long double)dfB};
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = (bool)llA*(bool)llB};
    }
    if (strcmp(op, "/") == 0) {
        
        // Check if the 2nd number is zero, as zero-division is illegal.
        if (convertNum(n.b, NUM_LONGDOUBLE).value.ldVal == 0) {fatalError(0x16, "Cannot divide by zero", -1);}
        // integer types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA/(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA/(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA/(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA/(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA/(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA/(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA/(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA/(unsigned long)ullB};
        // Decimal Types
        if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)dfA/(float)dfB};
        if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)dfA/(double)dfB};
        if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)dfA/(long double)dfB};
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = (bool)llA/(bool)llB};
    }
    if (strcmp(op, "%") == 0) {
        // Modulo is only defined for integers.
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA%(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA%(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA%(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA%(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA%(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA%(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA%(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA%(unsigned long)ullB};
        // Decimal Types
        if (n.a.type >= NUM_FLOAT) {
            fatalError(0x15, "Cannot use a decimal for the first input of a modulo operation (e.x 2.2%%5).", -1);
        }
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = (bool)llA%(bool)llB};
    }
    if (strcmp(op, "**") == 0) {
        // Integer Types:
        if (n.a.type > 0 && n.a.type < NUM_FLOAT) {
            if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)powInt(llA, llB)};
            if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)powInt(llA, llB)};
            if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)powInt(llA, llB)};
            if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)powInt(llA, llB)};
        }
        else if (n.a.type < 0) {
            // Unsigned Int Types:
            if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.cVal = (unsigned char)powUInt(ullA, ullB)};
            if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.sVal = (unsigned short)powUInt(ullA, ullB)};
            if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.iVal = (unsigned int)powUInt(ullA, ullB)};
            if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.lVal = (unsigned long)powUInt(ullA, ullB)};
        }
        else if (n.a.type >= NUM_FLOAT) {
            // Decimal Types
            long double nn = floor(convertNum(n.b, NUM_LONGDOUBLE).value.ldVal);
            if (convertNum(n.a, NUM_LONGDOUBLE).value.ldVal < 0 && convertNum(n.b, NUM_LONGDOUBLE).value.ldVal != nn) {fatalError(0x17, "Cannot raise a real number by a fractional exponent. Try using the complexNumber class.", -1);}
            if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)powl(dfA, dfB)};
            if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)powl(dfA, dfB)};
            if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)powl(dfA, dfB)};
        }
        // Boolean
        if (n.a.type == NUM_BOOL) return (num){.type = n.a.type, .value.bVal = 0};
    }
    if (strcmp(op, "//") == 0) {
        // Floor division is only defined for decimals.
        if (n.a.type >= NUM_FLOAT) {
            if (n.a.type == NUM_FLOAT) return (num){.type = n.a.type, .value.fVal = (float)floorl(llA/llB)};
            if (n.a.type == NUM_DOUBLE) return (num){.type = n.a.type, .value.dVal = (double)floorl(llA/llB)};
            if (n.a.type == NUM_LONGDOUBLE) return (num){.type = n.a.type, .value.ldVal = (long double)floorl(llA/llB)};
        }
        else {
            fatalError(0x18, "Floor division only works with floats.", -1);
        }

    }
    // Comparison operators:
    if (strcmp(op, "==") == 0) {
        if (n.a.type > NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA==dfB)};
        }
        else if (n.a.type >= 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA==llB)};
        }
        else if (n.a.type != 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA==ullB)};
        }
        
    }
    if (strcmp(op, ">=") == 0 || strcmp(op, "=>") == 0) {
        if (n.a.type > NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA>=dfB)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA>=llB)};
        }
        else if (n.a.type != 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA>=ullB)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA >= (bool)ullB)};
    }
    if (strcmp(op, "<=") == 0 || strcmp(op, "=<") == 0) {
        if (n.a.type < NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA<=dfB)};
        }
        else if (n.a.type < 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA<=llB)};
        }
        else if (n.a.type != 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA<=ullB)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA <= (bool)ullB)};
    }
    if (strcmp(op, "<") == 0 || strcmp(op, "<") == 0) {
        if (n.a.type >= NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA<dfB ? 1 : 0)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA<llB ? 1 : 0)};
        }
        else if (n.a.type != 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA<ullB ? 1 : 0)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA < (bool)ullB)};
    }
    if (strcmp(op, ">") == 0 || strcmp(op, ">") == 0) {
        if (n.a.type >= NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA>dfB ? 1 : 0)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA>llB ? 1 : 0)};
        }
        else if (n.a.type != 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA>ullB ? 1 : 0)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA > (bool)ullB)};
    }
    if (strcmp(op, "!=") == 0) {
        
        if (n.a.type >= NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA!=dfB)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA!=llB)};
        }
        else if (n.a.type < 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA!=ullB)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA != (bool)ullB)};
    }
    // Boolean Operators:
    if (strcmp(op, "&&") == 0 || strcmp(op, "&&") == 0) {
        if (n.a.type >= NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA&&dfB)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA&&llB)};
        }
        else if (n.a.type < 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA&&ullB)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA && (bool)ullB)};
    }
    if (strcmp(op, "||") == 0 || strcmp(op, "||") == 0) {
        if (n.a.type >= NUM_FLOAT) {
            return (num){.type = NUM_BOOL, .value.bVal=(dfA||dfB)};
        }
        else if (n.a.type > 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(llA||llB)};
        }
        else if (n.a.type < 0) {
            return (num){.type = NUM_BOOL, .value.bVal=(ullA||ullB)};
        }
        return (num){.type = NUM_BOOL, .value.bVal=((bool)ullA || (bool)ullB)};
    }
    if (strcmp(op, "^^") == 0 || strcmp(op, "^^") == 0) {
        // a xor b = a || b && !(a && b)
        return doBinaryOperation("&&", doBinaryOperation("||", a, b), doUnaryOperation("!", doBinaryOperation("&&", a, b)));
    }
    // Bitwise Operators (only defined for integers):
    if (strcmp(op, "^") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA^(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA^(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA^(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA^(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA^(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA^(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA^(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA^(unsigned long)ullB};
    
    }
    if (strcmp(op, "&") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA&(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA&(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA&(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA&(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA&(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA&(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA&(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA&(unsigned long)ullB};
    
    }
    if (strcmp(op, "|") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA|(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA|(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA|(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA|(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA|(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA|(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA|(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA|(unsigned long)ullB};
    
    }
    if (strcmp(op, ">>") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA>>(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA>>(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA>>(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA>>(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA>>(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA>>(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA>>(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA>>(unsigned long)ullB};
    
    }
    if (strcmp(op, "<<") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (n.a.type == NUM_CHAR) return (num){.type = n.a.type, .value.cVal = (char)llA<<(char)llB};
        if (n.a.type == NUM_SHORT) return (num){.type = n.a.type, .value.sVal = (short)llA<<(short)llB};
        if (n.a.type == NUM_INT) return (num){.type = n.a.type, .value.iVal = (int)llA<<(int)llB};
        if (n.a.type == NUM_LONG) return (num){.type = n.a.type, .value.lVal = (long)llA<<(long)llB};
        // Unsigned Int Types:
        if (n.a.type == NUM_UCHAR) return (num){.type = n.a.type, .value.ucVal = (unsigned char)ullA<<(unsigned char)ullB};
        if (n.a.type == NUM_USHORT) return (num){.type = n.a.type, .value.usVal = (unsigned short)ullA<<(unsigned short)ullB};
        if (n.a.type == NUM_UINT) return (num){.type = n.a.type, .value.uiVal = (unsigned int)ullA<<(unsigned int)ullB};
        if (n.a.type == NUM_ULONG) return (num){.type = n.a.type, .value.ulVal = (unsigned long)ullA<<(unsigned long)ullB};
    
    }


    fatalError(0x1A, "Cannot find binary operator.", -1);
    return (num){.type = NUM_BOOL, .value.bVal = false};
}
// Does unary operations; converting the number isn't really neccessary here like it is for doBinaryOperation..
num doUnaryOperation(char* op, num a) {
    long double dfA = toLD(a);
    int64_t llA = toLL(a);
    uint64_t ullA = toULL(a);
    // unary minus
    if (strcmp(op, "-") == 0) {
        if (a.type == NUM_CHAR) return (num){.type = a.type, .value.cVal = -(char)llA};
        if (a.type == NUM_SHORT) return (num){.type = a.type, .value.sVal = -(short)llA};
        if (a.type == NUM_INT) return (num){.type = a.type, .value.iVal = -(int)llA};
        if (a.type == NUM_LONG) return (num){.type = a.type, .value.lVal = -(long)llA};
        // Unsigned Int Types:
        if (a.type == NUM_UCHAR) return (num){.type = a.type, .value.ucVal = -(unsigned char)ullA};
        if (a.type == NUM_USHORT) return (num){.type = a.type, .value.usVal = -(unsigned short)ullA};
        if (a.type == NUM_UINT) return (num){.type = a.type, .value.uiVal = -(unsigned int)ullA};
        if (a.type == NUM_ULONG) return (num){.type = a.type, .value.ulVal = -(unsigned long)ullA};
        // Decimal Types
        if (a.type == NUM_FLOAT) return (num){.type = a.type, .value.fVal = -(float)dfA};
        if (a.type == NUM_DOUBLE) return (num){.type = a.type, .value.dVal = -(double)dfA};
        if (a.type == NUM_LONGDOUBLE) return (num){.type = a.type, .value.ldVal = -(long double)dfA};
        // Boolean
        if (a.type == NUM_BOOL) return (num){.type = a.type, .value.bVal = (llA == 0 ? 0 : -1)};
    }
    // unary bitwise not
    if (strcmp(op, "~") == 0) {
        if (a.type >= NUM_FLOAT || a.type == 0) {fatalError(0x19, "Bitwise operators (^, |, &, ~, >>, <<) only work with integers.", -1);}
        // Integer Types:
        if (a.type == NUM_CHAR) return (num){.type = a.type, .value.cVal = ~(char)llA};
        if (a.type == NUM_SHORT) return (num){.type = a.type, .value.sVal = ~(short)llA};
        if (a.type == NUM_INT) return (num){.type = a.type, .value.iVal = ~(int)llA};
        if (a.type == NUM_LONG) return (num){.type = a.type, .value.lVal = ~(long)llA};
        // Unsigned Int Types:
        if (a.type == NUM_UCHAR) return (num){.type = a.type, .value.ucVal = ~(unsigned char)ullA};
        if (a.type == NUM_USHORT) return (num){.type = a.type, .value.usVal = ~(unsigned short)ullA};
        if (a.type == NUM_UINT) return (num){.type = a.type, .value.uiVal = ~(unsigned int)ullA};
        if (a.type == NUM_ULONG) return (num){.type = a.type, .value.ulVal = ~(unsigned long)ullA};
    }
    // unary boolean not
    if (strcmp(op, "!") == 0) {
        return (num){.type = NUM_BOOL, .value.bVal=(llA == 0)};
    }
    fatalError(0x1B, "Unable to find unary operator", -1);
    return (num){.type = NUM_BOOL, .value.bVal = false};
}
// Compares two numbers. returns 1 if they're not equal, 0 if they are.
int cmpNum(num a, num b) {
    pair p = convertNums(a, b); // convert them both to same value
    switch (a.type) {
        case NUM_BOOL: {return !(p.a.value.bVal == p.b.value.bVal);}
        case NUM_CHAR: {return !(p.a.value.cVal == p.b.value.cVal);}
        case NUM_UCHAR: {return !(p.a.value.ucVal == p.b.value.ucVal);}
        case NUM_SHORT: {return !(p.a.value.sVal == p.b.value.sVal);}
        case NUM_USHORT: {return !(p.a.value.usVal == p.b.value.usVal);}
        case NUM_INT: {return !(p.a.value.iVal == p.b.value.iVal);}
        case NUM_UINT: {return !(p.a.value.uiVal == p.b.value.uiVal);}
        case NUM_LONG: {return !(p.a.value.lVal == p.b.value.lVal);}
        case NUM_ULONG: {return !(p.a.value.ulVal == p.b.value.ulVal);}
        case NUM_FLOAT: {return !(p.a.value.fVal == p.b.value.fVal);}
        case NUM_DOUBLE: {return !(p.a.value.dVal == p.b.value.dVal);}
        case NUM_LONGDOUBLE: {return !(p.a.value.ldVal == p.b.value.ldVal);}
    }
    return 1;
}