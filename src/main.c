/*
--- Main.c ---
This file handles the connections between the several components in this project (Essentially, it feeds each parts' output into the next part as input)
--- Important files: ---
Main.c - Reads the file content, & feeds it into the lexer.
lexer.c - Turns the file's plaintext content into an array of tokens (Simple structs containing just a token-type enum, & the characters that make it up.)
parser.c - Turns the array of tokens into a tree-like structure that preserves order of operations, and further simplifies the program.
bytecoder.c - Turns the tree back into a linear array (But this time, each element is much simpler, containing operations to do on the stack)
vm.c - Actually executes the instrutions
--- Less important files: ---
sysFunctions.c - Contains C implementations of the Aurum's standard library (this is where you'd find implementations of print(), scan(), io functions, etc)
winInclude.c - Contained some functions for interacting with Window's API, but now just contains fatalError(), which is responsible for throwing errors.
Tundora.c - This is an old library that I made for complex data structures. Anytime you see the List type, this is what its from.
bytecoder.h - In addition to containing bytecoder.c's structs, it also contains the majority of standard library insertions / function declarations. If you still have questions after looking at sysFunctions.c, then you should look here.
num.c - Contains some functions for dealing with numbers (If you're interested in how this language deals with storing numbers, this is a good place to go)
*/

// C standard imports
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
// Aurum headers
#include "processing/lexer.h"
#include "processing/parser.h"
#include "processing/bytecoder.h"
#include "processing/vm.h"
#include "misc/winInclude.h"
#include "processing/aurx.h"

void getDirectory(char* path);
char* getAurX(char* n) {
    char* buffer = malloc(strlen(n)+2);
    sprintf(buffer, "%sx", n);
    return buffer;
}
bool isExtension(char* buffer, char* ext) {
    char* toTake = malloc(strlen(ext)+1); toTake[0] = '\0';
    int idx = 0;
    for (int i = strlen(buffer)-strlen(ext); i < strlen(buffer); i++) {
        toTake[idx++] = buffer[i];
        toTake[idx] = '\0';
    }
    bool isTrue = strcmp(toTake, ext) == 0;
    free(toTake);
    return isTrue;
}
int main(int argc, char **argv) {
    globalPool = createPool(1000*1000*sizeof(typedValue));
    // Ensure that char, short, int, int64_t, float, double, & long double are all correct sizes.
    if (sizeof(char) != 1) fatalError(0x8, "Invalid size of char (expected 1). Check the environment in which you are building.", -1); 
    if (sizeof(short) != 2) fatalError(0x8, "Invalid size of short (expected 2). Check the environment in which you are building.", -1); 
    if (sizeof(int) != 4) fatalError(0x8, "Invalid size of int (expected 4). Check the environment in which you are building.", -1); 
    if (sizeof(int64_t) != 8) fatalError(0x8, "Invalid size of int64_t (expected 8). Check the environment in which you are building.", -1); 
    if (sizeof(float) != 4) fatalError(0x8, "Invalid size of float (expected 4). Check the environment in which you are building.", -1); 
    if (sizeof(double) != 8) fatalError(0x8, "Invalid size of double (expected 8). Check the environment in which you are building.", -1); 
    if (sizeof(long double) != 16) fatalError(0x8, "Invalid size of long double (expected 16). Check the environment in which you are building.", -1); 
    if (isDebug) printf("%lld", sizeof(num));
    bool saurX = saveAsAurX; // saveAsAurX is an override for debugging.
    if (argc == 3) {
        if (strcmp(argv[2], "-x") == 0) saurX = true;
    }
    // Set console output to 1252; will be future addition to support other stuff.
    setConsoleOutput(1252);
    
    // Set buffer to the path of the file that will be executed;
    char buffer[500]; buffer[0] = '\0';
    if (isDebug && !isDebugX) {
        sprintf(buffer, "./project/main.aur");
    }
    else if (isDebug && isDebugX) {
        sprintf(buffer, "./project/main.aurx");
    }
    else if (argc <= 1) {
        scanf("%s", buffer);
    }
    else {
        strcpy(buffer, argv[1]);
    }
    // Checks if the file to be executed has the .aur extension.
    bool exAurX = false;
    if (isExtension(buffer, executableExtension)) {
        exAurX = true;
    }
    else if (!isExtension(buffer, extension)) fatalError(0x6, "", -1);
    fromAurX(exAurX, buffer); // If exAurX is true, then it'll execute the AurX file. Otherwise, it'll try to manually parse the file.
    // Gets the file's plaintext content.
    if (isDebug) printf("%s", buffer);
    getDirectory(buffer);
    char* file = readTextFile(buffer);
    // Tokenizes the file into an array of tokens.
    if (isDebug) printf("%s", file);
    List* tokens = tokenize(file);
    // Parses the file into a tree, establishing an order of operations for the lines to be executed
    ASTNode* program = parseTokenList(tokens);
    if (isDebug) printAST(program, 0);
    // Converts it back into an array of instructions (which this time are extremely simple, containing only a numeric instruction id, and 0-2 parameters).
    byteCode* bc = convertToBytecode(program);
    
    if (saurX) {
        char* toSaveAs = getAurX(buffer);
        toAurX(bc, toSaveAs);
        free(toSaveAs);
        return 0;
    }
    if (isDebug) printBytecode(*bc, false, true);   
    // Executes the instructions
    executeBytecode(bc);
    return 0;
}
// Gets the path's directory by removing the last "\\X" in the path.
void getDirectory(char* path) {
    if (path == NULL) return;
    char buffer[500];
    buffer[0] = '\0';
    // Gets the index of the last slash in the file path, then cuts off that slash, & returns it.
    int lastSlashIdx = 0;
    for (int i = 0; i < strlen(path); i++) {if (path[i] == '\\' || path[i] == '/') lastSlashIdx = i;}
    for (int i = 0; i < strlen(path); i++) {
        if (i == lastSlashIdx) break;
        buffer[i] = path[i];
        buffer[i+1] = '\0';
    }
    projectFolder = strdup(buffer);
}