/*
-- winInclude.c ---
This file has getACP & setConsoleOutput, which are basically just changed-names from the windows.h functions.
fatalError() is used all across the language, and it essentially just throws errors.
*/
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <stdbool.h>
bool isDebug = false;
int getACP() {
    return GetACP();
}
void setConsoleOutput(int n) {
    SetConsoleOutputCP(n);
}
void fatalError(int code, char* n, int linen) {
    char* errstr = n;
    if (isDebug) {
        printf("Error: ");
        int n;
        scanf_s("%d", &n);
    }
    // If no custom message is provided, get a message using its error code.
    if (strlen(n) == 0) {
        switch (linen) {
            case 0x6: {errstr = "Cannot open files with extensions other than .aur or .aurx"; break;}
            case 0x7: {errstr = "Cannot assign to const after declaration."; break;}
            case 0x8: {errstr = "Invalid size of number type."; break;}
            case 0x9: {errstr = "Right parentheses not found"; break;}
            case 0xA: {errstr = "Unable to parse expression"; break;}
            case 0xB: {errstr = "Invalid input into pdtToNumtype()"; break;}
            case 0xC: {errstr = "Import statement error"; break;}
            case 0xD: {errstr = "Import not resolved"; break;}
            case 0xE: {errstr = "Return type not resolved"; break;}
            case 0xF: {errstr = "Parser stalled"; break;}
            case 0x10: {errstr = "Cannot have two identifiers within the same scope with the same name"; break;}
            case 0x11: {errstr = "Invalid type for new number"; break;}
            case 0x12: {errstr = "Invalid conversion for a number"; break;}
            case 0x13: {errstr = "Cannot raise an integer by a <0 number. Convert it to a decimal."; break;}
            case 0x14: {errstr = ""; break;}
            case 0x15: {errstr = "Cannot use a decimal for the first input of a modulo operation (e.x 2.2%5)"; break;}
            case 0x16: {errstr = "Cannot divide by zero"; break;}
            case 0x17: {errstr = "Cannot raise a negative number by a fractional exponent. Try using the complexNumber struct"; break;}
            case 0x18: {errstr = "Floor division only works with ints"; break;}
            case 0x19: {errstr = "Bitwise operators (^, |, &, ~, >>, <<) only work with integers"; break;}
            case 0x1A: {errstr = "Unable to find a binary operator"; break;}
            case 0x1B: {errstr = "Unable to find unary operator"; break;}
            case 0x1C: {errstr = "Unable to find default type of a typedValue"; break;}
            case 0x1D: {errstr = "Unknown ASTNode type"; break;}
            case 0x1E: {errstr = "Unknown identifier"; break;}
            case 0x1F: {errstr = "Structs cannot be declared within a function."; break;}
            case 0x20: {errstr = "Unknown parameter for #."; break;}
            case 0x21: {errstr = "Invalid array access"; break;}
            case 0x22: {errstr = "Invalid struct access"; break;}
            case 0x23: {errstr = "Invalid function call"; break;}
            case 0x24: {errstr = "Arrays cannot have multiple types"; break;}
            case 0x25: {errstr = "Only struct, variable, or function declarations are allowed in premain"; break;}
            case 0x26: {errstr = "Variable already exists"; break;}
            case 0x27: {errstr = "Cannot find struct"; break;}
            case 0x28: {errstr = "Unknown error with variable declaration"; break;}
            case 0x29: {errstr = "Invalid datatype"; break;}
            case 0x2A: {errstr = "Invalid assignment"; break;}
            case 0x2B: {errstr = "Invalid struct name"; break;}
            case 0x2C: {errstr = "Struct assignment error"; break;}
            case 0x2E: {errstr = "Unknown Assignment error"; break;}
            case 0x2F: {errstr = "Invalid index assignment"; break;}
            case 0x30: {errstr = "Invalid input for a system function"; break;}
            case 0x31: {errstr = "Invalid string parse"; break;}
            case 0x32: {errstr = "Invalid type conversion"; break;}
            case 0x33: {errstr = "toExp()/toSciNo() Input Error: Base cannot be negative or 1"; break;}
            case 0x34: {errstr = "Cannot use array.removeElement() or array.pop() when its length is zero"; break;}
            case 0x35: {errstr = "Cannot sort an array with nonnumerical element"; break;}
            case 0x36: {errstr = "Format Parsing error. Check your format string"; break;}
            case 0x37: {errstr = "Cannot use %%u on a float"; break;}
            case 0x38: {errstr = "Number of parameters passed differs from numbers of parameters suggested in format string"; break;}
            case 0x39: {errstr = "Thrown Error"; break;}
            case 0x40: {errstr = "Non-Decimal bases work only with integers."; break;}
            case 0x41: {errstr = "Library imported requires math to also be imported before this library."; break;}
            case 0x42: {errstr = "No such file or directory"; break;}
            case 0x43: {errstr = "File already exists"; break;}
            case 0x44: {errstr = "Path is a directory"; break;}
            case 0x45: {errstr = "File name too long"; break;}
            case 0x46: {errstr = "Too many symbolic links"; break;}
            case 0x47: {errstr = "Permission denied"; break;}
            case 0x48: {errstr = "Read-only file system"; break;}
            case 0x49: {errstr = "Text file busy"; break;}
            case 0x4A: {errstr = "Too many open files"; break;}
            case 0x4B: {errstr = "File table overflow (too many total files)"; break;}
            case 0x4C: {errstr = "Cannot allocate memory to file"; break;}
            case 0x4D: {errstr = "No space left on device"; break;}
            case 0x4E: {errstr = "Invalid mode"; break;}
            case 0x4F: {errstr = "File too large"; break;}
            case 0x50: {errstr = "Invalid operation parameters"; break;}
            default: {
                errstr = "Unknown error"; break;
            }
        }
    }
    

   char nn[50];
   nn[0] = '\0';
   if (linen == -1) sprintf(nn, "unknown");
   else sprintf(nn, "%d", linen);
   
   char m[50];
   m[0] = '\0';
   if (code == 0x40) sprintf(m, "L-ERR");
   else if (code <= 0x1B) sprintf(m, "P-ERR:");
   else if (code <= 0x2E) sprintf(m, "B-ERR");
   else sprintf(m, "V-ERR");
   printf("\n%s 0x%X: %s; Line number: %s\n", m, code, errstr, nn);
   if (isDebug) {int w;scanf_s("%d", &w);}
   exit(1);
}