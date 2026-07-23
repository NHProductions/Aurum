/*
--- bytecoder.c ---
This file is responsible for turning ASTNodes back into a linear format.
It does this by turning ASTNodes into an array of chunks, which are arrays of instructions.
These instructions are extremely simple, meant to operate on a stack.

Using the fizzbuzz example from parser.c, here's what the bytecode version of that would be.
Beside each instruction is how they effect the stack.
If you don't know what a stack is, it's essentially a data structure wherein operations affect the 1st index (operation 0)
Example: stack=[0,2,4,5,5]
If I were to load another number onto that stack (say 10), it would be stack=[10,0,2,4,5,5], instead of [0,2,4,5,5,10] like it would be if it were an array.
Essentially, it makes it simpler to calculate insertions/removals, as it's just removing the 1st element, instead of calculating the length.

Sections are labelled as they correspond to the original code.
    for loop
        Initialization: i=1
            0. LOAD CONST 1 - Load 1 onto the stack
            1. STORE LOCAL 1 - Store 1 into local 1 (i)
            2. JUMP 11 - Jump to line 11 (1st time the for loop happens)
        Update i: i++
            3. LOAD LOCAL 1 - Load i onto the stack
            4. LOAD CONST 37 - Load 1 onto the stack
            5. ADD - Remove i & 1 from the stack. add i+1 to the stack.
            6. STORE LOCAL 1 - Store i+1 into i.
        Check condition: i <= amt
            7. LOAD LOCAL 1 - Load i onto the stack
            8. LOAD LOCAL 0 - Load amt onto the stack
            9. LEQTHAN - Remove i and amt from the stack. add (i <= amt) onto the stack.
            10. JUMP IF FALSE 63 - Remove (i <= amt) from the stack. If that equals zero, jump to line 63 (ending the loop).
        1st if condition (i % 3 == 0) && (i % 5 != 0)
            1st part of the and statement (i % 3 == 0)
                11. LOAD LOCAL 1 - Load i onto the stack
                12. LOAD CONST 38 - Load 3 onto the stack
                13. MOD - Remove i & 3 from the stack, and put i % 3 onto the stack
                14. LOAD CONST 0 - Load 0
                15. EQUALS - Remove 0 & (i % 3), and add (i % 3 == 0) onto the stack.
            2nd part of the and statement (i % 5 != 0)
                16. LOAD LOCAL 1 - Load i onto the stack
                17. LOAD CONST 39 - Load 5 onto the stack
                18. MOD - Remove i & 5 from the stack, and put i % 5 onto the stack.
                19. LOAD CONST 0 - Load zero onto the stack
                20. EQUALS - Remove (i % 5) & zero from the stack. Add (i % 5 == 0) to the stack.
                21. NOT - Remove (i % 5 == 0) from the stack. Add !(i % 5 == 0)
            22. AND - Remove !(i % 5 == 0) and (i % 3 == 0) from the stack. Add (!(i % 5 == 0) && (i % 3 == 0))
            23. JUMP IF FALSE 27 - Remove (!(i % 5 == 0) && (i % 3 == 0)) from the stack. If that value is 0, jump to 27.
        1st if block (print "fizz")
            24. LOAD CONST 40 - Add the string "fizz" to the stack.
            25. CALL 7 1 - Remove 1 thing from the stack, and call function 7 (print)
        2nd if condition (i % 5 == 0) && (i % 3 != 0)
            Load 1st part of the and statement (i % 5 == 0)
                26. JUMP 62 - Jump to line 62.
                27. LOAD LOCAL 1 - Load i onto the stack
                28. LOAD CONST 39 - Load 5 onto the stack
                29. MOD - Remove i & 5 from the stack. Add (i % 5) onto the stack.
                30. LOAD CONST 0 - Load zero onto the stack.
                31. EQUALS - Remove (i % 5) and 0 from the stack. add (i % 5 == 0) to the stack.
            Load 2nd part of the and statement (i % 3 != 0)
                32. LOAD LOCAL 1 - Load i onto the stack.
                33. LOAD CONST 38 - Load 3 onto the stack.
                34. MOD - Remove i & 3 from the stack. add (i % 3) to the stack
                35. LOAD CONST 0 - Load zero onto the stack.
                36. EQUALS - Remove (i % 3) & zero from the stack. add (i % 3 == 0) to the stack.
                37. NOT - Remove (i % 3 == 0) from the stack. Add !(i % 3 == 0)
            38. AND - Remove !(i % 3 == 0) and (i % 5 == 0) from the stack. Add (!(i % 3 == 0) && (i % 5 == 0)) to the stack.
            39. JUMP IF FALSE 43 - Remove (!(i % 3 == 0) && (i % 5 == 0)) from the stack. If that equals zero, jump to line 43.
        2nd if block (print "buzz"):
            40. LOAD CONST 41 - Load "buzz" onto the stack.
            41. CALL 7 1 - Remove 1 parameter from the stack and call function 7 (print)
            42. JUMP 62 - Jump to line 62.
        3rd if condition (i % 5 == 0 && i % 3 == 0):
            Load 1st part of the and statement (i % 5 == 0)
                43. LOAD LOCAL 1 - Load i onto the stack
                44. LOAD CONST 39 - Load 5 onto the stack
                45. MOD - Remove i & 5 from the stack. Add (i % 5) to the stack.
                46. LOAD CONST 0 - Load zero onto the stack.
                47. EQUALS - Remove (i % 5) & zero from the stack. add (i % 5 == 0)
            Load 2nd part of the and statement (i % 3 == 0)
                48. LOAD LOCAL 1 - Load i onto the stack.
                49. LOAD CONST 38 - Load 3 onto the stack.
                50. MOD - Remove i & 3 from the stack. add (i % 3) to the stack.
                51. LOAD CONST 0 - Load zero onto the stack.
                52. EQUALS - Remove (i % 3) and zero from the stack. Add (i % 3 == 0) to the stack.
            53. AND - Remove (i % 3 == 0) and (i % 5 == 0) from the stack. add (i % 3 == 0 && i % 5 == 0) to the stack
            54. JUMP IF FALSE 58 - Remove (i % 3 == 0 && i % 5 == 0) from the stack. Jump to line 58 if it's equal to zero.
        3rd if block (print "fizzbuzz"):
            55. LOAD CONST 42 - Add "fizzbuzz" to the stack.
            56. CALL 7 1 - Remove "fizzbuzz" from the stack, and call function 7 (print).
            57. JUMP 62 - Jump to line 62.
        else block (print the number):
            58. LOAD CONST 43 - Load "%i" onto the stack.
            59. LOAD LOCAL 1 - Load i onto the stack
            60. CALL 7 2 - Take two parameters from the stack (%i and i), and call print().
            61. JUMP 62 - Jump to line 62
        start the loop again:
            62. JUMP 3 - Jump to line 3
While this may seem like it makes the code longer, it actually does the opposite.
Each ASTNode is 64 bytes (which is a lot); whereas each instruction is 16 bytes (4 times smaller), 
and with each instruction being simpler, it makes actually executing each instruction much simpler too.

Files to check next: bytecoder.h vm.c sysFunctions.c
*/

// standard library imports:
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
// Aurum imports:
#include "Tundora.h"
#include "parser.h"
#include "lexer.h"
#include "string.h"
#include "bytecoder.h"
#include "winInclude.h"
#include "num.h"

#include "mathFunctions.h"
#include "timeFunctions.h"
#include "randomFunctions.h"
#include "complexFunctions.h"
#include "lalgFunctions.h"
#include "ioFunctions.h"
// Globals; projectFolder is the folder in which main.aur is located, and it's used for getting other files.
char* toExec = NULL;
char* projectFolder = NULL;


void printBytecode(byteCode c, bool numsOnly, bool numberInstructions);
// When given a chunkidx and a bytecode struct, get the list of instructions from that chunk.
List* getInstructions(byteCode c, int chunkIdx) {
    return ((chunk*)List_GetElement(c.chunks, chunkIdx)->data)->instructions;
}
// Unary instruction: Only 1 argument (e.x LOAD_CONST n).
void addUnaryInstruction(byteCode c, char code, int arg, int chunkIdx) {
    instruction* toAdd = malloc(sizeof(instruction));
    toAdd->args = malloc(sizeof(int) * 1);
    toAdd->args[0] = arg;
    toAdd->argc = 1;
    toAdd->code = code;
    List_AppendElement(getInstructions(c, chunkIdx), toAdd);
}
// Nullary instruction: No argument (e.x ADD)
void addNullaryInstruction(byteCode c, char code, int chunkIdx) {
    instruction* toAdd = malloc(sizeof(instruction));
    toAdd->args = NULL;
    toAdd->argc = 0;
    toAdd->code = code;
    List_AppendElement(getInstructions(c, chunkIdx), toAdd);
}
// Binary instruction: 2 arguments (e.x CALL 7 2)
void addBinaryInstruction(byteCode c, char code, int arg0, int arg1, int chunkIdx) {
    instruction* toAdd = malloc(sizeof(instruction));
    toAdd->args = malloc(sizeof(int)*2);
    toAdd->argc = 2;
    toAdd->args[0] = arg0;
    toAdd->args[1] = arg1;
    toAdd->code = code;
    List_AppendElement(getInstructions(c, chunkIdx), toAdd);
}
// Converts numType to PDT type.
int numTypeToPDT(int t) {
    switch (t) {
        case NUM_BOOL: {return PDT_BOOL;}
        case NUM_CHAR: {return PDT_CHAR;}
        case NUM_DOUBLE: {return PDT_DOUBLE;}
        case NUM_FLOAT: {return PDT_FLOAT;}
        case NUM_INT: {return PDT_INT;}
        case NUM_LONG: {return PDT_LONG;}
        case NUM_LONGDOUBLE: {return PDT_LONGDOUBLE;}
        case NUM_SHORT: {return PDT_SHORT;}
        case NUM_UCHAR: {return PDT_CHAR;}
        case NUM_UINT: {return PDT_INT;}
        case NUM_ULONG: {return PDT_ULONG;}
        case NUM_USHORT: {return PDT_SHORT;}
    }
    return 0;
}
// Struct that has scopeinfo in it (such as where to jump if continue was called).
typedef struct {
    List* breakStatements;
    int continueTgt;
    List* locals;
} scopeInfo;
// When given a num array and a length, converts that into a string.
char* charArrToStr(num* arr, int len) {
    char* toReturn = malloc((len+1)*sizeof(char));
    for (int i = 0; i < len; i++) {
        toReturn[i] = arr[i].value.cVal;
        if (i == len-1 && arr[i].value.cVal != 0) {
            free(toReturn);
            return NULL;
        }
    }
    toReturn[len] = '\0';
    return toReturn;
}
// Returns if funcName is in the format of constructors (%s_%s). If it is, returns the struct in which it's a constructor for.
structDefinition* isConstructor(char* funcName, byteCode c) {
    int underscoreIdx = -1;
    char* sideA = malloc(strlen(funcName)+1);
    sideA[0] = '\0';
    char* sideB = malloc(strlen(funcName)+1);
    sideB[0] = '\0';
    for (int i = 0; i < strlen(funcName); i++) {
        if (funcName[i] == '_') {
            underscoreIdx = i;
        }
        else if (underscoreIdx == -1) {
            int Alen = strlen(sideA);
            sideA[Alen] = funcName[i];
            sideA[Alen+1] = '\0';
        }
        else if (underscoreIdx != -1) {
            int Blen = strlen(sideB);
            sideB[Blen] = funcName[i];
            sideB[Blen+1] = '\0';
        }
    }
    if (strcmp(sideA, sideB) != 0) {
        free(sideB);
        free(sideA);
        return NULL;
    }
    for (int i = 0; i < c.structDefs->length; i++) {
        structDefinition* sd = (structDefinition*)List_GetElement(c.structDefs, i)->data;
        if (strcmp(sd->name, sideA) == 0) {
            free(sideB);
            free(sideA);
            return sd;
        }
    }
    free(sideB);
    free(sideA);
    return NULL;
}
// When given a struct name, gets its definition.
structDefinition* getStructDefViaName(char* name, byteCode c) {
    if (name == NULL) return NULL;
    for (int i = 0; i < c.structDefs->length; i++) {
        if (strcmp(((structDefinition*)List_GetElement(c.structDefs, i)->data)->name, name) == 0) {
            return (structDefinition*)List_GetElement(c.structDefs, i)->data;
        }
    }
    return NULL;
}
// Finds an identifier when given bytecode, scopeinfo, and name of the identifier.
typedef struct {
    bool isGlobal;
    int index;
    bool exists;
    vmVariable* ptr;
} identifierLocation;
identifierLocation getIdentifier(byteCode c, scopeInfo s, char* name) {
    if (s.locals != NULL) {
        for (int i = 0; i < s.locals->length; i++) {
            vmVariable* tv = List_GetElement(s.locals, i)->data;
            if (strcmp(tv->name, name) == 0) {
                return (identifierLocation){.exists = true, .isGlobal = false, .index = i, .ptr = tv};
            }
        }
    }
    for (int i = 0; i < c.globals->length; i++) {
            vmVariable* tv = List_GetElement(c.globals, i)->data;
            if (strcmp(tv->name, name) == 0) {
                return (identifierLocation){.exists = true, .isGlobal = true, .index = i, .ptr = tv};
            }
    }
    return (identifierLocation){.exists = false, .isGlobal = false, .index = 0, .ptr = NULL};
}
// When given a function name, get it's bcFunction definition (it's name, how many arguments it has, it's return struct, and if it's a system function)
bcFunction* getFunctionIdentifier(char* n, byteCode c) {
    bcFunction* bcf = NULL;
    for (int i = 0; i < c.functionIdentifiers->length; i++) {
            bcFunction* b = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
            if (strcmp(b->name, n) == 0) {
                bcf = b;
                break;
            } 
    }
    return bcf;
}
// When given a complex statement such as n.a.b or n.a().b, get it's structDefinition.
structDefinition* getStructType(ASTNode* currentNode, byteCode c, scopeInfo s) {
    // n.a
    if (currentNode->type == AST_STRUCTACCESS && currentNode->left->type == AST_IDENTIFIER) { // e.x [struct.structAccess].structAccess
        
        char* structName = NULL;
        char* parentVarName = currentNode->left->value.nameVal;
        int idx = -1;
        bool isGlobal = false;
        vmVariable* v = NULL;
        if (s.locals != NULL) {
            for (int i = 0; i < s.locals->length; i++) {
                vmVariable* vmv = (vmVariable*)List_GetElement(s.locals, i)->data;
                if (strcmp(vmv->name, parentVarName) == 0) {
                    structName = vmv->structType;
                    idx = i;
                    isGlobal = false;
                    v = vmv;
                    break;
                }
            }
        }
        if (idx == -1) {
            for (int i = 0; i < c.globals->length; i++) {
                vmVariable* vmv = (vmVariable*)List_GetElement(c.globals, i)->data;
                if (strcmp(vmv->name, parentVarName) == 0) {
                    structName = vmv->structType;
                    idx = i;
                    isGlobal = true;
                    v = vmv;
                    break;
                }
            }
        }
        if (structName == NULL) { if (isDebug) {printf("Structname null");} return NULL;}
        structDefinition* structDef = NULL;
        for (int i = 0; i < c.structDefs->length; i++) {
            structDefinition* sd = (structDefinition*)List_GetElement(c.structDefs, i)->data;
            if (strcmp(sd->name, structName) == 0) {
                structDef = sd;
                for (int j = 0; j < sd->fields->length; j++) {
                    char* currentField = (char*)List_GetElement(sd->fields, j)->data;
                    structTypes* fieldType = (structTypes*)List_GetElement(sd->fieldTypes, j)->data;
                    if (strcmp(currentField, currentNode->right->value.nameVal) == 0) {
                        if (!fieldType->isPDT) {
                            return getStructDefViaName(fieldType->type.name, c);
                        }
                        if (fieldType->isArray) return getStructDefViaName("array", c);
                        else if (fieldType->type.pdtType == PDT_STRING) return getStructDefViaName("string", c);
                        return NULL;
                    }
                }
            }
        }
        char n[500]; n[0] = '\0'; sprintf(n, "%s_%s", structDef->name, currentNode->right->value.nameVal); // check function declarations
        for (int i = 0; i < c.functionIdentifiers->length; i++) {
            bcFunction* bcf = List_GetElement(c.functionIdentifiers, i)->data;
            if (strcmp(bcf->name, n) == 0) {
                return getStructDefViaName(bcf->returnStruct, c);
            }
        }
        return NULL;
    }
    // n.a.b
    else if (currentNode->type == AST_STRUCTACCESS && currentNode->left->type == AST_STRUCTACCESS) {
        structDefinition* sd = getStructType(currentNode->left, c, s);
        if (sd == NULL) {return NULL;};
        for (int i = 0; i < sd->fields->length; i++) {
            char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
            structTypes* fieldType = (structTypes*)List_GetElement(sd->fieldTypes, i)->data;
            if (strcmp(fieldName, currentNode->right->value.nameVal) == 0) {
                if (!fieldType->isPDT) {
                    return getStructDefViaName(fieldType->type.name,c);
                }
                else return NULL;
            }
        }
        return NULL;
    }
    // n[0]
    else if (currentNode->type == AST_ARRAYACCESS && currentNode->left->type == AST_IDENTIFIER) {
        char* structName = NULL;
        char* parentVarName = currentNode->left->value.nameVal;
        int idx = -1;
        bool isGlobal = false;
        bool arrayVerified = false;
        if (s.locals != NULL) {
            for (int i = 0; i < s.locals->length; i++) {
                vmVariable* vmv = (vmVariable*)List_GetElement(s.locals, i)->data;
                if (strcmp(vmv->name, parentVarName) == 0) {
                    structName = vmv->structType;
                    idx = i;
                    isGlobal = false;
                    arrayVerified = vmv->isArray;
                    break;
                }
            }
        }
        if (idx == -1) {
            for (int i = 0; i < c.globals->length; i++) {
                vmVariable* vmv = (vmVariable*)List_GetElement(c.globals, i)->data;
                if (strcmp(vmv->name, parentVarName) == 0) {
                    structName = vmv->structType;
                    idx = i;
                    isGlobal = true;
                    arrayVerified = vmv->isArray;
                    break;
                }
            }
        }
        if (structName == NULL) {if (isDebug) {printf("NULL NAME");} return NULL;}
        if (!arrayVerified) {if (isDebug) {printf("Fake array");} return NULL;} // not actually an array
        for (int i = 0; i < c.structDefs->length; i++) {
            structDefinition* sd = List_GetElement(c.structDefs, i)->data;
            if (strcmp(sd->name, structName) == 0) {
                return getStructDefViaName(sd->name, c);
            }
        }
        return NULL;
    }
    // n.a[0]
    else if (currentNode->type == AST_STRUCTACCESS && currentNode->left->type == AST_ARRAYACCESS) {
        structDefinition* sd = getStructType(currentNode->left, c, s);
        
        if (sd == NULL) {return NULL;};
        for (int i = 0; i < sd->fields->length; i++) {
            char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
            structTypes* fieldType = (structTypes*)List_GetElement(sd->fieldTypes, i)->data;
            if (strcmp(fieldName, currentNode->right->value.nameVal) == 0) {
                if (!fieldType->isPDT) {
                    return getStructDefViaName(fieldType->type.name,c);
                }
                else return NULL;
            }
        }
        return NULL;
    }
    else if (currentNode->type == AST_FUNCCALL) {
        if (currentNode->left->type == AST_STRUCTACCESS) {
            structDefinition* sd = getStructType(currentNode->left, c, s);
            return sd;
            char n[500]; n[0] = '\0'; sprintf(n, "%s_%s", sd->name, currentNode->left->right->value.nameVal);
            bcFunction* funcDef = getFunctionIdentifier(n, c);
            if (funcDef == NULL) return NULL;
            return getStructDefViaName(funcDef->returnStruct, c);
        }
        else if (currentNode->left->type == AST_IDENTIFIER) {
            bcFunction* funcDef = getFunctionIdentifier(currentNode->left->value.nameVal, c);
            if (funcDef == NULL) return NULL;
            return getStructDefViaName(funcDef->returnStruct, c);
        }
    }
    // a().b
    else if (currentNode->type == AST_STRUCTACCESS && currentNode->left->type == AST_FUNCCALL) {
        if (currentNode->left->left->type == AST_STRUCTACCESS) {
            return getStructType(currentNode->left, c, s);
        }
        else if (currentNode->left->left->type == AST_IDENTIFIER) {
            bcFunction* fer = getFunctionIdentifier(currentNode->left->left->value.nameVal, c);
            if (fer== NULL) return NULL;
            // currentNode.right has to be an identifier because this is a().b
            char* rightID = currentNode->right->value.nameVal;
            structDefinition* sd = getStructDefViaName(fer->returnStruct, c);
            // if sd.rightID exists, then return sd.
            for (int j = 0; j < sd->fields->length; j++) {
                char* fieldName = (char*)List_GetElement(sd->fields, j)->data;
                if (strcmp(fieldName, rightID) == 0) {
                    return sd;
                }
            }
            // if sd.rightFunc() exists, then return the struct definition of that.
            char buffer[50]; buffer[0] = '\0'; sprintf(buffer, "%s_%s", sd->name, rightID);
            for (int i = 0; i < c.functionIdentifiers->length; i++) {
                bcFunction* bcf = List_GetElement(c.functionIdentifiers, i)->data;
                if (strcmp(bcf->name, buffer) == 0) {
                    if (bcf->returnStruct == NULL) return NULL;
                    return getStructDefViaName(bcf->returnStruct, c);
                }

            }
                
            
        }
    }
    return NULL;
}
void toBytecode(byteCode c, ASTNode* currentNode, scopeInfo s, int chunkIdx);
// Gets an array assignment target so that multidimensional array assignments are parsed correctly.
static void toBytecodeArrayAssignmentTarget(byteCode c, ASTNode* currentNode, scopeInfo s, int chunkIdx, bool emitLoad) {
    if (currentNode == NULL) {
        fatalError(0x2D, "Invalid array access target.", -1);
        return;
    }
    if (currentNode->type != AST_ARRAYACCESS) {
        toBytecode(c, currentNode, s, chunkIdx);
        return;
    }

    if (currentNode->left != NULL) {
        if (currentNode->left->type == AST_ARRAYACCESS) {
            toBytecodeArrayAssignmentTarget(c, currentNode->left, s, chunkIdx, true);
        }
        else {
            toBytecode(c, currentNode->left, s, chunkIdx);
        }
    }

    if (currentNode->right != NULL) {
        toBytecode(c, currentNode->right, s, chunkIdx);
    }

    if (emitLoad) {
        addNullaryInstruction(c, OP_LOAD_IDX, chunkIdx);
    }
}
// Applies specifiers to vmVariables (such as const).
void applySpecifiers(ASTNode* cn, vmVariable* vmv) {
    if (cn->children == NULL) return;
    for (int i = 0; i < cn->children->length; i++) {
        ASTNode* child = List_GetElement(cn->children, i)->data;
        if (child->type == AST_IDENTIFIERSPECIFIER) {
            if (child->value.numberVal == VS_CONST) vmv->isConst = true;
        }
    }
}
// Does the same thing as applySpecifiers() but for structTypes.
void applySpecifiersStruct(ASTNode* cn, structTypes* st) {
    if (cn->children == NULL) return;
    for (int i = 0; i < cn->children->length; i++) {
        ASTNode* child = List_GetElement(cn->children, i)->data;
        if (child->type == AST_IDENTIFIERSPECIFIER) {
            if (child->value.numberVal == VS_CONST) st->isConst = true;
        }
    }
}
void toBytecode(byteCode c, ASTNode* currentNode, scopeInfo s, int chunkIdx) {
    switch (currentNode->type) {
        default: {
            fatalError(0x1D, "Unknown ASTNode type", -1);
            return;
        }
        /*
        Depending on scope, converts
        From:
            AST_IDENTIFIER(name)
        to either:
            LOAD GLOBAL 0
        or:
            LOAD LOCAL 0
        */
        case AST_IDENTIFIER: {
            // identifier -> LOAD_LOCAL n
            char* name = currentNode->value.nameVal;
            if (strcmp(name, "this") == 0) {
                addUnaryInstruction(c, OP_LOAD_LOCAL, 0, chunkIdx);
                ;
                return;
            }
            if (s.locals != NULL) {
                for (int i = 0; i < s.locals->length; i++) {
                    char* iName = ((vmVariable*)List_GetElement(s.locals, i)->data)->name;
                    if (strcmp(iName, name) == 0) {
                        addUnaryInstruction(c, OP_LOAD_LOCAL, i, chunkIdx);
                        return;
                    }
                }
            }
            // Identifier -> LOAD_GLOBAL n
            for (int i = 0; i < c.globals->length; i++) {
                char* iName = ((vmVariable*)List_GetElement(c.globals, i)->data)->name;
                if (strcmp(iName, name) == 0) {
                    addUnaryInstruction(c, OP_LOAD_GLOBAL, i, chunkIdx);
                    ;
                    return;
                }
            }
            // At this point, no global has been found, so it's an undefined identifier, which means there should be an error. (Global functions like print() will be added to globals before conversion starts.)
            fatalError(0x1E, "Undefined identifier", -1);
            break;
        }
        /*
        Most operators have their own instructions:
        AST_OPERATOR(+) -> OP_ADD
        Unary AST_OPERATOR(-) -> OP_NEG
        Binary AST_OPERATOR(-) -> OP_SUB
        AST_OPERATOR(*) -> OP_MUL
        AST_OPERATOR(/) -> OP_DIV
        AST_OPERATOR(<) -> OP_LTHAN
        AST_OPERATOR(>) -> OP_GTHAN
        AST_OPERATOR(<=) -> OP_LEQTHAN
        AST_OPERATOR(>=) -> OP_GEQTHAN
        AST_OPERATOR(==) -> OP_EQUALS
        AST_OPERATOR(||) -> OP_OR
        AST_OPERATOR(&&) -> OP_AND
        AST_OPERATOR(^^) -> OP_XOR
        unary AST_OPERATOR(!) -> OP_NOT
        AST_OPERATOR(%) -> OP_MOD
        AST_OPERATOR(<<) -> OP_LSHIFT
        AST_OPERATOR(>>) -> OP_RSHIFT
        AST_OPERATOR(|) -> OP_BOR
        AST_OPERATOR(&) -> OP_BAND
        AST_OPERATOR(^) -> OP_XOR
        unary AST_OPERATOR(~) -> OP_BNOT
        AST_OPERATOR(#) -> OP_ARRAY_LEN
        
        // and ** have different bytecode conversions
        they instead add a call to !opFunc(), which is a system function that does the logic for exponentiation and floor division
        AST_OPERATOR(//) -> OP_LOAD_CONST 1; OP_CALL 0 3
        AST_OPERATOR(**) -> OP_LOAD_CONST 0; OP_CALL 0 3
        */
        case AST_OPERATOR: {
            if (currentNode->left != NULL) toBytecode(c, currentNode->left, s, chunkIdx);
            if (currentNode->right != NULL) toBytecode(c, currentNode->right, s, chunkIdx);
            
            if (strcmp(currentNode->value.opVal, "+") == 0) {
                addNullaryInstruction(c, OP_ADD, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "-") == 0) {
                if (currentNode->left == NULL) addNullaryInstruction(c, OP_NEG, chunkIdx);
                else addNullaryInstruction(c, OP_SUB, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "#") == 0 ) {
                if (currentNode->left == NULL) addNullaryInstruction(c, OP_ARRAY_LEN, chunkIdx);
                else {fatalError(0x20, "Unknown parameter for #", -1);}
            }
            else if (strcmp(currentNode->value.opVal, "*") == 0) {
                addNullaryInstruction(c, OP_MUL, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "/") == 0) {
                addNullaryInstruction(c, OP_DIV, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, ">") == 0) {
                addNullaryInstruction(c, OP_GTHAN, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "<") == 0) {
                addNullaryInstruction(c, OP_LTHAN, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, ">=") == 0) {
                addNullaryInstruction(c, OP_GEQTHAN, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "<=") == 0) {
                addNullaryInstruction(c, OP_LEQTHAN, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "==") == 0) {
                addNullaryInstruction(c, OP_EQUALS, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "||") == 0) {
                addNullaryInstruction(c, OP_OR, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "&&") == 0) {
                addNullaryInstruction(c, OP_AND, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "!") == 0) {
                addNullaryInstruction(c, OP_NOT, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "%") == 0) {
                addNullaryInstruction(c, OP_MOD, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, ">>") == 0) {
                addNullaryInstruction(c, OP_RSHIFT, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, ">>") == 0) {
                addNullaryInstruction(c, OP_LSHIFT, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "|") == 0) {
                addNullaryInstruction(c, OP_BOR, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "&") == 0) {
                addNullaryInstruction(c, OP_BAND, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "^") == 0) {
                addNullaryInstruction(c, OP_BXOR, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "~") == 0) {
                if (currentNode->left == NULL) addNullaryInstruction(c, OP_BNOT, chunkIdx);
                else {fatalError(0x20, "Unknown parameter for ~", -1);}
            }
            else if (strcmp(currentNode->value.opVal, "**") == 0) {
                // 2 ** 5 -> pow(2, 5);
                addUnaryInstruction(c, OP_LOAD_CONST, 0, chunkIdx); // a**b -> !opFunc(a,b,0)
                addBinaryInstruction(c, OP_CALL, 0, 3, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "//") == 0) {
                // 2 // 5 -> pow(2, 5);
                addUnaryInstruction(c, OP_LOAD_CONST, 1, chunkIdx); // a//b -> !opFunc(a,b,1)
                addBinaryInstruction(c, OP_CALL, 0, 3, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "!=") == 0) {
                addNullaryInstruction(c, OP_EQUALS, chunkIdx);
                addNullaryInstruction(c, OP_NOT, chunkIdx);
            }
            else if (strcmp(currentNode->value.opVal, "!") == 0) {
                addNullaryInstruction(c, OP_NOT, chunkIdx);
            }
            break;
        }
        /*
        Turns AST_STRUCT into a structDefinition* (adding it to the list of current struct definitions),
        and converts its function declarations into bytecode chunks.
        Chunks converted in this way are named as structName_functionName.
        */
        case AST_STRUCT: {
            if (chunkIdx != 0) {fatalError(0x1F, "Structs cannot be declared within a function.", -1);}
            structDefinition* sd = malloc(sizeof(structDefinition));
            *sd = (structDefinition){
                .fields = malloc(sizeof(List)),
                .fieldTypes = malloc(sizeof(List)),
                .name = currentNode->value.nameVal
            };
            *sd->fields = NewList();
            *sd->fieldTypes = NewList();
            if (currentNode->children == NULL) {List_AppendElement(c.structDefs, sd); return;}
            if (currentNode->children->length == 0) {List_AppendElement(c.structDefs, sd); return;}

            for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* node = (ASTNode*)List_GetElement(currentNode->children, i)->data;
                if (node->type == AST_VARDECLARATION) {
                    List_AppendElement(sd->fields, node->right->value.nameVal);
                    structTypes* st = malloc(sizeof(structTypes));
                    *st = (structTypes){
                        .isPDT = (node->left->type == AST_DATATYPE),
                        .type.pdtType = 0,
                        .isArray = node->left->left == NULL ? false : node->left->left->type == AST_ARRAYASSIGNMENT
                    };
                    applySpecifiersStruct(node->left, st);
                    if (st->isPDT) st->type.pdtType = node->left->value.numberVal;
                    else st->type.name = node->left->value.nameVal;
                    List_AppendElement(sd->fieldTypes, st);
                }
            }
            List_AppendElement(c.structDefs, sd);
            for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* node = (ASTNode*)List_GetElement(currentNode->children, i)->data;
                if (node->type == AST_FUNCDEL) {
                    // name = %s_NAME; where %s is the function name.
                    ASTNode* modifiedFuncDel = newASTNode();
                    *modifiedFuncDel = (ASTNode){
                        .children = node->children,
                        .left = newASTNode(),
                        .right = node->right,
                        .type = AST_FUNCDEL,
                        .value.anyVal = NULL,
                    };
                    modifiedFuncDel->left->type = AST_IDENTIFIER;
                    modifiedFuncDel->left->value.nameVal = malloc(strlen(node->left->value.nameVal) + 2 + strlen(currentNode->value.nameVal) );
                    modifiedFuncDel->left->value.nameVal[0] = '\0';
                    sprintf(modifiedFuncDel->left->value.nameVal, "%s_%s", currentNode->value.nameVal, node->left->value.nameVal);
                    ASTNode* thisInsertion = newASTNode();
                    *thisInsertion = (ASTNode){
                        .children = malloc(sizeof(List)),
                        .left = newASTNode(),
                        .right = newASTNode(),
                        .type = AST_PARAM,
                        .value.anyVal = NULL,
                    };
                    *thisInsertion->children = NewList();
                    ASTNode* datatypeChild = newASTNode(); *datatypeChild = (ASTNode){
                        .children = NULL,
                        .type = AST_COMPLEXDATATYPE,
                        .value.nameVal = currentNode->value.nameVal,
                        .left = NULL,
                        .right = NULL
                    }; List_AppendElement(thisInsertion->children, datatypeChild);
                    ASTNode* identifierChild = newASTNode(); *identifierChild = (ASTNode){
                        .children = NULL,
                        .type = AST_IDENTIFIER,
                        .value.nameVal = strdup("this"),
                        .left = NULL,
                        .right = NULL,
                    }; List_AppendElement(thisInsertion->children, identifierChild);
                    List_InsertElement(modifiedFuncDel->children, 0, thisInsertion);
                    toBytecode(c, modifiedFuncDel, s, chunkIdx);
                }
            }
            ;
            break;
        }
        /*
        Turns AST_STRUCTACCESS into OP_GET_FIELD x.
        a.b => OP_LOAD_LOCAL 0; OP_GET_FIELD 0;
        */
        case AST_STRUCTACCESS: {
            if (currentNode->left->type == AST_IDENTIFIER) {
                identifierLocation il = getIdentifier(c, s, currentNode->left->value.nameVal);
                if (il.ptr == NULL) fatalError(0x1E, "", -1);
                structDefinition* sd = getStructDefViaName(il.ptr->structType, c);
                
                for (int i = 0; i < sd->fields->length; i++) {
                    char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                    structTypes* st = (structTypes*)List_GetElement(sd->fieldTypes, i)->data;
                    if (strcmp(fieldName, currentNode->right->value.nameVal) == 0) {
                        if (il.isGlobal) addUnaryInstruction(c, OP_LOAD_GLOBAL, il.index, chunkIdx);
                        else addUnaryInstruction(c, OP_LOAD_LOCAL, il.index, chunkIdx);
                        addUnaryInstruction(c, OP_GET_FIELD, i, chunkIdx);
                        
                        return;   
                    }
                }
                fatalError(0x21, "Unknown struct access", -1);
            }
            else if (currentNode->left->type == AST_STRUCTACCESS) {
                toBytecode(c, currentNode->left, s, chunkIdx);
                structDefinition* sd = getStructType(currentNode->left, c, s);
                if (sd == NULL) {
                    fatalError(0x21, "Unknown struct access", -1);
                }
                if (currentNode->right->type != AST_IDENTIFIER) {fatalError(0x21, "Unknown struct access", -1);}
                for (int i = 0; i < sd->fields->length; i++) {
                    char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                    typedValue* fieldType = (typedValue*)List_GetElement(sd->fieldTypes, i)->data;
                    if (strcmp(fieldName, currentNode->right->value.nameVal ) == 0) {
                        addUnaryInstruction(c, OP_GET_FIELD, i, chunkIdx);
                        return;
                    }
                }
                fatalError(0x21, "Unknown struct access", -1);
                ;
                break;
            }
            else if (currentNode->left->type == AST_ARRAYACCESS) {
                toBytecode(c, currentNode->left, s, chunkIdx);
                structDefinition* sd = getStructType(currentNode->left, c, s);
                if (sd == NULL) {
                    fatalError(0x22, "Invalid array access", -1);
                }
                for (int i = 0; i < sd->fields->length; i++) {
                    char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                    typedValue* fieldType = (typedValue*)List_GetElement(sd->fieldTypes, i)->data;
                    if (strcmp(fieldName, currentNode->right->value.nameVal ) == 0) {
                        addUnaryInstruction(c, OP_GET_FIELD, i, chunkIdx);
                        ;
                        return;
                    }
                }
                fatalError(0x22, "Invalid array access", -1);
            }
            else if (currentNode->left->type == AST_FUNCCALL) {
                toBytecode(c, currentNode->left, s, chunkIdx);
                structDefinition* sd = getStructType(currentNode, c, s);
                if (sd == NULL) {
                    fatalError(0x23, "Invalid function call", -1);
                }
                for (int i = 0; i < sd->fields->length; i++) {
                    char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                    typedValue* fieldType = (typedValue*)List_GetElement(sd->fieldTypes, i)->data;
                    if (strcmp(fieldName, currentNode->right->value.nameVal ) == 0) {
                        addUnaryInstruction(c, OP_GET_FIELD, i, chunkIdx);
                        ;
                        return;
                    }
                }
            }
            ;
            break;
        }
        /*
        If the array contains no identifiers, add it to the lsit of constants, and do OP_LOAD_CONSTANT x.
        If the array has an identifier or for some reason is unable to be added to constants, it adds each element of the array into the bytecode, then uses OP_BUILD_ARRAY x.

        [2,4,6,8] -> OP_LOAD_CONST x
        [2,a,b,9] -> OP_LOAD_CONST n; OP_LOAD_LOCAL 0; OP_LOAD_LOCAL 1; OP_LOAD_CONST m; OP_BUILD_ARRAY 4;
        */
        case AST_ARRAYLITERAL: {
            
            // If the array is in constants, use OP_LOAD_CONST.
            if (currentNode->children == NULL) return;
            if (currentNode->children->length == 0) {
                for (int i = 0; i < c.constants->length; i++) {
                    typedValue* tv = List_GetElement(c.constants, i)->data;
                    if (tv->valueType == TYPE_ARRAY) {if (tv->value.av.len == 0) {addUnaryInstruction(c, OP_LOAD_CONST, i, chunkIdx); ; return;}}
                }
                typedValue* toApp = malloc(sizeof(typedValue));
                *toApp = (typedValue){
                    .valueType = TYPE_ARRAY,
                    .ptr = NULL,
                    .value.av = (arrayValue){
                        .arrayType = AT_NUM,
                        .data = NULL,
                        .len = 0
                    }
                };
                List_AppendElement(c.constants, toApp);
                return;
            };
            // If it already has an identifier, then it's not in constants.
            bool hasIdentifiers = false;
            bool inConstants = false;
            for (int i = 0; i < currentNode->children->length; i++) {
                int type = ((ASTNode*)List_GetElement(currentNode->children, i)->data)->type;
                if (type != AST_NUMBER) {
                    hasIdentifiers = true;
                    inConstants = false;
                    break;
                }
            }
            if (!hasIdentifiers) {
                for (int i = 0; i < c.constants->length; i++) {
                    typedValue* tv = (typedValue*)List_GetElement(c.constants, i)->data;
                    if (tv->valueType == TYPE_ARRAY) {
                        if (tv->value.av.arrayType != AT_UNKNOWN) {
                            if (tv->value.av.len != currentNode->children->length) continue;
                            bool found = true;
                            for (int j = 0; j < currentNode->children->length; j++) {
                                num A = ((ASTNode*)List_GetElement(currentNode->children, j)->data)->value.numVal;
                                num B = tv->value.av.data[j]->value.numberValue;
                                if (cmpNum(A,B) != 0) {
                                    found = false;
                                    break;
                                }
                            }
                            if (found) {
                                addUnaryInstruction(c, OP_LOAD_CONST, i, chunkIdx);
                                ;
                                return;
                            }
                            
                        }
                    }
                }
            }
            // manually build the array
            int type = TYPE_NULL;
            int numType = NUM_BOOL;
            int elementSize = 0;
            typedValue** data = malloc(1);
            int datalen = 0;
            for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, i)->data;
                if (t->type == AST_NUMBER) {
                    if (type == TYPE_NULL) numType = t->value.numVal.type;
                    if (type == TYPE_NULL) type = TYPE_NUM;
                    if (type != TYPE_NUM) {
                        fatalError(0x24, "Arrays cannot have multiple types.", -1);
                    }
                    if (t->value.numVal.type != numType) {fatalError(0x24, "Arrays cannot have multiple types (Check the individual types of the numbers).", -1);}
                    num tval = t->value.numVal;
                    toBytecode(c, t, s, chunkIdx);
                    typedValue* cpy = malloc(sizeof(typedValue));
                    *cpy = (typedValue){
                        .valueType = TYPE_NUM,
                        .value.numberValue = (num){.type = tval.type, .value = tval.value},
                        .ptr = NULL,
                    };
                    data = realloc(data, sizeof(typedValue*)*(datalen+1));
                    data[datalen] = cpy;
                    datalen++;
                }
                else {
                    toBytecode(c, t, s, chunkIdx); // TODO: add error-handling here.
                }
            }
            if (hasIdentifiers == false) {
                typedValue* toStore = malloc(sizeof(typedValue));
                *toStore = (typedValue){
                    .valueType = TYPE_ARRAY,
                    .value.av = (arrayValue){
                        .data = data,
                        .arrayType = type == TYPE_NUM ? AT_NUM : AT_UNKNOWN,
                        .len = currentNode->children->length
                    },
                    .ptr = NULL,
                };
                
                List_AppendElement(c.constants, toStore);
                addUnaryInstruction(c, OP_BUILD_ARRAY, currentNode->children->length, chunkIdx);
            }
            else {
                free(data);
                addUnaryInstruction(c, OP_BUILD_ARRAY, currentNode->children->length, chunkIdx);
            }
            ;
            break;
        }
        /*
        Converts AST_STRING into AST_LOAD_CONST x.        
        */
        case AST_STRING: {
            for (int i = 0; i < c.constants->length; i++) {
                typedValue* tv = (typedValue*)List_GetElement(c.constants, i)->data;
                if (tv->valueType == TYPE_ARRAY) {
                    if (tv->value.av.arrayType == AT_CHARARR) {
                        num* converted = malloc(sizeof(num)*tv->value.av.len);
                        bool isNumArr = true;
                        for (int i = 0; i < tv->value.av.len; i++) {
                            typedValue* idx = tv->value.av.data[i];
                            if (idx->valueType != TYPE_NUM) {isNumArr = false; break;};
                            converted[i] = idx->value.numberValue;
                        }
                        if (!isNumArr) {free(converted); break;}
                        char* arrc = charArrToStr(converted, tv->value.av.len);
                        if (arrc == NULL) continue;
                        if (strncmp(arrc, currentNode->value.anyVal, strlen(currentNode->value.anyVal)) == 0 && strlen(arrc) == strlen(currentNode->value.anyVal)) {
                            addUnaryInstruction(c, OP_LOAD_CONST, i, chunkIdx);
                            ;
                            return;
                        }
                        free(arrc);
                        free(converted);
                    }
                }
            }
            typedValue* tv = malloc(sizeof(typedValue));
            *tv = (typedValue){
                .valueType = TYPE_ARRAY,
                .value.av = (arrayValue){
                    .data = malloc(sizeof(typedValue*)*(strlen(currentNode->value.anyVal)+1) ),
                    .arrayType = AT_CHARARR,
                    .len = strlen(currentNode->value.anyVal)+1
                },
                .ptr = NULL,
            };
            for (int i = 0; i <= strlen(currentNode->value.anyVal); i++) {
                typedValue* toApp = malloc(sizeof(typedValue));
                *toApp = (typedValue){
                    .valueType = TYPE_NUM,
                    .value.numberValue = (num){
                        .type = NUM_CHAR,
                        .value.cVal = strlen(currentNode->value.anyVal) ? currentNode->value.anyVal[i] : 0
                    },
                    .ptr = NULL
                };
                (tv->value.av.data)[i] = toApp;
            }
            addUnaryInstruction(c, OP_LOAD_CONST, c.constants->length, chunkIdx);
            List_AppendElement(c.constants, tv);
            ;
            break;
        }
        /*
        Converts AST_NUMBER into AST_LOAD_CONST x.
        */
        case AST_NUMBER: {
            typedValue val = (typedValue){.valueType = TYPE_NUM, .value.numberValue = currentNode->value.numVal};
            for (int i = 0; i < c.constants->length; i++) {
                typedValue* tv = (typedValue*)List_GetElement(c.constants, i)->data;
                if (tv->valueType == val.valueType && val.valueType == TYPE_NUM) {
                    if (tv->value.numberValue.type != val.value.numberValue.type) continue;
                    if (cmpNum(val.value.numberValue, tv->value.numberValue) == 0) {
                         addUnaryInstruction(c,OP_LOAD_CONST, i, chunkIdx);   
                         return;
                    }
                }
            }
            typedValue* toAppend = malloc(sizeof(typedValue));
            *toAppend = (typedValue){
                .valueType = TYPE_NUM,
                .value.numberValue = currentNode->value.numVal,
                .ptr = NULL,
            };
            List_AppendElement(c.constants, toAppend);
            addUnaryInstruction(c, OP_LOAD_CONST, c.constants->length-1, chunkIdx);        
            ;
            break;    
        }
        /*
        Converts each child into bytecode.
        */
        case AST_PRGM: {
            if (currentNode->children == NULL) return;
            for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* n = (ASTNode*)List_GetElement(currentNode->children, i)->data;
                if (chunkIdx == 0) {
                    if (n->type == AST_IMPORT) continue;
                    if (n->type != AST_PRGM && n->type != AST_STRUCT && n->type != AST_VARDECLARATION && n->type != AST_ASSIGNMENT && n->type != AST_FUNCDEL) {
                        if (isDebug) printf("Illegal type found: %d", n->type);
                        if (isDebug) printAST(n,0);
                        fatalError(0x25, "Only struct, variable, or function declarations are allowed in premain.", i);
                    }
                }
                toBytecode(c, n, s, chunkIdx);
            }
            ;
            break;
        }
        /*
        Converts AST_VARDECLARATION into:
        OP_STORE_GLOBAL/OP_STORE_LOCAL n - If it's a simple struct or function call
        OP_NEWSTRUCT n; OP_STORE_LOCAL n - If there's a constructor
        */
        case AST_VARDECLARATION: {
            // DECLARATION/Assignment FORMAT:
            /*
            .left = datatype (0 if type is AST_ASSIGNMENT)
            .right = name
            .children[0] = value
            */
            char* name = currentNode->right->value.nameVal;
            if (s.locals != NULL) {
                for (int i = 0; i < s.locals->length; i++) {
                    char* n = ((vmVariable*)List_GetElement(s.locals, i)->data)->name;
                    if (strcmp(n, name) == 0) {
                        fatalError(0x26, "Variable already exists (Check global scope).", -1);
                    }
                }
            }
            for (int i = 0; i < c.globals->length; i++) {
                char* n = ((vmVariable*)List_GetElement(c.globals, i)->data)->name;
                if (strcmp(n, name) == 0) {
                    fatalError(0x26, "Variable already exists (Check local scope).", -1);
                }
            }
            // if currentNode->left->type is AST_COMPLEXDATATYPE, then that requires different handling (which will be implemented later on)
            if (currentNode->left->type == AST_DATATYPE && currentNode->left->left == NULL) {
                if (currentNode->children->length > 0) {
                    toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data, s, chunkIdx);
                    char* cpy = strdup(name);
                    vmVariable* toApp = malloc(sizeof(vmVariable));
                    *toApp = (vmVariable){
                        .name = cpy,
                        .structType = currentNode->left->left == NULL ? (currentNode->left->value.numberVal == PDT_STRING ? strdup("string") : NULL) : strdup("array"),
                        .t = TYPE_NULL,
                        .isArray = (currentNode->left->left == NULL) ? false : (currentNode->left->type == AST_ARRAYASSIGNMENT)
                    };
                    applySpecifiers(currentNode->left, toApp);
                    if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                    else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                }
            }
            else if (currentNode->left->type == AST_COMPLEXDATATYPE && currentNode->left->left == NULL) {
                structDefinition* varStruct = NULL;
                int structIdx = -1;
                // get struct definition:
                for (int i = 0; i < c.structDefs->length; i++) {
                    structDefinition* sd = (structDefinition*)List_GetElement(c.structDefs, i)->data;
                    if (strcmp(sd->name, currentNode->left->value.nameVal) == 0) {
                        varStruct = sd;
                        structIdx = i;
                        break;
                    }
                }
                // If it's null, then this is an invalid var declaration.
                if (varStruct == NULL) {
                    for (int i = 0; i < c.structDefs->length && isDebug; i++) {
                        printf("\n: %s %s", ((structDefinition*)List_GetElement(c.structDefs, i)->data)->name, currentNode->left->value.nameVal);
                    }
                    fatalError(0x27, "Cannot find struct", -1);
                }
                // if it's in the format of structName n;
                if (currentNode->children == NULL) {
                    currentNode->children = malloc(sizeof(List));
                    *currentNode->children = NewList();
                }
                if (currentNode->children->length == 0) {
                    addNullaryInstruction(c, OP_CLEAR_STACK, chunkIdx);
                    addUnaryInstruction(c, OP_NEW_STRUCT, structIdx, chunkIdx);
                    ;
                    return;
                }
                // if it's in the format of structName n = structName() or structName n = func();
                ASTNode* eqSide = (ASTNode*)List_GetElement(currentNode->children, 0)->data;
                if (eqSide->type == AST_FUNCCALL) {
                    if (eqSide->left->type == AST_IDENTIFIER) {
                        // structName n = structName()
                        if (strcmp(eqSide->left->value.nameVal, varStruct->name) == 0) {
                            char* constructorName = malloc((2*strlen(varStruct->name)) + 2);
                            sprintf(constructorName, "%s_%s", varStruct->name, varStruct->name);
                            for (int i = 0; i < c.functionIdentifiers->length; i++) {
                                bcFunction* bcc = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
                                char* bcName = bcc->name;
                                if (strcmp(bcName, constructorName) == 0) {
                                    addNullaryInstruction(c, OP_CLEAR_STACK, chunkIdx);
                                    eqSide->left->value.nameVal = constructorName;
                                    addUnaryInstruction(c, OP_NEW_STRUCT, structIdx, chunkIdx);
                                    toBytecode(c, eqSide, s, chunkIdx);
                                    break;
                                }
                            }
                            char* cpy = strdup(currentNode->right->value.nameVal);
                            vmVariable* toApp = malloc(sizeof(vmVariable));
                            *toApp = (vmVariable){
                                .name = cpy,
                                .structType = currentNode->left->left == NULL ? (char*)((structDefinition*)List_GetElement(c.structDefs, structIdx)->data)->name : strdup("array"),
                                .t = TYPE_NULL,
                                .isArray = currentNode->left->left == NULL ? false : currentNode->left->type == AST_ARRAYASSIGNMENT
                            };
                            applySpecifiers(currentNode->left, toApp);
                            if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                            else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                            
                            // constructor
                            ;
                            return;
                        }
                        else {
                            structDefinition* sd = getStructType(eqSide, c, s);
                            toBytecode(c, eqSide, s, chunkIdx);
                            vmVariable* toApp = malloc(sizeof(vmVariable));
                            *toApp = (vmVariable){
                                .name = strdup(currentNode->right->value.nameVal),
                                .structType = strdup(sd->name),
                                .t = TYPE_NULL,
                                .isArray = currentNode->left->left == NULL ? false : currentNode->left->type == AST_ARRAYASSIGNMENT
                            };
                            applySpecifiers(currentNode->left, toApp);
                            if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                            else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                            ;
                            return;
                        }
                    }
                    else if (eqSide->left->type == AST_STRUCTACCESS) {
                        structDefinition* sd = getStructType(eqSide, c, s);
                        toBytecode(c, eqSide, s, chunkIdx);
                        vmVariable* toApp = malloc(sizeof(vmVariable));
                        *toApp = (vmVariable){
                            .name = strdup(currentNode->right->value.nameVal),
                            .structType = strdup(sd->name),
                            .t = TYPE_NULL,
                            .isArray = currentNode->left->left == NULL ? false : currentNode->left->type == AST_ARRAYASSIGNMENT
                        };
                        applySpecifiers(currentNode->left, toApp);
                        if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                        else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                        ;
                        return;
                    }
                }
                else if (eqSide->type == AST_ARRAYLITERAL) {
                    toBytecode(c, eqSide, s, chunkIdx);
                    char* cpy = strdup(currentNode->right->value.nameVal);
                    vmVariable* toApp = malloc(sizeof(vmVariable));
                    *toApp = (vmVariable){
                        .name = cpy,
                        .structType = strdup("array"),
                        .t = TYPE_NULL,
                        .isArray = true
                    };
                    applySpecifiers(currentNode->left, toApp);
                    if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                    else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                    ;
                    return;
                }
                fatalError(0x28, "Unknown error with variable declaration.", -1);
                
                break;
            }
            else if (currentNode->left->left != NULL) {
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data, s, chunkIdx);
                char* cpy = strdup(name);
                vmVariable* toApp = malloc(sizeof(vmVariable));
                *toApp = (vmVariable){
                    .name = cpy,
                    .structType = strdup("array"),
                    .t = 0,
                    .isArray = true,
                };
                applySpecifiers(currentNode->left, toApp);
                if (chunkIdx == 0) {List_AppendElement(c.globals, toApp); addUnaryInstruction(c, OP_STORE_GLOBAL, c.globals->length-1, chunkIdx);}
                else {List_AppendElement(s.locals, toApp); addUnaryInstruction(c, OP_STORE_LOCAL, s.locals->length-1, chunkIdx);}
                
            }
            else {
                fatalError(0x29, "Invalid datatype.", -1);
            }
            ;
            break;
        }
        /*
        converts AST_ASSIGNMENT into either OP_STORE_LOCAL, OP_STORE_IDX, or OP_SET_FIELD depending on what's being set.
        */
        case AST_ASSIGNMENT: {
            if (currentNode->right->type == AST_IDENTIFIER) {
                if (currentNode->children->length == 0) {fatalError(0x2A, "Invalid assignment.", -1);}
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data,s, chunkIdx);
                for (int i = 0; i < s.locals->length; i++) {
                    vmVariable* vmv = ((vmVariable*)List_GetElement(s.locals, i)->data);
                    if (strcmp(currentNode->right->value.nameVal, vmv->name) == 0 ) {
                        if (vmv->isConst) fatalError(0x7, "", -1);    
                        addUnaryInstruction(c, OP_STORE_LOCAL, i, chunkIdx);
                        ;
                        return;
                    }
                }
                for (int i = 0; i < c.globals->length; i++) {
                    vmVariable* vmv = ((vmVariable*)List_GetElement(c.globals, i)->data);
                    if (strcmp(currentNode->right->value.nameVal, vmv->name) == 0 ) {
                        if (vmv->isConst) fatalError(0x7, "", -1);
                        addUnaryInstruction(c, OP_STORE_GLOBAL, i, chunkIdx);
                        ;
                        return;
                    }
                }
            }
            else if (currentNode->right->type == AST_ARRAYACCESS) {
                toBytecodeArrayAssignmentTarget(c, currentNode->right, s, chunkIdx, false);
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data, s, chunkIdx);
                addNullaryInstruction(c, OP_STORE_IDX, chunkIdx);
            }
            else if (currentNode->right->type == AST_STRUCTACCESS) {
                if (currentNode->right->left->type == AST_IDENTIFIER && currentNode->right->right->type == AST_IDENTIFIER) {
                    // easy n.x type
                    char* identifierName = currentNode->right->left->value.nameVal;
                    char* structName = NULL;
                    int idOrigin = 0;
                    int idIdx = -1;
                    if (s.locals != NULL) {
                        for (int i = 0; i < s.locals->length; i++) {
                            char* ss = ((vmVariable*)List_GetElement(s.locals, i)->data)->name;
                            if (strcmp(identifierName, ((vmVariable*)List_GetElement(s.locals, i)->data)->name) == 0 ) {
                                structName = ((vmVariable*)List_GetElement(s.locals, i)->data)->structType;
                                idIdx = i;
                                idOrigin = 1;
                                break;
                            }
                        }
                    }
                    if (idIdx == -1) {
                        for (int i = 0; i < c.globals->length; i++) {
                            if (strcmp(identifierName, ((vmVariable*)List_GetElement(c.globals, i)->data)->name) == 0 ) {
                                structName = ((vmVariable*)List_GetElement(c.globals, i)->data)->structType;
                                idOrigin = 2;
                                idIdx = i;
                                break;
                            }
                        }
                    }
                    if (structName == NULL) {fatalError(0x22, "Invalid struct access", -1);}
                    // check if n.x is a valid access.
                    structDefinition* sd = NULL;
                    for (int i = 0; i < c.structDefs->length; i++) {
                        if (strcmp(((structDefinition*)List_GetElement(c.structDefs, i)->data)->name, structName ) == 0) {
                            sd = (structDefinition*)List_GetElement(c.structDefs, i)->data;
                            break;
                        }
                    }
                    if (sd == NULL) {
                        fatalError(0x2B, "Invalid struct name", -1);
                    }
                    for (int i = 0; i < sd->fields->length; i++) {
                        char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                        char* nameV = currentNode->right->right->value.nameVal;
                        if (strcmp(fieldName, nameV) == 0) {
                            structTypes* st = List_GetElement(sd->fieldTypes, i)->data;
                            if (st->isConst) fatalError(0x7, "", -1);
                            if (idOrigin == 1) addUnaryInstruction(c, OP_LOAD_LOCAL, idIdx, chunkIdx);
                            if (idOrigin == 2) addUnaryInstruction(c, OP_LOAD_GLOBAL, idIdx, chunkIdx);
                            toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data, s, chunkIdx);
                            addUnaryInstruction(c, OP_SET_FIELD, i, chunkIdx);
                            ;
                            return;
                        }
                    }
                    fatalError(0x2C, "Invalid struct assignment", -1);
                }
                else {
                    toBytecode(c, currentNode->right->left, s, chunkIdx);
                    structDefinition* sd = getStructType(currentNode->right->left, c, s);
                    if (sd == NULL) {
                        fatalError(0x2C, "Invalid struct assignment", -1);
                    }
                    for (int i = 0; i < sd->fields->length; i++) {
                        char* fieldName = (char*)List_GetElement(sd->fields, i)->data;
                        if (strcmp(fieldName, currentNode->right->right->value.nameVal) == 0) {
                            toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 0)->data, s, chunkIdx);
                            addUnaryInstruction(c, OP_SET_FIELD, i, chunkIdx);
                            ;
                            return;
                        }
                    }
                }
                
            }
            //;
            break;
        }
        /*
        converts AST_CONDITIONAL into an OP_JUMP_IF_FALSE call.
        e.x if (2==3) {n();}
        it'll be:
        1. OP_LOAD_CONST 0
        2. OP_LOAD_CONST 1
        3. OP_EQUALS
        4. OP_JUMP_IF_FALSE 6
        5. OP_CALL n
        6. HALT
        */
        case AST_CONDITIONAL: {
            instruction* i = malloc(sizeof(instruction));
            i->argc = 1;
            i->args = malloc(sizeof(int));
            i->args[0] = 0; // assign to this later so jmpiffalse is correct.
            i->code = OP_JUMP_IF_FALSE;
            
            toBytecode(c, currentNode->left, s, chunkIdx);
            List_AppendElement(getInstructions(c, chunkIdx), i);
            for (int i = 0; i < currentNode->right->children->length; i++) {
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->right->children, i)->data, s, chunkIdx);
            }
            i->args[0] = getInstructions(c, chunkIdx)->length + 1;
            ;
            break;
        }
        // If there's an else statement, just convert the block.
        case AST_ELSE: {
            for (int i = 0; i < currentNode->right->children->length; i++) {
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->right->children, i)->data, s, chunkIdx);
            }
            ;
            break;
        }
        // Converts an if statement into:
        /*
        {condition}
        OP_JUMP_IF_FALSE elif1
        {block1}
        OP_JUMP afterLoop
        {elif1}
        OP_JUMP_IF_FALSE else
        {block1}
        OP_JUMP afterLoop
        {else}
        {elseBlock}
        
        */
        case AST_IF: {
           instruction* toModify = malloc(sizeof(instruction)*currentNode->children->length);
           for (int i = 0; i < currentNode->children->length; i++) {
                toModify[i] = (instruction){
                    .argc = 1,
                    .args = malloc(sizeof(int)),
                    .code = OP_JUMP
                };
           }

           for (int i = 0; i < currentNode->children->length; i++) {
            ASTNode* n = (ASTNode*)List_GetElement(currentNode->children, i)->data;
            toBytecode(c, n, s, chunkIdx);
            List_AppendElement(getInstructions(c, chunkIdx), &(toModify[i]) );
           }
           for (int i = 0; i < currentNode->children->length; i++) {
            *(toModify[i].args) = getInstructions(c, chunkIdx)->length; // JMP n
           }
           break;
        }
        // Adds the current index to s.breakTargets. After whatever loop it is, the actual jump index will be clarified.
        case AST_BREAK: {
            instruction* jmp = malloc(sizeof(instruction));
            *jmp = (instruction){
                .argc = 1,
                .args = malloc(sizeof(int)),
                .code = OP_JUMP
            };
            List_AppendElement(s.breakStatements, jmp);
            List_AppendElement(getInstructions(c, chunkIdx), jmp);
            break;
        }
        // continueTarget was given in the scopeInfo, so this adds an OP_JUMP whose argument is continueTarget.
        case AST_CONTINUE: {
            addUnaryInstruction(c, OP_JUMP, s.continueTgt, chunkIdx);
            ;
            break;
        }
        // Converts all children of AST_BLOCK.
        case AST_BLOCK: {
            for (int i = 0; i < currentNode->children->length; i++) {
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, i)->data, s, chunkIdx);
            }
            ;
            break;
        }
        /*
        converts a while loop into:
        {condition}
        OP_JUMP_IF_FALSE endLoop
        loopStuff
        OP_JUMP condition
        
        */
        case AST_WHILE: {
            int jmpNum = getInstructions(c, chunkIdx)->length; // Line to jump to when continue happens.
            List BS = NewList();
            toBytecode(c, currentNode->left, (scopeInfo){&BS,0, s.locals}, chunkIdx);
            instruction* jmpFalse = malloc(sizeof(instruction));
            *jmpFalse = (instruction){
                .argc = 1,
                .args = malloc(sizeof(int)),
                .code = OP_JUMP_IF_FALSE
            };
            List_AppendElement(&BS, jmpFalse);
            List_AppendElement(getInstructions(c, chunkIdx), jmpFalse);
            for (int i = 0; i < currentNode->right->children->length; i++) {
                ASTNode* ln = (ASTNode*)List_GetElement(currentNode->right->children, i)->data;
                toBytecode(c, ln, (scopeInfo){&BS,0, s.locals}, chunkIdx);
            }
            addUnaryInstruction(c, OP_JUMP, jmpNum, chunkIdx);
            int ilen = getInstructions(c, chunkIdx)->length;
            for (int i = 0; i < BS.length; i++) {
                ((instruction*)List_GetElement(&BS, i)->data)->args[0] = ilen;
            }
            ;
            break;
        }
        /*
        converts AST_FOR into:
        {varDeclaration}
        OP_JUMP {1stLoop}
        {updateValue}
        {checkCondition}
        OP_JUMP_IF_FALSE {afterLoop}
        {block}
        OP_JUMP {updateValue}
        */
        case AST_FOR: {
            List BS = NewList();
            toBytecode(c, List_GetElement(currentNode->children, 0)->data, s, chunkIdx); // Init
            instruction* n = malloc(sizeof(instruction));
            *n = (instruction){
                .args = malloc(sizeof(int)),
                .argc = 1,
                .code = OP_JUMP
            };
            int loopjmp = getInstructions(c, chunkIdx)->length+1;
            List_AppendElement(getInstructions(c, chunkIdx), n); // JMP X (skip condition checking)
            toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, 2)->data, (scopeInfo){&BS,0, s.locals}, chunkIdx); // Var modification

            toBytecode(c, List_GetElement(currentNode->children, 1)->data, s, chunkIdx);
            instruction* jmp0 = malloc(sizeof(instruction));
            *jmp0 = (instruction){
                .argc = 1,
                .args = malloc(sizeof(int)),
                .code = OP_JUMP_IF_FALSE
            };
            List_AppendElement(getInstructions(c, chunkIdx), jmp0);
            n->args[0] = getInstructions(c, chunkIdx)->length; // Set jmp to correct part
            for (int i = 3; i < currentNode->children->length; i++) {
                toBytecode(c, (ASTNode*)List_GetElement(currentNode->children, i)->data, (scopeInfo){&BS,n->args[0], s.locals}, chunkIdx); // Block
            }
            addUnaryInstruction(c, OP_JUMP, loopjmp, chunkIdx);
            jmp0->args[0] = getInstructions(c, chunkIdx)->length;
            for (int i = 0; i < BS.length; i++) {
                ((instruction*)List_GetElement(&BS, i)->data)->args[0] = jmp0->args[0];
            }
            ;
            break;
        }
        /*
        converts AST_ARRAYACCESS into:
        {loadLeft}
        {loadRight}
        OP_LOAD_IDX
        */
        case AST_ARRAYACCESS: {
            if (currentNode->left != NULL) toBytecode(c, currentNode->left, s, chunkIdx);
            if (currentNode->right != NULL) toBytecode(c, currentNode->right, s, chunkIdx);
            addNullaryInstruction(c, OP_LOAD_IDX, chunkIdx);
            break;
        }
        /*
        Makes a new chunk, and loads each line of the function into that chunk.
        */
        case AST_FUNCDEL: {
            int newChunkIdx = 0;
            if (strcmp(currentNode->left->value.nameVal, "main") == 0) {
                for (int i = 0; i < c.chunks->length; i++) {
                    if (strcmp(((chunk*)List_GetElement(c.chunks, i)->data)->name, "main") == 0) {
                        newChunkIdx = i;
                    }
                }
            }
            else {
                for (int i = 2; i < c.chunks->length; i++) {
                    if (strcmp(((chunk*)List_GetElement(c.chunks, i)->data)->name, currentNode->left->value.nameVal) == 0) {
                        newChunkIdx = i;
                    }
                }
                if (newChunkIdx == 0) {
                    int paramNum = 0;
                    for (int i = 0; i < currentNode->children->length; i++) {
                        if (((ASTNode*)List_GetElement(currentNode->children, i)->data)->type == AST_PARAM) {
                            paramNum++;
                        }
                    }
                    bcFunction* cpy = malloc(sizeof(bcFunction));
                    *cpy = (bcFunction){
                        .argc = paramNum,
                        .name = strdup(currentNode->left->value.nameVal),
                        .isSystem = false,
                    };
                    char* returnName = NULL;
                    if (currentNode->right->type == AST_DATATYPE) {
                        if (currentNode->right->value.numberVal == PDT_STRING) returnName = "string";
                    }
                    else if (currentNode->right->type == AST_COMPLEXDATATYPE) {
                        returnName = currentNode->right->value.nameVal;
                    }
                    if (returnName != NULL) cpy->returnStruct = strdup(returnName);
                    else cpy->returnStruct = NULL;
                    if (currentNode->right->left != NULL) returnName = "array";
                    List_AppendElement(c.functionIdentifiers, cpy);
                    newChunkIdx = c.chunks->length;
                    chunk* toAdd = malloc(sizeof(chunk));
                    *toAdd = (chunk){.instructions = malloc(sizeof(List)), .name = cpy->name};
                    *toAdd->instructions = NewList();
                    List_AppendElement(c.chunks, toAdd);
                }
            }
            ASTNode* block = (ASTNode*)List_GetElement(currentNode->children, -1)->data;
            scopeInfo newScope = (scopeInfo){
                .breakStatements = malloc(sizeof(List)),
                .continueTgt = 0,
                .locals = malloc(sizeof(List))
            };
            *newScope.breakStatements = NewList();
            *newScope.locals = NewList();
            for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* n = ((ASTNode*)List_GetElement(currentNode->children, i)->data);
                if (n->type == AST_PARAM) {
                    char* cpy = NULL;
                    for (int j = 0; j < n->children->length; j++) {
                        ASTNode* current = ((ASTNode*)(List_GetElement(n->children, j)->data));
                        if (current->type == AST_IDENTIFIER ) {
                            cpy = strdup(current->value.nameVal);
                        }
                    }
                    vmVariable* toApp = malloc(sizeof(vmVariable));
                    ASTNode* dataNode = (ASTNode*)List_GetElement(n->children, 0)->data;
                    char* structType = dataNode->type == AST_COMPLEXDATATYPE ? dataNode->value.nameVal : NULL;
                    *toApp = (vmVariable){
                        .name = cpy,
                        .structType = structType,
                        .t = 0,
                        .isArray = dataNode->left == NULL ? false : dataNode->left->type == AST_ARRAYASSIGNMENT
                    };
                    List_AppendElement(newScope.locals, toApp);
                }
            }
            for (int i = 0; i < (block->children->length); i++) {
                toBytecode(c, (ASTNode*)List_GetElement(block->children, i)->data, newScope, newChunkIdx);
            }
            if (chunkIdx != 0) addNullaryInstruction(c, OP_RETURN, chunkIdx);
            ;
            break;
        }
        /*
        Converts AST_SWITCH into:
        {condition1} 
        JUMP_IF_FALSE condition2
        {block}
        JUMP afterStatement
        {condition2}
        JUMP_IF_FALSE condition3
        {block}
        JUMP afterStatement
        .
        .
        .
        Additionally, conditions that are in the format of (29) get converted into (a == 29).
        */
        case AST_SWITCH: {
           if (currentNode->children == NULL) return; // empty
           ASTNode* toSwitch = currentNode->left;
           List* toJump = malloc(sizeof(List));
           *toJump = NewList();
           for (int i = 0; i < currentNode->children->length; i++) {
                ASTNode* child = (ASTNode*)List_GetElement(currentNode->children, i)->data;
                if (child->type != AST_CASE) continue; // isn't valid switch statement
                instruction* toAppend = malloc(sizeof(instruction));
                *toAppend = (instruction){
                    .argc = 1,
                    .args = malloc(sizeof(int)),
                    .code = OP_JUMP_IF_FALSE
                };
                
                toBytecode(c, child->left, s, chunkIdx);
                if (child->left->type != AST_OPERATOR) {
                    toBytecode(c, toSwitch, s, chunkIdx);
                    addNullaryInstruction(c, OP_EQUALS, chunkIdx);
                }
                List_AppendElement(((chunk*)List_GetElement(c.chunks, chunkIdx)->data)->instructions, toAppend );
                toBytecode(c, child->right, s, chunkIdx); // block
                // add jmp statement to jump to after it
                instruction* jmpInstruction = malloc(sizeof(instruction));
                *jmpInstruction = (instruction){
                    .argc = 1,
                    .args = malloc(sizeof(int)),
                    .code = OP_JUMP
                };
                List_AppendElement(((chunk*)List_GetElement(c.chunks, chunkIdx)->data)->instructions, jmpInstruction);
                List_AppendElement(toJump, jmpInstruction);
                toAppend->args[0] = ((chunk*)List_GetElement(c.chunks, chunkIdx)->data)->instructions->length;
            }
            for (int i = 0; i < toJump->length; i++) {
                ((instruction*)List_GetElement(toJump, i)->data)->args[0] = ((chunk*)List_GetElement(c.chunks, chunkIdx)->data)->instructions->length;
            }
            freeListKeepData(toJump);
           break;
        }
        // adds OP_RETURN to the bytecode.
        case AST_RETURN: {
            if (currentNode->right != NULL) {
                toBytecode(c, currentNode->right, s, chunkIdx );
            }
            addNullaryInstruction(c, OP_RETURN, chunkIdx);
            
            break;
        }
        // Finds the function & adds OP_CALL n to the code.
        case AST_FUNCCALL: {
            
            if (currentNode->left->type == AST_IDENTIFIER) {
                for (int i = 0; i < c.functionIdentifiers->length; i++) {
                    bcFunction* bf = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
                    structDefinition* sd = isConstructor(bf->name, c);
                    if (strcmp(bf->name, currentNode->left->value.nameVal) == 0) {
                        if (currentNode->children != NULL) {
                            for (int j = 0; j < currentNode->children->length; j++) {
                                ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, j)->data;
                                if (t->type != AST_PARAM) {    
                                    continue;
                                };
                                toBytecode(c, t->left, s, chunkIdx);
                            }
                        }
                        addBinaryInstruction(c, OP_CALL, i, currentNode->children == NULL ? sd==NULL?0:1 : currentNode->children->length+(sd==NULL?0:1), chunkIdx);
                        
                        return;
                    }
                    else if (sd != NULL) {
                        if (strcmp(currentNode->left->value.nameVal,sd->name) == 0) {
                            if (sd != NULL && currentNode->children->length != bf->argc-1) {fatalError(0x2D, "Unknown assignment error.", -1);}
                            int structIdx = -1;
                            for (int j = 0; j < c.structDefs->length; j++) {
                                if (strcmp(((structDefinition*)List_GetElement(c.structDefs, j)->data)->name, sd->name) == 0) {structIdx = j; break;}
                            }
                            addUnaryInstruction(c, OP_NEW_STRUCT, structIdx, chunkIdx);
                            if (currentNode->children != NULL) {
                                for (int j = 0; j < currentNode->children->length; j++) {
                                    ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, j)->data;
                                    if (t->type != AST_PARAM) continue;
                                    toBytecode(c, t->left, s, chunkIdx);
                                }
                            }
                            addBinaryInstruction(c, OP_CALL, i, (sd!=NULL?1:0)+(currentNode->children == NULL ? 0 : currentNode->children->length), chunkIdx);
                            
                            return;
                        }
                    }
                }
                fatalError(0x2E, "Unknown function.", -1);
                break;
            }
            else if (currentNode->left->type == AST_STRUCTACCESS) {
                
                if (currentNode->left->left->type == AST_STRUCTACCESS) {
                    toBytecode(c, currentNode->left->left, s, chunkIdx);
                    if (currentNode->children != NULL) {
                        for (int j = 0; j < currentNode->children->length; j++) {
                            ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, j)->data;
                            if (t->type != AST_PARAM) continue;
                            toBytecode(c, t->left, s, chunkIdx);
                        }
                    }
                    structDefinition* sd = getStructType(currentNode->left->left, c, s);
                    if (sd == NULL) {
                        fatalError(0x23, "Invalid struct call", -1);
                    }
                    char* functionName = currentNode->left->right->value.nameVal;
                    char* actualName = malloc(strlen(functionName)+strlen(sd->name)+2);
                    sprintf(actualName, "%s_%s", sd->name, functionName);
                    for (int i = 0; i < c.functionIdentifiers->length; i++) {
                        bcFunction* bf = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
                        if (strcmp(actualName, bf->name) == 0) {
                            addBinaryInstruction(c, OP_CALL, i,(sd->name != NULL)+currentNode->children->length, chunkIdx);
                            free(actualName);
                            ;
                            return;
                        }
                    }
                    if (isDebug) printf("Function not found! %s", actualName);
                    break;
                }
                else if (currentNode->left->left->type == AST_IDENTIFIER) {
                    identifierLocation il = getIdentifier(c, s, currentNode->left->left->value.nameVal);
                    vmVariable* vmv = il.ptr;
                    structDefinition* sd = getStructDefViaName(vmv->structType, c);
                    char* functionName = currentNode->left->right->value.nameVal;
                    char* actualName = malloc(strlen(sd->name)+strlen(functionName)+2);
                    sprintf(actualName, "%s_%s", sd->name, functionName);
                    for (int i = 0; i < c.functionIdentifiers->length; i++) {
                        bcFunction* bf = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
                        if (strcmp(actualName, bf->name) == 0) {
                            
                            if (il.isGlobal) addUnaryInstruction(c, OP_LOAD_GLOBAL, il.index, chunkIdx);
                            else addUnaryInstruction(c, OP_LOAD_LOCAL, il.index, chunkIdx);
                            if (currentNode->children != NULL) {
                                for (int j = 0; j < currentNode->children->length; j++) {
                                    ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, j)->data;
                                    if (t->type != AST_PARAM) continue;
                                    toBytecode(c, t->left, s, chunkIdx);
                                }
                            }
                            addBinaryInstruction(c, OP_CALL, i, currentNode->children == NULL ? 1 : 1+currentNode->children->length, chunkIdx);
                            free(actualName);
                            return;
                        }
                    }
                }
                else if (currentNode->left->left->type == AST_FUNCCALL) {
                    // a().n()
                    toBytecode(c, currentNode->left->left, s, chunkIdx);
                    if (currentNode->children != NULL) {
                        for (int j = 0; j < currentNode->children->length; j++) {
                            ASTNode* t = (ASTNode*)List_GetElement(currentNode->children, j)->data;
                            if (t->type != AST_PARAM) continue;
                            toBytecode(c, t->left, s, chunkIdx);
                        }
                    }
                    structDefinition* sd = getStructType(currentNode->left->left, c, s);
                    char tempName[500];
                    sprintf(tempName, "%s_%s", sd->name, currentNode->left->right->value.nameVal);
                    for (int i = 0; i < c.functionIdentifiers->length; i++) {
                        bcFunction* funcName = List_GetElement(c.functionIdentifiers, i)->data;
                        if (strcmp(funcName->name, tempName) == 0) {
                            addBinaryInstruction(c, OP_CALL, i,currentNode->children == NULL ? 1 : 1+currentNode->children->length, chunkIdx);
                            ;
                            return;
                        }
                    }
                }
            }
        }
    
    }
}
// Converts an opcode into a string. Only used for debugging.
char* opToStr(Opcode o) {
    switch (o) {
        case OP_LOAD_CONST: {return "LOAD CONST";}
        case OP_LOAD_GLOBAL: {return "LOAD GLOBAL";}
        case OP_STORE_GLOBAL: {return "STORE GLOBAL";}
        case OP_ADD: {return "ADD";}
        case OP_SUB: {return "SUB";}
        case OP_MUL: {return "MUL";}
        case OP_DIV: {return "DIV";}
        case OP_CALL: {return "CALL";}
        case OP_RETURN: {return "RETURN";}
        case OP_JUMP: {return "JUMP";}
        case OP_JUMP_IF_FALSE: {return "JUMP IF FALSE";}
        case OP_HALT: {return "HALT";}
        case OP_NEG: {return "NEG";}
        case OP_GTHAN: {return "GTHAN";}
        case OP_LTHAN: {return "LTHAN";}
        case OP_GEQTHAN: {return "GEQTHAN";}
        case OP_LEQTHAN: {return "LEQTHAN";}
        case OP_EQUALS: {return "EQUALS";}
        case OP_OR: {return "OR";}
        case OP_AND: {return "AND";}
        case OP_NOT: {return "NOT";}
        
        case OP_LOAD_LOCAL: {return "LOAD LOCAL";}
        case OP_STORE_LOCAL: {return "STORE LOCAL";}

        case OP_LOAD_IDX: {return "LOAD IDX";}
        case OP_STORE_IDX: {return "STORE IDX";}
        case OP_ARRAY_LEN: {return "LEN";}
        case OP_BUILD_ARRAY: {return "BUILD ARRAY";}

        case OP_NEW_STRUCT: {return "NEW STRUCT";}
        case OP_GET_FIELD: {return "GET FIELD";}
        case OP_SET_FIELD: {return "SET FIELD";}
        case OP_CLEAR_STACK: {return "CLEAR STACK";}
        case OP_MOD: {return "MOD";}

    }
    return "UNKNOWN INSTRUCTION";
}
// When given an index, gets the bcFunction* definition of the function.
bcFunction* getFunc(byteCode c, int idx) {
    bool premainReached = false;
    int offsetIdx = idx;
    for (int i = 0; i < c.functionIdentifiers->length; i++) {
        bcFunction* bcf = (bcFunction*)List_GetElement(c.functionIdentifiers, i)->data;
        if (bcf->name == NULL) {
            offsetIdx = idx+i;
            break;
        }
        if (strcmp(bcf->name, "premain") == 0) {
            offsetIdx = idx+i;
            break;
        }
    }
    return ((bcFunction*)List_GetElement(c.functionIdentifiers, offsetIdx)->data);
}
// Prints bytecode
void printBytecode(byteCode c, bool numsOnly, bool numberInstructions) {

    printf("\nGLOBAL TABLE:\n");
    for (int i = 0; i < c.globals->length; i++) {
        vmVariable vmv = *(vmVariable*)List_GetElement(c.globals, i)->data;
        printf(" %d | %s %s\n", i, vmv.structType == NULL ? "" : vmv.structType, vmv.name);
    }
    printf("\nCONSTANT TABLE:\n");
    for (int i = 0; i < c.constants->length; i++) {
        typedValue* tv = (typedValue*)List_GetElement(c.constants, i)->data;
        if (tv->valueType == TYPE_NUM) {
            char* toPrint = malloc(1000);
            sprintfNum(tv->value.numberValue, toPrint, true);
            printf(" %d (num) | %s\n", i, toPrint);
            free(toPrint);
        }
        else if (tv->valueType == TYPE_ARRAY) {
            if (tv->value.av.arrayType != AT_UNKNOWN && tv->value.av.data != NULL) {
                typedValue** data = (typedValue**)tv->value.av.data;
                num nm = data[0]->value.numberValue;
                char* toSend = malloc(tv->value.av.len + 1);
                toSend[0] = '\0';
                if (nm.type == NUM_CHAR) {
                    for (int k = 0; k < tv->value.av.len; k++) {
                        toSend[k] = data[k]->value.numberValue.value.cVal;
                    }
                    toSend[tv->value.av.len] = '\0';
                }
                printf(" %d (array) | %s [", i, toSend);
                free(toSend);
                for (int j = 0; j < tv->value.av.len; j++) {
                    char toPrint[1000]; toPrint[0] = '\0';
                    typedValue* n = data[j];
                    if (n) sprintfNum(n->value.numberValue, toPrint, true);
                    printf(" %s ", toPrint);
                }
                printf("]\n");
            }
            else if (tv->value.av.len == 0) printf("\n%d (Empty array) | []\n", i);
        }
        else if (tv->valueType == TYPE_NULL) {
            printf("\n%d (null) | Null? \n", i);
        }
    }
    printf("STRUCT DEFINITIONS:\n");
    for (int i = 0; i < c.structDefs->length; i++) {
        structDefinition* sd = List_GetElement(c.structDefs, i)->data;
        printf(" STRUCT %s:\n", sd->name);
        for (int j = 0; j < sd->fields->length; j++) {
            char* fieldName = (char*)List_GetElement(sd->fields, j)->data;
            structTypes* st = (structTypes*)List_GetElement(sd->fieldTypes, j)->data;
            if (st->isPDT) printf("  %d. %s : %d%s\n", j, fieldName, st->type.pdtType, st->isArray ? "[]" : "");
            else printf("  %d. %s : %s%s\n", j, fieldName, st->type.name, st->isArray ? "[]" : "" );
        }
    }
    printf("BYTECODE:");
    for (int chunkIdx = 0; chunkIdx < c.chunks->length; chunkIdx++) {
        printf("\n %d -> %s Argc=%d:", chunkIdx, getFunc(c, chunkIdx)->name, getFunc(c, chunkIdx)->argc);
        for (int i = 0; i < getInstructions(c, chunkIdx)->length; i++) {
            instruction j = *(instruction*)List_GetElement(getInstructions(c, chunkIdx), i)->data;
            if (numberInstructions) printf("\n  %d. ", i);
            else printf("\n  ");
            if (!numsOnly) printf("%s", opToStr(j.code));
            else printf("%d", j.code);
            if (j.argc == 0) continue;
            for (int k = 0; k < j.argc; k++) {
                printf(" %d", j.args[k]);
            }
        }
    }
}
// Checks if the list of imports has a specific import.
bool hasImport(List* imports, char* name) {
    for (int i = 0; i < imports->length; i++) {
        if (strcmp((char*)List_GetElement(imports, i)->data, name) == 0) {
            return true;
        }
    }
    return false;
}
// Declares a new blank struct definition (used for the string & array structs)
structDefinition* newDef(char* name) {
    structDefinition* toReturn = malloc(sizeof(structDefinition));
    *toReturn = (structDefinition){
        .fields = malloc(sizeof(List)),
        .fieldTypes = malloc(sizeof(List)),
        .name = strdup(name)
    };
    *toReturn->fields = NewList();
    *toReturn->fieldTypes = NewList();
    return toReturn;
}
// Takes a file path, and inserts the parsed tree into toAppend.
void doDefinitionInsertion(const char* txt, ASTNode* toAppend) {
    List* tokens = tokenize(txt);
    ASTNode* program = parseTokenList(tokens);
    for (int i = program->children->length-1; i >= 0; i--) {
        List_InsertElement(toAppend->children, 0, (ASTNode*)List_GetElement(program->children, i)->data);
    }
}
// Imports aurum definitions from bytecoder.h.
// This is mainly used to import structs from various system libraries.
// However, the lalg library is written entirely in Aurum.
void initStructs(ASTNode* toAppend, byteCode* c, List* imports) {
    
    
    
    if (hasImport(imports, "math") && hasImport(imports, "cplx")) doDefinitionInsertion(cplxDefinitions, toAppend);
    if (!hasImport(imports, "math") && hasImport(imports, "cplx")) fatalError(0x40, "", -1);
    if (hasImport(imports, "math") && hasImport(imports, "lalg")) doDefinitionInsertion(lalgDefinitions, toAppend);
    if (!hasImport(imports, "math") && hasImport(imports, "lalg")) fatalError(0x40, "", -1);
    if (hasImport(imports, "math")) doDefinitionInsertion(mathDefinitions, toAppend);
    if (hasImport(imports, "time")) doDefinitionInsertion(timeDefinitions, toAppend);
    if (hasImport(imports, "io")) doDefinitionInsertion(ioDefinitions, toAppend);
    doDefinitionInsertion(defaultDefinitions, toAppend);
    List_AppendElement(c->structDefs, newDef("array"));
    List_AppendElement(c->structDefs, newDef("string"));
}
// Appends all functions from bcf into add.
void getFunctions(List* add, const bcFunction* bcf) {
    
    for (int i = 0; bcf[i].argc != -1; i++) {
        bcFunction bcfi = bcf[i];
        bcFunction* bcc = malloc(sizeof(bcFunction));
        *bcc = (bcFunction){
            .argc = bcf[i].argc,
            .name = strdup(bcfi.name),
            .isSystem = true,
            .returnStruct = bcfi.returnStruct == NULL ? NULL : strdup(bcfi.returnStruct)
        };
        List_AppendElement(add, bcc);
    }
    return;
}
// Initializes the bytecode struct.
byteCode initBytecode(List* imports) {
    // Allocate stuff
    byteCode c = (byteCode){
        .constants = malloc(sizeof(List)),
        .globals = malloc(sizeof(List)),
        .chunks = malloc(sizeof(List)),
        .functionIdentifiers = malloc(sizeof(List)),
        .structDefs = malloc(sizeof(List))
    };
    *c.constants = NewList();
    *c.globals = NewList();
    *c.chunks = NewList();
    *c.functionIdentifiers = NewList();
    *c.structDefs = NewList();
    // Adds zero & one for !opFunc(), which is the function that is responsible for doing ** & //
    typedValue* zero = malloc(sizeof(typedValue));
    *zero = (typedValue){
        .ptr = NULL,
        .value.numberValue = (num){.value.iVal = 0, .type = NUM_INT},
        .valueType = TYPE_NUM
    };
    typedValue* one = malloc(sizeof(typedValue));
    *one = (typedValue){
        .ptr = NULL,
        .value.numberValue = (num){.value.iVal = 1, .type = NUM_INT},
        .valueType = TYPE_NUM
    };
    List_AppendElement(c.constants, zero);
    List_AppendElement(c.constants, one);
    // If there's an import, moves the functions from that import into c.functionidentifiers.
    if (isDebug) {
        for (int i = 0; i < imports->length; i++) {
            printf("%s", (char*)List_GetElement(imports, i)->data);
        }
    }
    getFunctions(c.functionIdentifiers, defaultFunctions);
    if (hasImport(imports, "math")) getFunctions(c.functionIdentifiers, mathFunctions);
    if (hasImport(imports, "time")) getFunctions(c.functionIdentifiers, timeFunctions);
    if (hasImport(imports, "rand")) getFunctions(c.functionIdentifiers, randomFunctions);
    if (hasImport(imports, "cplx")) getFunctions(c.functionIdentifiers, cplxFunctions);
    if (hasImport(imports, "io")) getFunctions(c.functionIdentifiers, ioFunctions);
    // Both premain & main are guarenteed to have the indices of 0 & 1 respectively.
    // premain: area where globals & structs are initialized. No other things can happen.
    chunk* premain = malloc(sizeof(chunk));
    *premain = (chunk){
        .instructions = malloc(sizeof(List)),
        .name = strdup("premain")
    };
    *premain->instructions = NewList();
    List_AppendElement(c.chunks, premain);
    bcFunction* premainName = malloc(sizeof(bcFunction));
    *premainName = (bcFunction){
        .argc = 0,
        .name = strdup("premain"),
        .isSystem = false,
    };
    List_AppendElement(c.functionIdentifiers, premainName);
    // main: the entry-point for the vm.
    chunk* main = malloc(sizeof(chunk));
    *main = (chunk){
        .instructions = malloc(sizeof(List)),
        .name = strdup("main")
    };
    *main->instructions = NewList();
    List_AppendElement(c.chunks, main);
    bcFunction* mainName = malloc(sizeof(bcFunction));
    *mainName = (bcFunction){
        .argc = 0,
        .name = strdup("main"),
        .isSystem = false
    };
    List_AppendElement(c.functionIdentifiers, mainName);
    return c;
}
// Reads a text file.
char* readTextFile(const char* filename) {
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        if (isDebug) printf("Unable to open file!"); exit(1);
        return NULL;
    }

    // Move to end of file
    fseek(file, 0, SEEK_END);

    // Get file size
    long fileSize = ftell(file);

    // Return to beginning
    rewind(file);

    // Allocate memory (+1 for null terminator)
    char* buffer = malloc(fileSize + 1);

    if (buffer == NULL) {
        if (isDebug) printf("NULL");
        fclose(file);
        return NULL;
    }

    // Read file contents
    fread(buffer, 1, fileSize, file);

    // Null terminate string
    buffer[fileSize] = '\0';

    fclose(file);

    return buffer;
}
// Adds any imports into systemImports. Any non-system imports, then parse & add the file into prgm.
void doImports(ASTNode* prgm, List* systemImports) {
    for (int i = 0; i < prgm->children->length; i++) {
        ASTNode* currentNode = List_GetElement(prgm->children, i)->data;
        if (currentNode->type == AST_IMPORT) {
            if (currentNode->value.nameVal[0] == '@') {
                char* n = malloc(strlen(currentNode->value.nameVal)+1); // +1 for null terminator, -1 for no @.
                for (int j = 0; j < strlen(currentNode->value.nameVal); j++) {
                    n[j] = currentNode->value.nameVal[j+1]; 
                    n[j+1] = '\0';
                }
                for (int j = 0; j < systemImports->length; j++) {
                    if (strcmp((char*)List_GetElement(systemImports, j)->data, n) == 0) {
                        List_DeleteElement(prgm->children, i);
                        return;
                    }
                }
                List_DeleteElement(prgm->children, i);
                i--;
                bool found = false;
                for (int j = 0; j < systemImports->length; j++) {
                    if (strcmp(n, (char*)List_GetElement(systemImports, i)->data ) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) List_AppendElement(systemImports, n);
            }
            else {
                char nn[500];
                nn[0] = '\0';
                sprintf(nn, "%s//%s", projectFolder, currentNode->value.nameVal);
                
                char* readFile = readTextFile(nn);
                List* new = tokenize(readFile);
                ASTNode* node = parseTokenList(new);
                doImports(node, systemImports);
                for (int j = node->children->length-1; j >= 0; j--) {
                    ASTNode* n = List_GetElement(node->children, j)->data;
                    if (n->type != AST_PRGM && n->type != AST_IMPORT) {
                        List_InsertElement(prgm->children, 0, n);
                        i += 1;
                    }
                }
                List_DeleteElement(prgm->children, i);
                i--;
            }
            
        }
    }

}
// Recursively frees AST nodes.

byteCode* convertToBytecode(ASTNode* prgm) {
    List* imports = malloc(sizeof(List));
    *imports = NewList();
    doImports(prgm, imports);
    byteCode* c = malloc(sizeof(byteCode));
    *c = initBytecode(imports);
    List* breakStatements = malloc(sizeof(List));
    *breakStatements = NewList();
    scopeInfo globalScope = (scopeInfo){
        .breakStatements = breakStatements,
        .continueTgt = 0,
        .locals = NULL
    };
    initStructs(prgm, c, imports);
    toBytecode(*c, prgm, globalScope, 0);
    freeAST(prgm); 
    return c;
}
