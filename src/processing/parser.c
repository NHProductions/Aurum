/*
--- Parser.c ---
This file converts the token array into a tree-like structure that preserves order-of-operations & further simplifies the data.

In this stage, grouping symbols are removed (As the tree-like structure already has them),
In addition, during this stage, numbers are converted from strings into nums (see num.c), which is essentially just a union between all number types.
Semicolons are also removed.

Here's the token array from last time:
KW(function) IDENTIFIER(fizzbuzz) GROUPING(() KW(int) IDENTIFIER(amt) GROUPING()) KW(->) KW(void) GROUPING({)
    KW(for) GROUPING(() KW(int) IDENTIFIER(n) OP(=) NUMBER(1) DELIM(;) IDENTIFIER(n) OP(<=) IDENTIFIER(amt) DELIM(;) IDENTIFIER(n) OP(++) GROUPING()) GROUPING({)
        KW(if) GROUPING(() GROUPING(() IDENTIFIER(n) OP(%) NUMBER(3) OP(==) NUMBER(0) GROUPING()) OP(&&) GROUPING(() IDENTIFIER(n) OP(%) NUMBER(5) OP(!=) NUMBER(0) GROUPING()) GROUPING()) GROUPING({)
            IDENTIFIER(print) GROUPING(() STRING(fizz\n) GROUPING()) DELIM(;)
        GROUPING(})
        KW(elif) GROUPING(() GROUPING(() IDENTIFIER(n) OP(%) NUMBER(5) OP(==) NUMBER(0) GROUPING()) OP(&&) GROUPING(() IDENTIFIER(n) OP(%) NUMBER(3) OP(!=) NUMBER(0) GROUPING()) GROUPING()) GROUPING({)
            IDENTIFIER(print) GROUPING(() STRING(buzz\n) GROUPING()) DELIM(;)
        GROUPING(})
        KW(elif) GROUPING(() GROUPING(() IDENTIFIER(n) OP(%) NUMBER(5) OP(==) NUMBER(0) GROUPING()) OP(&&) GROUPING(() IDENTIFIER(n) OP(%) NUMBER(3) OP(==) NUMBER(0) GROUPING()) GROUPING()) GROUPING({)
            IDENTIFIER(print) GROUPING(() STRING(fizzbuzz\n) GROUPING()) DELIM(;)
        GROUPING(})
        KW(else) GROUPING({)
            IDENTIFIER(print) GROUPING(() STRING(%i) DELIM(,) IDENTIFIER(n) GROUPING()) DELIM(;)
        GROUPING(})
    GROUPING(})
GROUPING(})

So, here's what this would be after parsing; nodes without "Left" or "Right" are apart of their parent's node's children list.:
FUNCTION
    Left: IDENTIFIER(fizzbuzz)
    Right: DATATYPE(int)
    PARAM:
        DATATYPE(int)
        IDENTIFIER(amt)
    BLOCK:
        FOR:
            VARDECLARATION:
                Left: DATATYPE(int)
                Right: IDENTIFIER(i)
                NUMBER(1)
            OPERATOR(<=)
                IDENTIFIER(n)
                IDENTIFIER(amt)
            VARASSIGNMENT:
                Left: NULL
                Right: IDENTIFIER(n)
                OPERATOR(+)
                    IDENTIFIER(n)
                    NUMBER(1)
            BLOCK:
                IF:
                    CONDITIONAL:
                        OPERATOR(&&)
                            OPERATOR(==)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(3)
                                NUMBER(0)
                            OPERATOR(!=)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(5)
                                NUMBER(0)
                        BLOCK:
                            FUNCTIONCALL:
                                Left: IDENTIFIER(print)
                                PARAMETER:
                                    STRING("fizz\n");
                    CONDITIONAL:
                        OPERATOR(&&)
                            OPERATOR(==)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(5)
                                NUMBER(0)
                            OPERATOR(!=)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(3)
                                NUMBER(0)
                        BLOCK:
                            FUNCTIONCALL:
                                Left: IDENTIFIER(print)
                                PARAMETER:
                                    STRING("buzz\n")
                    CONDITIONAL:
                        OPERATOR(&&)
                            OPERATOR(==)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(5)
                                NUMBER(0)
                            OPERATOR(==)
                                OPERATOR(%)
                                    IDENTIFIER(n)
                                    NUMBER(3)
                                NUMBER(0)
                        BLOCK:
                            FUNCTIONCALL:
                                Left: IDENTIFIER(print)
                                PARAMETER:
                                    STRING("fizzbuzz\n")
                    ELSE:
                        BLOCK:
                            FUNCTIONCALL:
                                Left: IDENTIFIER(print)
                                PARAMETER:
                                    STRING("%i")
                                    IDENTIFIER(n)

Files to check next: parser.h num.c, num.h, bytecoder.c
*/
// Standard headers:
#include <stdio.h>
// Aurum headers:
#include "../dataStorage/Tundora.h"
#include "parser.h"
#include "lexer.h"
#include "../misc/winInclude.h"
#include "bytecoder.h"
#include "../dataStorage/array.h"
// Functions declarations:
ASTNode* evaluateExpression(ASTNode* n);
ASTNode* parseIfStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s);
ASTNode* parseWhileStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s);
ASTNode* parseForStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s);
ASTNode* parseFunctionDeclaration(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s);
ASTNode* parseAssignment(int* CI, int* nextDelim, List* tokens, scope* s);
ASTNode* parseSwitchStatement(int* CI, int* nextDelim, List* tokens, scope* s);
ASTNode* parseStructStatement(int* CI, int* nextDelim, List* tokens, scope* s);
void printScope(scope s);

// Allocates a new AST node.
ASTNode* newASTNode() {
    ASTNode* toReturn = malloc(sizeof(ASTNode));
    *toReturn = (ASTNode){
        .children = NULL, 
        .left = NULL,
        .right = NULL,
        .type = AST_NONE,
        .value.numberVal = 0
    };
    return toReturn;
};
// Creates a new Num node from a num.
ASTNode* newNumNode(num a) {
    ASTNode* toReturn = newASTNode();
    toReturn->left = NULL;
    toReturn->right = NULL;
    toReturn->type = AST_NUMBER;
    toReturn->value.numVal = (num){
        .type = a.type,
        .value = a.value
    };
    return toReturn;
}
// Creates a new identifier node from a string.
ASTNode* newIdentifierNode(char* name) {
    ASTNode* toReturn = newASTNode();
    toReturn->left = NULL;
    toReturn->right = NULL;
    toReturn->type = AST_IDENTIFIER;
    toReturn->value.nameVal = strdup(name);
    return toReturn;
};
// Creates a new operator node from a string.
ASTNode* newOperatorNode(char* op, ASTNode* Left, ASTNode* Right) {
    ASTNode* toReturn = newASTNode();
    toReturn->left = Left;
    toReturn->right = Right;
    toReturn->type = AST_OPERATOR;
    toReturn->value.opVal = op;
    return toReturn;
};

// List of arithmetic operations from having least precedence (processed first in order of operations) to most (processed last).
typedef enum {
    NULLLAYER,
    EQ, // =
    LOR, // ||
    LXOR, // ^^
    LAND, // &&
    BOR, // |
    BXOR, // ^
    BAND, // &
    EQUALITY, // == !=
    COMPARISON, // >= <= < >
    BITSHIFT, // << >>
    ADDSUB, // +-
    MULDIV, // */
    EXP, // ^
    UNARY, // unary -, !, and ~
    PRIMARY // Function calls, identifiers, array accesses;
} arithmeticOperationPrecedence;
// Gets the next precedence
arithmeticOperationPrecedence getNextPrecedence(arithmeticOperationPrecedence aop) {
    if (aop == PRIMARY) return NULLLAYER;
    return aop + 1;
}
// List of operators by several arrays of strings. These arrays are terminated by an empty string.
static char* eqOps[] = {"=", "|=", "^=", "--", "++", "&=", ">>=", "<<=", "-=", "+=", "/=", "//=", "%=", "**=", "*=", "\0"};
static char* lorOps[] = {"||", "\0"};
static char* lxorOps[] = {"^^", "\0"};
static char* landOps[] = {"&&", "\0"};
static char* borOps[] = {"|", "\0"};
static char* bxorOps[] = {"^", "\0"};
static char* bandOps[] = {"&", "\0"};
static char* equalityOps[] = {"==", "!=", "\0"};
static char* compOps[] = {">", "<", ">=", "<=", "\0"};
static char* bitShiftOps[] = {">>", "<<", "\0"};
static char* addSubOps[] = {"-", "+", "\0"};
static char* mulDivOps[] = {"*", "/", "//", "%", "\0"};
static char* expOps[] = {"**", "\0"};
static char* unaryOps[] = {"!", "~", "-", "#", "&", "\0"};
// Gets a list of operators when given an aop.
char** getOperators(arithmeticOperationPrecedence aop) {
    switch (aop) {
        case EQ: {return eqOps;}
        case ADDSUB: {return addSubOps;}
        case MULDIV: {return mulDivOps;}
        case EXP: {return expOps;}
        case LOR: {return lorOps;}
        case LXOR: {return lxorOps;}
        case LAND: {return landOps;}
        case BOR: {return borOps;}
        case BXOR: {return bxorOps;}
        case BAND: {return bandOps;}
        case EQUALITY: {return equalityOps;}
        case COMPARISON: {return compOps;}
        case BITSHIFT: {return bitShiftOps;}
        case UNARY: {return unaryOps;}
        default: {return eqOps;}
    }
    printf("Illegal getOperators call! %d", aop);
    return NULL;
}
// Checks if a node is an operator, and that the operator is equal to the string given.
bool isOperator(ASTNode* a, char* b) {
    if (a->type == AST_OPERATOR) {
        if (strcmp(a->value.opVal, b) == 0) {
            return true;
        }
    }
    return false;
}
// Counts the amount of strings in a string array terminated by a empty string.
int countStrsArr(char** c) {
    int l = 0;
    int max = 50;
    if (c == NULL) return 0;
    while (l < max) {
        if (strlen(c[l]) == 0) break;
        l++;
    }
    return l;
}
// Converts a token type to an AST node type.
ASTType tokenTypeToASTType(tokenType t) {
    switch (t) {
        case T_IDENTIFIER: {return AST_IDENTIFIER;}
        case T_OPERATOR: {return AST_OPERATOR;}
        case T_NUMBER: {return AST_NUMBER;}
        case T_STRING: {return AST_STRING;}
        case T_KEYWORD: {return AST_NONE;}
        case T_GROUPING: {return AST_BLOCKEND;}
        case T_DELIMITER: {return AST_EXPREND;}
        case T_LIBRARY: {return AST_LIBRARY;}
        default: {return AST_NONE;}
    }
    return AST_NONE;
}
// Converts a identifier specifier (const) to a string (CONST).
char* IDspecToString(varSpecifiers v) {
    switch (v) {
        case VS_CONST: {return "CONST";}
    }
    return NULL;
}
// Converts an AST node to a string. Used mainly in printAST() / debugging.
char* ASTToString(ASTNode* node) {
    // Print node info
    char toReturn[500];
    toReturn[0] = '\0';
    switch (node->type) {
        case AST_NUMBER: {
            sprintfNum(node->value.numVal, toReturn, true);
            break;
        }
        case AST_NONE: {
            printf("AST None");
            exit(1);
        }
        case AST_LIBRARY: {sprintf(toReturn, "LIBRARY: %s", node->value.nameVal); break;}
        case AST_IMPORT: {sprintf(toReturn, "IMPORT %s", node->value.nameVal); break;}
        case AST_IDENTIFIER: {sprintf(toReturn,"IDENTIFIER: %s", node->value.nameVal); break;}
        case AST_OPERATOR: {sprintf(toReturn,"OPERATOR: %s", node->value.opVal);break;}
        case AST_PRGM: {sprintf(toReturn,"PROGRAM:"); break;}
        case AST_STRING: {sprintf(toReturn,"STRING: \"%s\"", node->value.anyVal); break;}
        case AST_ASSIGNMENT: {sprintf(toReturn,"VAR ASSIGNMENT: %s", node->value.nameVal); break;}
        case AST_VARDECLARATION: {sprintf(toReturn,"VAR DECLARATION: %s", node->value.nameVal); break;}
        case AST_IF: {sprintf(toReturn,"IF STATEMENT:");break;}
        case AST_CONDITIONAL: {sprintf(toReturn,"CONDITIONAL:"); break;}
        case AST_BLOCK: {sprintf(toReturn,"BLOCK:"); break;}
        case AST_WHILE: {sprintf(toReturn,"WHILE:"); break;}
        case AST_ELSE: {sprintf(toReturn,"ELSE:"); break;}
        case AST_CONTINUE: {sprintf(toReturn,"CONTINUE;"); break;}
        case AST_BREAK: {sprintf(toReturn,"BREAK;"); break;}
        case AST_FOR: {sprintf(toReturn,"FOR:");break;}
        case AST_FUNCDEL: {sprintf(toReturn, "FUNCTION: "); break;}
        case AST_BLOCKEND: {sprintf(toReturn, "BLOCKEND; %s", node->value.anyVal); break;}
        case AST_EXPREND: {sprintf(toReturn, "EXPREND"); break;}
        case AST_VOID: {sprintf(toReturn, "VOID"); break;}
        case AST_ARROW: {sprintf(toReturn, "->"); break;}
        case AST_DATATYPE: {sprintf(toReturn, "DATATYPE: %f", node->value.numberVal); break;}
        case AST_PARAM: {sprintf(toReturn, "PARAMETER %d", node->type); break;}
        case AST_IDENTIFIERSPECIFIER: {sprintf(toReturn, "%s", IDspecToString(node->value.numberVal) ); break;}
        case AST_RETURN: {sprintf(toReturn, "RETURN", node->value.anyVal); break;}
        case AST_FUNCCALL: {sprintf(toReturn, "FUNCTION CALL: %s", node->value.nameVal); break;}
        case AST_ARRAYLITERAL: {sprintf(toReturn, "ARRAY LITERAL:"); break;}
        case AST_ARRAYACCESS: {sprintf(toReturn, "ARRAY ACCESS"); break;}
        case AST_ARRAYASSIGNMENT: {sprintf(toReturn, "ARRAY ASSIGNMENT: %s", node->value.nameVal); break;}
        case AST_SWITCH: {sprintf(toReturn, "SWITCH: %s", node->value.nameVal); break;}
        case AST_CASE: {sprintf(toReturn, "CASE:"); break;}
        case AST_STRUCTACCESS: {sprintf(toReturn, "STRUCT ACCESS: "); break;}
        case AST_STRUCT: {sprintf(toReturn, "STRUCT %s: ", node->value.nameVal); break;}
        case AST_COMPLEXDATATYPE: {sprintf(toReturn, "COMPLEX DATATYPE %s:", node->value.nameVal); break;}
        default: {
            sprintf(toReturn, "UNKNOWN NODE: %d", node->type); break;
        }
    }
    return strdup(toReturn);
}
// Converts a lexer token into an AST Node (NOT the same as parsing; this just converts the list of tokens into a list of ASTNodes, which make it easier to parse later on.)
ASTNode* tokenToASTNode(token t) {
    ASTNode* toReturn = newASTNode();
    toReturn->left = NULL;
    toReturn->right = NULL;
    toReturn->type = tokenTypeToASTType(t.type);
    
    switch (t.type) {
        // If it's an operator, library, grouping, or identifier, then just strdup() the value.
        case T_OPERATOR: {
            toReturn->value.opVal = strdup(t.value);
            free(t.value);
            break;
        }
        case T_LIBRARY: {
            toReturn->value.nameVal = strdup(t.value);
            free(t.value);
            break;
        }
        case T_IDENTIFIER: {
            toReturn->value.nameVal = strdup(t.value);
            free(t.value);
            break;
        }
        case T_GROUPING: {
            toReturn->value.anyVal = strdup(t.value);
            free(t.value);
            break;
        }
        // If it's a number, convert the token into a num struct, and store it in toReturn.
        case T_NUMBER: {
            toReturn->value.numVal = strToNum(t);
            free(t.value);
            break;
        }
        // For chars, just convert it into a number.
        case T_CHAR: {
            toReturn->type = AST_NUMBER;
            toReturn->value.numVal = (num){
                .type = NUM_CHAR,
                .value.cVal = t.value[0]
            };
            free(t.value);
            break;
        }
        // toReturn.type was already assigned via tokenTypeToASTType().
        case T_DELIMITER: {
            return toReturn;
        }
        // Strdup the token, and free it.
        case T_STRING: {
            toReturn->value.anyVal = strdup(t.value);
            free(t.value);
            return toReturn;
        }
        // For keywords, it's a bit more complex; this just goes through the keywords, checking if the token is that word.
        case T_KEYWORD: {
            // For booleans, just convert it into a number.
            if (strcmp(t.value, "false") == 0) {
                toReturn->type = AST_NUMBER;
                toReturn->value.numVal = (num){.type = NUM_BOOL, .value.bVal = false};
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "true") == 0) {
                toReturn->type = AST_NUMBER;
                toReturn->value.numVal = (num){.type = NUM_BOOL, .value.bVal = true};
                free(t.value);
                return toReturn;
            }
            // For identifier-specifiers (such as const), convert it into AST_IDS, and set it's numberVal (which is a double, not a num struct) into a VS enum.
            if (strcmp(t.value, "const") == 0) {                
                toReturn->type = AST_IDENTIFIERSPECIFIER;
                if (strcmp(t.value, "const") == 0) toReturn->value.numberVal = VS_CONST;
                free(t.value);
                return toReturn;
            }
            // For NaN and inf, convert it into a float.
            if (strcmp(t.value, "NaN") == 0) {
                toReturn->type = AST_NUMBER;
                toReturn->value.numVal = (num){.type = NUM_FLOAT, .value.fVal = nanf("")};
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "inf") == 0) {
                toReturn->type = AST_NUMBER;
                toReturn->value.numVal = (num){.type = NUM_FLOAT, .value.fVal = INFINITY};
                free(t.value);
                return toReturn;
            }
            // converts import statements into AST_import.
            if (strcmp(t.value, "import") == 0) {toReturn->type = AST_IMPORT;toReturn->value.nameVal = strdup(t.value); free(t.value); return toReturn;}
            // For datatypes, just convert it into AST_DATATYPE, and assign it's numberVal into the PDT enum.
            if (strcmp(t.value, "string") == 0) {toReturn->type = AST_DATATYPE; toReturn->value.numberVal = PDT_STRING;free(t.value); return toReturn;}
            if (strcmp(t.value, "auto") == 0) {toReturn->type = AST_DATATYPE; toReturn->value.numberVal = PDT_AUTO; free(t.value);return toReturn;}
            if (strcmp(t.value, "bool") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_BOOL;free(t.value); return toReturn;}
            if (strcmp(t.value, "char") == 0 || strcmp(t.value, "int8") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_CHAR; free(t.value);return toReturn;}
            if (strcmp(t.value, "short") == 0 || strcmp(t.value, "int16") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_SHORT;free(t.value); return toReturn;}
            if (strcmp(t.value, "int") == 0 || strcmp(t.value, "int32") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_INT; free(t.value);return toReturn;}
            if (strcmp(t.value, "long") == 0 || strcmp(t.value, "int64") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_LONG;free(t.value); return toReturn;}
            if (strcmp(t.value, "float") == 0 || strcmp(t.value, "double32") == 0) {toReturn->type = AST_DATATYPE; toReturn->value.numberVal = PDT_FLOAT; free(t.value);return toReturn;}
            if (strcmp(t.value, "double") == 0 || strcmp(t.value, "double64") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_DOUBLE; free(t.value);return toReturn;}
            if (strcmp(t.value, "longdouble") == 0 || strcmp(t.value, "double128") == 0) {toReturn->type = AST_DATATYPE; toReturn->value.numberVal = PDT_LONGDOUBLE;free(t.value); return toReturn;}
            if (strcmp(t.value, "uchar") == 0 || strcmp(t.value, "uint8") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_UCHAR; free(t.value);return toReturn;}
            if (strcmp(t.value, "ushort") == 0 || strcmp(t.value, "uint16") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_USHORT; free(t.value);return toReturn;}
            if (strcmp(t.value, "uint") == 0 || strcmp(t.value, "uint32") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_UINT; free(t.value);return toReturn;}
            if (strcmp(t.value, "ulong") == 0 || strcmp(t.value, "uint64") == 0) {toReturn->type = AST_DATATYPE;toReturn->value.numberVal = PDT_ULONG; free(t.value);return toReturn;}
            // For the rest of these keywords, it just converts the AST type into the AST Type that matches it.
            if (strcmp(t.value, "if") == 0) {
                toReturn->type = AST_IF;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "elif") == 0) {
                toReturn->type = AST_ELIF;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "else") == 0) {
                toReturn->type = AST_ELSE;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "while") == 0) {
                toReturn->type = AST_WHILE;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "continue") == 0) {
                toReturn->type = AST_CONTINUE;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "break") == 0) {
                toReturn->type = AST_BREAK;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "for") == 0) {
                toReturn->type = AST_FOR;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "function") == 0) {
                toReturn->type = AST_FUNCDEL;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "->") == 0) {
                toReturn->type = AST_ARROW;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "void") == 0) {
                toReturn->type = AST_VOID;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "return") == 0) {
                toReturn->type = AST_RETURN;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, ",") == 0) {
                toReturn->type = AST_COMMA;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, ".") == 0) {
                toReturn->type = AST_DOT;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "switch") == 0) {
                toReturn->type = AST_SWITCH;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "case") == 0) {
                toReturn->type = AST_CASE;
                free(t.value);
                return toReturn;
            }
            if (strcmp(t.value, "struct") == 0) {
                toReturn->type = AST_STRUCT;
                free(t.value);
                return toReturn;
            }
            break;
        }
        default: {
            if (isDebug) printf("Unknown token %d", t.type); exit(1);
            toReturn->value.anyVal = strdup(t.value);
            break;
        }
    }
    return toReturn;
}
// When given a list of AST tokens, start index, and valid openParen & closeParen, it finds the right grouping char
// e.x in a list of [{ ... }], it would find the closing paren for {.
int findRightParen(List tokens, int startIndex, char openParen, char closeParen) {
    int depth = -1;
    for (int i = startIndex; i < tokens.length;i++) {
        ASTNode* token = (ASTNode*)List_GetElement(&tokens, i)->data;
        if (token->type != AST_BLOCKEND) continue;
        if (token->value.anyVal[0] == closeParen && depth == 0) {
            return i;
        }
        else if (token->value.anyVal[0] == closeParen)
        {
            depth--;
        }  
        if (token->value.anyVal[0] == openParen) {
            depth++;
        }
        
    }
    // If no right parentheses was found, then throw an error.
    if (isDebug) {
        for (int i = 0; i < tokens.length; i++) {
            printAST(List_GetElement(&tokens, 0)->data, i);
        }
        fatalError(9, "Right parentheses not found", -1);
    }
    return 0;
}
/*
    Checks if a comma is surrounded on both sides by (). If it is, then it's a function call, and thus not valid for array literals (it needs to be coutned inside the function).
    If it isn't, then it's legal for array literals (as it seperates elements).
*/
bool isCommaValid(List* tokens, int commaIdx, int lBracket, int rBracket, char invalidLeft, char invalidRight) {
    ASTNode* currentNode = (ASTNode*)List_GetElement(tokens, commaIdx)->data;
    if (currentNode->type != AST_COMMA) return false;
    if (lBracket < 0 || rBracket < 0 || lBracket >= tokens->length || rBracket >= tokens->length) return false;
    if (commaIdx <= lBracket || commaIdx >= rBracket) return false;

    int depth = 0;
    for (int i = lBracket + 1; i < commaIdx; i++) {
        ASTNode* cn = (ASTNode*)List_GetElement(tokens, i)->data;
        if (cn->type != AST_BLOCKEND) continue;
        char c = cn->value.anyVal[0];
        if (c == invalidLeft) {
            depth++;
        }
        else if (c == invalidRight) {
            if (depth > 0) {
                depth--;
            }
        }
    }

    return depth == 0;
}
// Returns if the comma is a top level comma (e.x seperates a function parameter, or array stuff.)
static bool isTopLevelComma(List* tokens, int commaIdx, int startIdx, int endIdx) {
    if (commaIdx <= startIdx || commaIdx >= endIdx) return false;

    int parenDepth = 0;
    int bracketDepth = 0;
    int braceDepth = 0;

    for (int i = startIdx; i < commaIdx; i++) {
        ASTNode* current = (ASTNode*)List_GetElement(tokens, i)->data;
        if (current->type != AST_BLOCKEND) continue;

        char c = current->value.anyVal[0];
        switch (c) {
            case '(':
                parenDepth++;
                break;
            case ')':
                if (parenDepth > 0) parenDepth--;
                break;
            case '[':
                bracketDepth++;
                break;
            case ']':
                if (bracketDepth > 0) bracketDepth--;
                break;
            case '{':
                braceDepth++;
                break;
            case '}':
                if (braceDepth > 0) braceDepth--;
                break;
            default:
                break;
        }
    }

    return parenDepth == 0 && bracketDepth == 0 && braceDepth == 0;
}
// Makes a sublist of a list.
List* makeSublist(List tokens, int start, int end) {
    List* ToReturn = malloc(sizeof(List));
    *ToReturn = NewList();

    if (start > end) return ToReturn;

    for (int i = start; i <= end; i++) {
        List_AppendElement(ToReturn,
        List_GetElement(&tokens, i)->data);
    }

    return ToReturn;
}
// Prints an AST node
int lnNums = -1;
void printAST(ASTNode* node, int depth) {
    // Cosmetic stuff:
    if (depth == 0) {
        printf("\n");
    }
    if (node == NULL) return;
    for (int i = 0; i < depth; i++) {
        printf("  "); // two spaces per level
        if (lnNums >= 0) lnNums++;
        if (lnNums >= 0) {
            printf(" %d.", lnNums);
        }
    }
    // Converts the current node into a string, prints it, then recurses into the children.
    char* toFree = ASTToString(node);
    printf("%s\n", toFree);
    free(toFree);
    // Recurse into children
    if (node->left != NULL)  printAST(node->left, depth + 1);
    if (node->right != NULL) printAST(node->right, depth + 1);
    if (node->children != NULL && node->type != AST_NUMBER && node->type != AST_IDENTIFIER && node->type != AST_OPERATOR && node->type != AST_STRING) {
        if (node->children->length > 0) {
            for (int i = 0; i < node->children->length; i++) {
                ASTNode* child = (ASTNode*)List_GetElement(node->children, i)->data;
                if (child != NULL) printAST(child, depth + 1);
            }
        }
    }
}
// Recursively frees an ASTNode.
void freeAST(ASTNode* p) {

    if (p == NULL) return;
    if (p->left != NULL) {freeAST(p->left); p->left = NULL;}
    if (p->right != NULL) {freeAST(p->right); p->right = NULL;}
    if (p->children != NULL) {
       for (int i = 0; i < p->children->length; i++) {
            freeAST((ASTNode*)List_GetElement(p->children, i)->data);
       }
    }
    if (p->children != NULL) {
        freeListKeepData(p->children);
        p->children = NULL;
    }
    free(p);
    p = NULL;
}
struct identifierExistsResult {
    bool exists;
    void* dest;
    int type;
};
struct identifierExistsResult identifierExists(scope* s, char* name);
/*
This function parses an expression into an ASTNode

An expression here is considered basically anything that isn't any of the other categories
examples:
2+5*9
functionCall(5,19,55)*219;
[6,1,2]
struct.Access * 29
struct.Function([121,5]);
And any chained expressions with the above.

It does this by recursively adding tokens to a tree depending on the current operating precedence level
e.x first it'll split according to '=', then bitwise operators, then arithmetic operators, and so on until primary expressions.

doesn't actually evaluate any expressions; that's what evaluateExpression() does.
*/
ASTNode* parseExpression(List* tokens, int startIndex, int* nextIndex, arithmeticOperationPrecedence aop) {
    int idx = startIndex;

    // Checks if parameters are valid (aop & startIndex).
    if (aop == NULLLAYER) return NULL;
    if (startIndex >= tokens->length) return NULL;
    
    // Checks the current index to see if it's valid for parsing (Identifiers, array accessing, operators, grouping symbols, numbers).
    // If it's invalid, copies the node & returns it.
    ASTNode* toCheck = ((ASTNode*)List_GetElement(tokens, startIndex)->data) ;
    bool isInvalidNode = toCheck->type != AST_IDENTIFIER && toCheck->type != AST_ARRAYACCESS && toCheck->type != AST_OPERATOR && toCheck->type != AST_NUMBER && toCheck->type != AST_BLOCKEND ;
    if (toCheck->type == AST_BLOCKEND) {
        if (toCheck->value.anyVal[0] != '(' && toCheck->value.anyVal[0] != ')' && toCheck->value.anyVal[0] != '[' && toCheck->value.anyVal[0] != ']') isInvalidNode = true;
    }
    if (isInvalidNode) {
        ASTNode* node = malloc(sizeof(ASTNode));
        *node = (ASTNode){
            .children = toCheck->children,
            .left = toCheck->left,
            .right = toCheck->right,
            .type = toCheck->type,
            .value = toCheck->value
        };
        //free(toCheck);
        *nextIndex = idx + 1;
        return node;
    }
    // Parses equality, boolean operations, bitwise operations, comparison, bitshifts, & arithmetic operations.
    // splits a list into two (left & right), then merges them under an operator node.
    if (aop != PRIMARY && aop != UNARY) { 
        char** ops = getOperators(aop);
        int opLen = countStrsArr(ops);
        ASTNode* left = parseExpression(tokens, idx, &idx, getNextPrecedence(aop));
        while (idx < tokens->length) {
            ASTNode* token = (ASTNode*)List_GetElement(tokens, idx)->data;
            if (token->type != AST_OPERATOR) break;
            bool found = false;
            char* opVal = "";
            for (int i = 0; i < opLen; i++) {
                if (strcmp(ops[i], token->value.opVal) == 0 ) {found = true; opVal = strdup(ops[i]); break;}
            }
            if (!found) break;
            ASTNode* opNode = malloc(sizeof(ASTNode));
            opNode->type = AST_OPERATOR;
            opNode->value.opVal = opVal;
            opNode->left = left;
            opNode->children = NULL;
            idx++;
            opNode->right = parseExpression(tokens, idx, &idx, getNextPrecedence(aop));
            left = opNode;
        }
        *nextIndex = idx;
        return left;
    }
    // Parses unary operators
    else if (aop == UNARY) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens,idx)->data;
        char** unaryOps = getOperators(aop);
        int unaryOpsLen = countStrsArr(unaryOps);
        if (token->type == AST_OPERATOR) {
            char* op = token->value.opVal;
            bool found = false;
            for (int i = 0; i < unaryOpsLen; i++) {
                if (strcmp(op, unaryOps[i]) == 0) {found = true; break;}
            }
            if (found) {
                idx++;
                ASTNode* operand = parseExpression(tokens, idx, &idx, UNARY);
                ASTNode* node = newASTNode();
                node->type = AST_OPERATOR;
                node->value.opVal = strdup(op);
                node->left = NULL;
                node->right = operand;
                node->children = NULL;
                *nextIndex = idx;
                return node;
            }
        }
        return parseExpression(tokens, idx, nextIndex, PRIMARY);
    }
    // Parses primary expressions (function calls, array access, struct access, identifiers)
    else {
        // Checks if idx is valid
        if (idx >= tokens->length) {
            *nextIndex = idx;
            return NULL;
        }
        // Gets the current token. If it's a number, return it.
        ASTNode* token = (ASTNode*)List_GetElement(tokens, idx)->data;
        if (token->type == AST_NUMBER) {
            ASTNode* node = malloc(sizeof(ASTNode));
            *node = (ASTNode){
                .children = NULL,
                .value = token->value,
                .left = NULL,
                .right = NULL,
                .type = AST_NUMBER
            };
            //free(token);
            *nextIndex = idx + 1;
            return node;
        }
        // If it's an identifier, get the next token and proceed accordingly.
        if (token->type == AST_IDENTIFIER) {
            ASTNode* node = newASTNode();
            node->type = AST_IDENTIFIER;
            ASTNode* toCheck = NULL;
            if (idx+1 < tokens->length-1) toCheck = (ASTNode*)List_GetElement(tokens, idx+1)->data;
            bool idxChanged = false;
            
            int endIdx = -1;
            ASTNode* toReturnCpy = NULL;
            if (toCheck != NULL) {
            // Checks if toCheck is a grouping symbol (function call or array access), or dot (struct access).
            if (toCheck->type == AST_BLOCKEND || toCheck->type == AST_DOT) {
                ASTNode* toReturn = newASTNode();
                toReturn->type = AST_NONE;
                // If it is, then loops until the next token isn't one of those, parsing struct access, array access, and function calls.
                while (toCheck->type == AST_BLOCKEND || toCheck->type == AST_DOT) {
                    bool isInitialized = toReturn->type == AST_NONE;
                    // Gets the type of the resulting node; if toCheck is a dot, then it's AST_STRUCTACCESS, if the first character in toCheck.value is [, then it's array access, otherwise it's a function call.
                    int type = toCheck->type == AST_DOT ? AST_STRUCTACCESS : AST_NONE;
                    if (type == AST_NONE) type = toCheck->value.anyVal[0] == '[' ? AST_ARRAYACCESS : AST_FUNCCALL;
                    // Function/Array Access Parsing:
                    if (type != AST_STRUCTACCESS) {
                        // Grab the right parentheses
                        char leftChar = toCheck->value.anyVal[0];
                        char rightChar = leftChar == '[' ? ']' : ')';
                        int rightIdx = findRightParen(*tokens, idx+1, leftChar, rightChar);
                        // If the previous index wasn't negative one, then copy the previous value of toReturn, and set toReturn to the new node. Otherwise, continue.
                        if (endIdx != -1) {
                            toReturnCpy = newASTNode();
                            toReturnCpy->children = toReturn->children;
                            toReturnCpy->left = toReturn->left;
                            toReturnCpy->right = toReturn->right;
                            toReturnCpy->type = toReturn->type;
                            toReturnCpy->value = toReturn->value;
                            toReturn->children = malloc(sizeof(List));
                            *toReturn->children = NewList();
                        }
                        toReturn->type = type;
                        // If it's AST_ARRAYACCESS, then parse/evaluate the index, and set toReturn.left to the array, and toReturn.right to the index.
                        if (toReturn->type == AST_ARRAYACCESS) {
                            toReturn->left = endIdx != -1 ? toReturnCpy : newIdentifierNode(token->value.nameVal);
                            List* SL = makeSublist(*tokens, idx+2, rightIdx-1);
                            int a;
                            toReturn->right = parseExpression(SL, 0, &a, EQ);
                            toReturn->right = evaluateExpression(toReturn->right);
                            freeListKeepData(SL);
                        }
                        // If it's a function call, parse the parameters (setting them as children to toReturn), set toReturn.left to the function name, and toReturn.right NULL.
                        // It does this by iterating through the tokens, and if it sees a top level comma, it grabs all values from the previous comma to this comma, and parses them.
                        // and appends them to toReturn.children as an AST_PARAM.
                        if (toReturn->type == AST_FUNCCALL) {
                            int c = idx + 2;
                            toReturn->children = malloc(sizeof(List));
                            *toReturn->children = NewList();
                            toReturn->left = endIdx != -1 ? toReturnCpy : newIdentifierNode(token->value.nameVal);
                            toReturn->right = NULL;

                            if (c < rightIdx) {
                                int argStart = c;
                                for (int i = c; i <= rightIdx; i++) {
                                    ASTNode* ct = (ASTNode*)List_GetElement(tokens, i)->data;
                                    if (ct->type == AST_COMMA && isTopLevelComma(tokens, i, c, rightIdx)) {
                                        if (i > argStart) {
                                            ASTNode* newParam = newASTNode();
                                            newParam->type = AST_PARAM;
                                            int a = 0;
                                            List* SL = makeSublist(*tokens, argStart, i - 1);
                                            ASTNode* toAppend = parseExpression(SL, 0, &a, EQ);
                                            toAppend = evaluateExpression(toAppend);
                                            newParam->left = toAppend;
                                            List_AppendElement(toReturn->children, newParam);
                                            freeListKeepData(SL);
                                        }
                                        argStart = i + 1;
                                    }
                                    else if (i == rightIdx) {
                                        if (argStart < rightIdx) {
                                            ASTNode* newParam = newASTNode();
                                            newParam->type = AST_PARAM;
                                            int a = 0;
                                            List* SL = makeSublist(*tokens, argStart, rightIdx - 1);
                                            ASTNode* toAppend = parseExpression(SL, 0, &a, EQ);
                                            toAppend = evaluateExpression(toAppend);
                                            newParam->left = toAppend;
                                            List_AppendElement(toReturn->children, newParam);
                                            freeListKeepData(SL);
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Update the stuff, so the while loop can continue.
                        endIdx = rightIdx;
                        idx = endIdx;
                        if (idx+1 >= tokens->length) break;
                        toCheck = (ASTNode*)List_GetElement(tokens, idx+1)->data;
                        idxChanged = true;
                    }
                    else {
                        // Same deal as function/arrays; if endIdx isn't -1, then copy toReturn, setting it as toReturn.left.
                        if (endIdx != -1) {
                            toReturnCpy = newASTNode();
                            toReturnCpy->children = toReturn->children;
                            toReturnCpy->left = toReturn->left;
                            toReturnCpy->right = toReturn->right;
                            toReturnCpy->type = toReturn->type;
                            toReturnCpy->value = toReturn->value;
                            toReturn->children = malloc(sizeof(List));
                            *toReturn->children = NewList();
                        }
                        // Pretty simple here; just set toReturn.left to toReturnCpy, and then parse the right side.
                        toReturn->type = AST_STRUCTACCESS;
                        toReturn->left = endIdx != -1 ? toReturnCpy : newIdentifierNode(token->value.nameVal);
                        toReturn->right = (ASTNode*)List_GetElement(tokens, idx+2)->data;
                        endIdx = idx+2;
                        idx = endIdx;
                        if (endIdx+1 == tokens->length) break;
                        toCheck = (ASTNode*)List_GetElement(tokens, idx+1)->data;
                        idxChanged = true;
                    }
                }
                // Finally, update nextIndex for the next parseexpression call.
                *nextIndex = endIdx + 1;
                //free(token);
                return toReturn;
            }
            }
            // If the index wasn't changed at all, then that means that this isn't a array access, function call, or struct access, so just return it as an AST_IDENTIFIER.
            if (!idxChanged) {
            node->left = NULL;
            node->right = NULL;
            node->value.nameVal = strdup(token->value.nameVal);
            *nextIndex = idx + 1;
            }
            //free(token);
            return node;

        }
        // If the current token is '[', then that means it's an array literal.
        else if (strcmp(token->value.anyVal, "[") == 0) {
            // Initializes the array literal
            int endIndex = findRightParen(*tokens, startIndex, '[', ']');
            int c = idx+1;
            ASTNode* toReturn = newASTNode();
            toReturn->type = AST_ARRAYLITERAL;
            toReturn->children = malloc(sizeof(List));
            *toReturn->children = NewList();
            // If it's an empty array, just skip over it.
            if (endIndex == startIndex+1) {
                // empty array
                *nextIndex = endIndex+1;
                return toReturn;
            }
            // Iterates through the array, keeping track of the last found comma.
            // If it finds a comma, then parses everything between the last found comma to this comma, appending it to toReturn as a child.
            for (int i = idx+1; i <= endIndex; i++) {
                ASTNode* t = (ASTNode*)List_GetElement(tokens, i)->data;
                if ( (t->type == AST_COMMA && isCommaValid(tokens, i, startIndex, endIndex, '(', ')')) || i == endIndex ) {
                    List* SL = makeSublist(*tokens, c, i-1);
                    int a;
                    ASTNode* toAppend = parseExpression(SL, 0, &a, EQ);
                    toAppend = evaluateExpression(toAppend);
                    List_AppendElement(toReturn->children, toAppend);
                    freeListKeepData(SL);
                    c = i+1;
                }
                // Multidimensional array; parse it, then jump over it.
                if (t->type == AST_BLOCKEND) {
                    if (t->value.anyVal[0] == '[') {
                        int rightParen = findRightParen(*tokens, i, '[', ']');
                        List* SL = makeSublist(*tokens, i, rightParen);
                        int a;
                        ASTNode* toAppend = parseExpression(SL, 0, &a, EQ);
                        toAppend = evaluateExpression(toAppend);
                        List_AppendElement(toReturn->children, toAppend);
                        i = rightParen+1;
                        freeListKeepData(SL);
                        c = i+1;
                    }
                }
            }
            // Update the nextIndex.
            *nextIndex = endIndex+1;
            return toReturn;
        }
        // At this point, function calls were already parsed, so parentheses simply mean just grouping for math.
        // so this parses things like (2+3)*5.
        if (strcmp(token->value.anyVal, "(") == 0) {
                int endIndex = findRightParen(*tokens, startIndex, '(', ')');
                if (endIndex <= idx) return NULL;
                List* sublist = makeSublist(*tokens, idx+1, endIndex-1);
                int subNext = 0;
                ASTNode* node = parseExpression(sublist, 0, &subNext, EQ);
                *nextIndex = endIndex+1;
                freeListKeepData(sublist);
                return node;
            
        }
    }
    fatalError(10, "Unable to parse expression", -1);
    return NULL;
}
// When given a list of tokens and an index, finds the next AST_EXPREND (semicolon).
int findNextDelimiter(List* tokens, int startIndex) {
    int toReturn = -1;
    if (startIndex > tokens->length) return -1;
    for (int i = startIndex; i < tokens->length; i++) {
        ASTNode* currentToken = (ASTNode*)List_GetElement(tokens, i)->data;
        if (currentToken->type == AST_EXPREND) {
            return i;
        }
    }
    if (toReturn == -1) return tokens->length;
    return toReturn;
}
// Checks if an operator is an assignment-operator.
bool isOpAssignment(char* n) {
    return (strcmp(n, "=") == 0)
    || (strcmp(n, "+=") == 0)
    || (strcmp(n, "++") == 0)
    || (strcmp(n, "--") == 0)
    || (strcmp(n, "-=") == 0)
    || (strcmp(n, "/=") == 0)
    || (strcmp(n, "//=") == 0)
    || (strcmp(n, "*=") == 0) 
    || (strcmp(n, "**=") == 0) 
    || (strcmp(n, "^=") == 0) 
    || (strcmp(n, "&=") == 0) 
    || (strcmp(n, "|=") == 0) 
    || (strcmp(n, "%=") == 0) 
    || (strcmp(n, ">>=") == 0)
    || (strcmp(n, "<<=") == 0);
}
// When given an assignment-operator, converts it into the normal operator (e.x += -> +).
char* getOpAssignment(char* n) {
    if (strcmp(n, "+=") == 0) return "+";
    if (strcmp(n, "-=") == 0) return "-";
    if (strcmp(n, "/=") == 0) return "/";
    if (strcmp(n, "//=") == 0) return "//";
    if (strcmp(n, "*=") == 0) return "*";
    if (strcmp(n, "**=") == 0) return "**";
    if (strcmp(n, "^=") == 0) return "^";
    if (strcmp(n, "&=") == 0) return "&";
    if (strcmp(n, "|=") == 0) return "|";
    if (strcmp(n, "%=") == 0) return "%";
    if (strcmp(n, ">>=") == 0) return ">>";
    if (strcmp(n, "<<=") == 0) return "<<";
    return NULL;
}
// Checks if a list of tokens is variable assignment.
int isTokensVarAssignment(List* tokens, scope* s) {
    int identifierIndex = -1;
    bool hasType = false;
    bool hasIdentifier = false;
    bool hasAssignment = false;
    int identifierNum = 0;
    for (int i = 0; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        if (token->type == AST_DATATYPE) hasType = true;
        if (token->type == AST_IDENTIFIER) { 
            bool isCmplxType = false;
            struct identifierExistsResult ier = identifierExists(s, token->value.nameVal);
            if (ier.type == 3) {hasType = true; isCmplxType = true; hasIdentifier = false;}

            if (!isCmplxType) hasIdentifier = true; 
        }
        if (token->type == AST_OPERATOR && isOpAssignment(token->value.opVal)) hasAssignment = true;
        
    }
    if (hasType && hasIdentifier)
        return 1;      // declaration

    if (hasAssignment)
        return 1;      // assignment

    return 0;
}
// This is the main function used for checking types of statements.
// Essentially, it just checks if a list has that type before it reaches the semicolon or a grouping symbol.
bool isTypeOfStatement(List* tokens, int type) {
    for (int i = 0; i < tokens->length; i++) {
        ASTNode* t = (ASTNode*)List_GetElement(tokens, i)->data;
        if (t->type == type) return true;
        if (t->type == AST_EXPREND || t->type == AST_BLOCKEND) break;
    }
    return false;
}
// Copies a scope.
scope* createScope(scope s) {
    scope* toReturn = malloc(sizeof(scope));

    toReturn->customTypes = NewList();
    toReturn->functions   = NewList();
    toReturn->variables   = NewList();

    // Copy custom types
    for (int i = 0; i < s.customTypes.length; i++) {
        customType* src = (customType*)List_GetElement(&s.customTypes, i)->data;
        customType* dst = malloc(sizeof(customType));
        *dst = (customType){
            .isArray = src->isArray,
        };
        memcpy(&dst->value, &src->value, sizeof(src->value));
        List_AppendElement(&toReturn->customTypes, dst);
    }

    // Copy functions
    for (int i = 0; i < s.functions.length; i++) {
        function* src =
            (function*)List_GetElement(&s.functions, i)->data;

        function* dst = malloc(sizeof(function));
        *dst = (function){
            .name = strdup(src->name)
        };
        List_AppendElement(&toReturn->functions, dst);
    }

    // Copy variables
    for (int i = 0; i < s.variables.length; i++) {
        variable* src =
            (variable*)List_GetElement(&s.variables, i)->data;

        variable* dst = malloc(sizeof(variable));
        *dst = (variable){
            .isConst = src->isConst,
            .name = strdup(src->name),
        };
        memcpy(&dst->type, &src->type, sizeof(src->type));

        List_AppendElement(&toReturn->variables, dst);
    }

    return toReturn;
}
// When given a PDT, returns if it's a number type.
bool isNumberType(int pdt) {
    return (pdt >= PDT_BOOL && pdt <= PDT_LONGDOUBLE);
}
// Converts a PDT into a number type.
int pdtToNumtype(int pdt) {
    if (isNumberType(pdt)) {
        switch (pdt) {
            case PDT_BOOL: {return NUM_BOOL;}
            case PDT_CHAR: {return NUM_CHAR;}
            case PDT_SHORT: {return NUM_SHORT;}
            case PDT_USHORT: {return NUM_USHORT;}
            case PDT_UCHAR: {return NUM_UCHAR;}
            case PDT_INT: {return NUM_INT;}
            case PDT_UINT: {return NUM_UINT;}
            case PDT_LONG: {return NUM_LONG;}
            case PDT_ULONG: {return NUM_ULONG;}
            case PDT_FLOAT: {return NUM_FLOAT;}
            case PDT_DOUBLE: {return NUM_DOUBLE;}
            case PDT_LONGDOUBLE: {return NUM_LONGDOUBLE;}
            default: {return NUM_BOOL;}
        }
    }
    else {
        fatalError(12, "Invalid input into pdtToNumType.", -1);
        return NUM_BOOL;
    }
}
/* 
This function parses anything that parseExpression() doesn't
So, this parses the following:
import statements
struct declaration
while loops
if statements
function declarations
switch statements
return statements
for loops
variable declaration

It does that by repeatedly making a sublist of tokens, checking if their a type of statement. If they are that type of statement, parse it using that statement's parsing function.
If it founds that a sublist isn't any type of statement, then call parseExpression() on it.
*/
ASTNode* parseStatements(List* tokens, ASTNode* prgmNode, scope* s) {
    int nextDelim = findNextDelimiter(tokens, 0);
    int currentIndex = 0;
    while (currentIndex < tokens->length) {
        ASTNode* cnn = List_GetElement(tokens, currentIndex)->data;
        int oldIndex = currentIndex;
        int exprNum;
        List* partialTokenList = makeSublist(*tokens, currentIndex, nextDelim-1);
        /*
        Turns:
            import "file.aur";
            import @math;
        which tokenized as:
            [AST_IMPORT() AST_STRING("file.aur") AST_EXPREND AST_IMPORT() AST_LIBRARY(@math)]
        into:
            AST_IMPORT("file.aur"):
            AST_IMPORT("@math"):
        */
        if (isTypeOfStatement(partialTokenList, AST_IMPORT)) {
            ASTNode* toAppend = newASTNode();
            *toAppend = (ASTNode){
                .children = NULL,
                .left = NULL,
                .right = NULL,
                .type = AST_IMPORT,
                .value.anyVal = NULL
            };
            ASTNode* nextNode = List_GetElement(tokens, currentIndex+1)->data;
            if (nextNode->type == AST_STRING) {
                currentIndex += 2;
                toAppend->value.nameVal = strdup(nextNode->value.anyVal);
                free(nextNode->value.anyVal);
                nextDelim = findNextDelimiter(tokens, currentIndex);
                List_AppendElement(prgmNode->children, toAppend);
                continue;
            }
            else if (nextNode->type == AST_LIBRARY) {
                currentIndex += 2;
                toAppend->value.nameVal = strdup(nextNode->value.nameVal);
                free(nextNode->value.nameVal);
                nextDelim = findNextDelimiter(tokens, currentIndex);
                List_AppendElement(prgmNode->children, toAppend);
                continue;
            }
            fatalError(12, "Import statement error", -1);
        }
        /*
        Turns 
            struct name {
                int var;
            }
        which tokenized as:
            [AST_STRUCT AST_IDENTIFIER("name") AST_BLOCKEND({) AST_DATATYPE(int) AST_IDENTIFIER(var) AST_EXPREND ...] into the following:
        into:
            AST_STRUCT("name")
                child0: AST_VARDECLARATION
                    left: int   
                    right: IDENTIFIER("var")
                    children: Null
                    .
                    . Other declarations in the struct
                    .
        */
        if (isTypeOfStatement(partialTokenList, AST_STRUCT)) {
            ASTNode* toAppend = parseStructStatement(&currentIndex, &nextDelim, tokens, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        Turns
            while (1 == 1) {
                doStuff()
            }
        which tokenized as:
            [AST_WHILE AST_BLOCKEND(() AST_NUMBER(1) AST_OPERATOR(==) AST_NUMBER(1) AST_BLOCKEND()) AST_FUNCCALL("DoStuff")] into the following:
        into:
            AST_WHILE:
                left: AST_OPERATOR(==)
                    left: AST_NUMBER(1)
                    right: AST_NUMBER(1)
                right: AST_BLOCK
                    child0: AST_FUNCCALL("DoStuff")
                    .
                    . Other stuff in the while block
                    .
        */
        else if (isTypeOfStatement(partialTokenList, AST_WHILE)) {
            ASTNode* toAppend = newASTNode();
            toAppend->type = AST_WHILE;
            toAppend->children = malloc(sizeof(List));
            *toAppend->children = NewList();
            toAppend = parseWhileStatement(&currentIndex, &nextDelim, tokens, toAppend, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        turns:
            if (1 == 2) {
                A()
            }
            elif (1 == 3) {
                B()
            }
            else {
                C()
            }
        which tokenized as:
            [AST_IF AST_BLOCKEND(() AST_NUMBER(1) AST_OPERATOR(==) AST_NUMBER(2) AST_BLOCKEND()) AST_BLOCKEND({) AST_FUNCCALL(A) AST_BLOCKEND(})
            AST_ELIF AST_BLOCKEND(() AST_NUMBER(1) AST_OPERATOR(==) AST_NUMBER(3) AST_BLOCKEND()) AST_BLOCKEND({) AST_FUNCCALL(B) AST_BLOCKEND(})
            AST_ELSE AST_BLOCKEND({) AST_FUNCCALL(C) AST_BLOCKEND(}) ]
        into:
            AST_IF
                child0: AST_CONDITIONAL:
                    left: AST_OPERATOR(==)
                        left: AST_NUMBER(1)
                        right: AST_NUMBER(2)
                    right: AST_BLOCK
                        child0: FUNCCALL(A)
                child1: AST_CONDITIONAL:
                    left: AST_OPERATOR(==)
                        left: AST_NUMBER(1)
                        right: AST_NUMBER(3)
                    right: AST_BLOCK
                        child0: FUNCCALL(B)
                child2: AST_ELSE:
                    left: AST_BLOCK:
                        FUNCCALL(C)
        */
        else if (isTypeOfStatement(partialTokenList, AST_IF)) {
            
            ASTNode* toAppend = newASTNode();
            toAppend->children = malloc(sizeof(List));
            toAppend->type = AST_IF;
            *(toAppend->children) = NewList();
            parseIfStatement(&currentIndex, &nextDelim, tokens, toAppend, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        turns:
            function n(int a) -> string {
                newStr(a);
            }
        which tokenized as:
            [ AST_FUNCTION AST_IDENTIFIER(n) AST_BLOCKEND(() AST_DATATYPE(int) AST_IDENTIFIER(a) AST_BLOCKEND()) AST_ARROW
              AST_DATATYPE(string) AST_BLOCKEND({) AST_FUNCCALL(newStr) AST_BLOCKEND(})]
        into:
            AST_FUNCDEL:
                left: IDENTIFIER(n)
                right: AST_DATATYPE(string)
                child0: AST_PARAM:
                    child0: AST_DATATYPE(int)
                    child1: AST_IDENTIFIER(n)
                child1: AST_BLOCK:
                    child0: FUNCCALL(newStr)
        */
        else if (isTypeOfStatement(partialTokenList, AST_FUNCDEL)) {
            ASTNode* toAppend = parseFunctionDeclaration(&currentIndex, &nextDelim, tokens, prgmNode, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        turns:
            switch (a) {
                case (25) {
                    b();
                }
                case (36) {
                    c();
                }
                case (a == 76) {
                    d();
                }
            }
        which tokenizes as:
            [AST_SWITCH() AST_BLOCKEND(() AST_IDENTIFIER(a) AST_BLOCKEND()) AST_BLOCKEND({)
            AST_CASE() AST_BLOCKEND(() AST_NUMBER(25) AST_BLOCKEND()) AST_BLOCKEND({) AST_FUNCCALL(b) AST_BLOCKEND(})
            AST_CASE() AST_BLOCKEND(() AST_NUMBER(36) AST_BLOCKEND()) AST_BLOCKEND({) AST_FUNCCALL(c) AST_BLOCKEND(})
            AST_CASE() AST_BLOCKEND(() AST_IDENTIFIER(a) AST_OPERATOR(==) AST_NUMBER(76) AST_BLOCKEND()) AST_BLOCKEND({) AST_FUNCCALL(d) AST_BLOCKEND(})
            AST_BLOCKEND(})
            ]
        into:
            AST_SWITCH:
                left: AST_IDENTIFIER(a)
                child0: AST_CASE
                    left: AST_NUMBER(25)
                    right: AST_BLOCK:
                        child0: FUNCCALL(b)
                child1: AST_CASE
                    left: AST_NUMBER(36)
                    right: AST_BLOCK
                        child0: FUNCCALL(c)
                child2: AST_CASE
                    left: AST_NUMBER(36)
                    right: AST_BLOCK
                        child0: FUNCCALL(d)
        */
        else if (isTypeOfStatement(partialTokenList, AST_SWITCH)) {
            ASTNode* toAppend = parseSwitchStatement(&currentIndex, &nextDelim, tokens, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        turns:
            return 5;
        which tokenized as:
            AST_RETURN AST_NUMBER(5)
        into:
            AST_RETURN
                left: AST_NUMBER(5)
        */
        else if (isTypeOfStatement(partialTokenList, AST_RETURN) ) {
            ASTNode* toAppend = newASTNode();
            toAppend->type = AST_RETURN;
            
            if (partialTokenList->length == 1 || partialTokenList->length == 0) {
                List_AppendElement(prgmNode->children, toAppend);
            }
            else {
                List* right = makeSublist(*tokens, currentIndex+1, nextDelim-1);
                
                toAppend->right = NULL;
                toAppend->left = NULL;
                int a;
                toAppend->right = parseExpression(right, 0, &a, EQ);
                List_AppendElement(prgmNode->children, toAppend);
            }
            currentIndex = nextDelim+1;
            nextDelim = findNextDelimiter(tokens, currentIndex);
        }
        /*
        turns:
            for (int i = 0; i < 20; i++) {
                a();
            }
        which tokenized as:
            [ AST_FOR AST_BLOCKEND(() AST_DATATYPE(int) AST_IDENTIFIER(i) AST_OPERATOR(=) AST_NUMBER(0) AST_EXPREND 
              AST_IDENTIFIER(i) AST_OPERATOR(<) AST_NUMBER(20) AST_EXPREND AST_IDENTIFIER(i) AST_OPERATOR(++) AST_BLOCKEND())
              AST_BLOCKEND({) AST_FUNCCALL(a) AST_BLOCKEND(})
            ]
        into:
            AST_FOR
                child0: AST_VARDECLARATION
                    left: AST_DATATYPE(int)
                    right: AST_IDENTIFIER(i)
                    child0: AST_NUMBER(0)
                child1: AST_OPERATOR
                    left: AST_IDENTIFIER(i)
                    right: AST_NUMBER(20)
                child2: AST_VARASSIGNMENT
                    right: AST_IDENTIFIER(i)
                    child0: AST_OPERATOR(+)
                        AST_IDENTIFIER(i)
                        AST_NUMBER(1)
                child3: AST_BLOCK
                    child0: FUNCCALL(a)
        */
        else if (isTypeOfStatement(partialTokenList, AST_FOR)) {
            ASTNode* toAppend = newASTNode();
            toAppend->type = AST_FOR;
            toAppend->children = malloc(sizeof(List));
            *toAppend->children = NewList();
            parseForStatement(&currentIndex, &nextDelim, tokens, toAppend, s);
            List_AppendElement(prgmNode->children, toAppend);
        }
        /*
        Turns:
            int a = 5i;
            string b = "a";
            structName c = structName()
            c.a = 2;
            a = 59;
            b[0] = 'a';
            const int[] n = [2,4,6];
        which tokenized as:
            [AST_DATATYPE(int) AST_IDENTIFIER(a) AST_OPERATOR(=) AST_NUMBER(5) AST_EXPREND
            AST_DATATYPE(string) AST_IDENTIFIER(b) AST_OPERATOR(=) AST_STRING("a") AST_EXPREND
            AST_IDENTIFIER("structName") AST_IDENTIFIER(c) AST_OPERATOR(=) AST_FUNCCALL(structName) AST_EXPREND
            AST_IDENTIFIER(c) AST_DOT AST_IDENTIFIER(a) AST_OPERATOR(=) AST_NUMBER(2) AST_EXPREND
            AST_IDENTIFIER(a) AST_OPERATOR(=) AST_NUMBER(59) AST_EXPREND
            AST_IDENTIFIER(b) AST_BLOCKEND([) AST_NUMBER(0) AST_BLOCKEND(]) AST_OPERATOR(=) AST_NUMBER(65)
            AST_DATATYPE(int) AST_BLOCKEND([) AST_BLOCKEND(]) AST_IDENTIFIER(n) AST_OPERATOR(=) AST_BLOCKEND([) AST_NUMBER(2), AST_COMMA, AST_NUMBER(4), AST_COMMA AST_NUMBER(6) AST_BLOCKEND(])
            ]
        into:
            AST_VARDECLARATION:
                left: DATATYPE(int)
                right: IDENTIFIER(a)
                child0: AST_NUMBER(5)
            AST_VARDECLARATION:
                left: DATATYPE(string)
                right: IDENTIFIER(b)
                child0: AST_STRING("a")
            AST_VARDECLARATION:
                left: COMPLEXDATATYPE(structName)
                right: IDENTIFIER(c)
                child0: AST_FUNCCALL(structName)
            AST_VARASSIGNMENT:
                right: AST_STRUCTACCESS:
                    left: IDENTIFIER(c)
                    right: IDENTIFIER(a)
                child0: AST_NUMBER(2)
            AST_VARASSIGNMENT:
                right: AST_IDENTIFIER(a)
                child0: AST_NUMBER(59)
            AST_VARASSIGNMENT:
                right: AST_ARRAYACCESS
                    left: IDENTIFIER(b)
                    right: NUMBER(0)
                child0: AST_NUMBER(65)
            AST_VARDECLARATION:
                left: AST_DATATYPE(int)
                    left: AST_ARRAYACCESS // this signals that this is an array declaration
                    child0: AST_IDENTIFIERSPECIFIER(const) // All id-specifiers go in as children of the datatype
                right: AST_IDENTIFIER(n)
                child0: AST_ARRAYLITERAL
                    child0: NUMBER(2)
                    child1: NUMBER(4)
                    child2: NUMBER(6)
        */
        else if (isTokensVarAssignment(partialTokenList, s) != 0) {
            
            ASTNode* toAppend = parseAssignment(&currentIndex, &nextDelim, tokens, s);
            List_AppendElement(prgmNode->children, toAppend);
        }    
        // Otherwise, parse the expression & append it to the program node.
        else {
            
            ASTNode* toAppend = parseExpression(partialTokenList, 0, &exprNum, EQ);
            ASTNode* evaluatedExpr = evaluateExpression(toAppend);
            freeAST(toAppend);
            if (evaluatedExpr != NULL) List_AppendElement(prgmNode->children, evaluatedExpr);
            currentIndex = nextDelim+1;
            nextDelim = findNextDelimiter(tokens, currentIndex);
            
        }
        // If the currentIdnex hasn't changed, then that's bad
        if (currentIndex <= oldIndex) {
            fatalError(0xE, "Parser stalled", -1);
        }
        freeListKeepData(partialTokenList);
    }
    return prgmNode;
}
// Parses struct statements (see struct section in parseStatements())
ASTNode* parseStructStatement(int* CI, int* nextDelim, List* tokens, scope* s) {

    // Initialize toReturn, grab the name, and get openBraceIndex.
    ASTNode* toReturn = newASTNode();
    toReturn->type = AST_STRUCT;
    int openBraceIndex = -1;
    for (int i = *CI; i < *nextDelim; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        if (token->type == AST_IDENTIFIER) {
            toReturn->value.nameVal = token->value.nameVal;
        }
        if (token->type == AST_BLOCKEND) {
            openBraceIndex = i;
            break;
        }
    }
    // Find close brace index, add the new struct into the s cope, and parse everything from openBrace to closeBrace.
    // In addition, also creates a new scope.
    int closeBraceIndex = findRightParen(*tokens, openBraceIndex, '{', '}');
    customType* ct = malloc(sizeof(customType));
    *ct = (customType){
        .isArray = false,
        .value.name = toReturn->value.nameVal
    };
    List_AppendElement(&s->customTypes, ct);
    
    List* SL = makeSublist(*tokens, openBraceIndex+1, closeBraceIndex-1);
    toReturn->children = malloc(sizeof(List));
    *toReturn->children = NewList();
    scope* ns = createScope(*s); // previously fixed free(ns).
    parseStatements(SL, toReturn, ns);
    List_Free_List(&ns->customTypes);
    List_Free_List(&ns->functions);
    List_Free_List(&ns->variables);
    free(ns);
    *CI = closeBraceIndex+1;
    *nextDelim = findNextDelimiter(tokens, *CI);
    freeListKeepData(SL);
    return toReturn;


}
// Parses switch statements (see switch section in parseStatements())
ASTNode* parseSwitchStatement(int* CI, int* nextDelim, List* tokens, scope* s) {
    
    // In the first line of a switch statement, grabs the open/closed parentheses, and the open brace
    // switch (n) { stuff }
    // -------^-^-^
    int openParenIndex = -1;
    int closeParenIndex = -1;
    int openBraceIndex = -1;
    for (int i = *CI; i < tokens->length; i++) {
        ASTNode* t = (ASTNode*)List_GetElement(tokens, i)->data;
        if (t->type == AST_BLOCKEND) {
            if (t->value.anyVal[0] == '(') openParenIndex = i;
            if (t->value.anyVal[0] == ')') closeParenIndex = i;
            if (t->value.anyVal[0] == '{') openBraceIndex = i;
        }
        if (openParenIndex != -1 && closeParenIndex != -1 && openBraceIndex != -1) break;
    }
    // Using the open brace, finds the close brace.
    int closeBraceIndex = findRightParen(*tokens, openParenIndex, '{', '}');

    // Initializes the toreturn, setting .left to the condition.
    ASTNode* toReturn = newASTNode();
    toReturn->children = malloc(sizeof(List));
    *toReturn->children = NewList();
    toReturn->type = AST_SWITCH;
    List* toParse = makeSublist(*tokens, openParenIndex+1, closeParenIndex-1);
    int a;
    ASTNode* leftA = parseExpression(toParse, 0, &a, EQ);
    toReturn->left = evaluateExpression(leftA);
    freeAST(leftA);
    freeListKeepData(toParse);
    // Loops from openBraceIndex to closeBraceIndex.
    // If it encounters a case statement, grabs the parentheses, adding it as a AST_CASE.left, and makes a block, adding it as a AST_CASE.right.
    // Then it parses everything from the case's open brace to its close brace.
    // Finally, it appends that AST_CASE to toReturn, and updates c (the current index).
    int c = openBraceIndex+1;
    while (c < closeBraceIndex) {
        
        ASTNode* tok = (ASTNode*)List_GetElement(tokens, c)->data;
        if (tok->type != AST_CASE) break;
        ASTNode* toAppend = newASTNode();
        toAppend->type = AST_CASE;
        ASTNode* toCheck = (ASTNode*)List_GetElement(tokens, c+1)->data;
        
        int obi = c+1;
        if (toCheck->value.anyVal[0] == '(') {
            int parenA = c+1;
            int parenB = findRightParen(*tokens, parenA, '(', ')');
            List* SL = makeSublist(*tokens, parenA+1, parenB-1);
            int b;
            ASTNode* tol = parseExpression(SL, 0, &b, EQ);
            toAppend->left = evaluateExpression(tol);
            freeAST(tol);
            obi = parenB+1;
        }
        int cbi = findRightParen(*tokens, obi, '{', '}');
        List* SL = makeSublist(*tokens, obi+1, cbi-1);
        toAppend->right = newASTNode();
        toAppend->right->type = AST_BLOCK;
        toAppend->right->children = malloc(sizeof(List));
        *toAppend->right->children = NewList();
        scope* ns = createScope(*s);
        parseStatements(SL, toAppend->right, ns);
        freeListKeepData(SL);
        List_Free_List(&ns->customTypes);
        List_Free_List(&ns->functions);
        List_Free_List(&ns->variables);
        free(ns);
        List_AppendElement(toReturn->children, toAppend);
        c = cbi+1;
    }
    *CI = 1+closeBraceIndex;
    *nextDelim = findNextDelimiter(tokens, *CI);
    return toReturn;
}
// Finds an identifier (struct, function name, or variable name) within a scope.
// if it doesn't exist, returns (identifierExistsResult){.exists = false, .dest = NULL, .type = 0}.
// Otherwise, if it's a variable, .type = 1. If it's a function, .type = 2, and if it's a struct, .type = 3.

struct identifierExistsResult identifierExists(scope* s, char* name) {
    struct identifierExistsResult toReturn = (struct identifierExistsResult){.dest = NULL, .exists = false, .type = 0};
    int i = 0;
    // Override for system struct declarations
    while (strlen(systemStructDecl[i]) != 0) {
        char* current = systemStructDecl[i];
        if (strcmp(systemStructDecl[i], name) == 0) {
            toReturn.dest = NULL; toReturn.exists = true, toReturn.type = 3;
            return toReturn;
        }
        i++;
    }
    // Searches through the scope to find the identifier.
    for (int i = 0; i < s->functions.length; i++) {
        function* f = (function*)List_GetElement(&s->functions, i)->data;
        if (strcmp(f->name, name) == 0) {
            toReturn.dest = f; toReturn.exists = true; toReturn.type = 2;
            return toReturn; // here's the error
        }
    }
    for (int i = 0; i < s->variables.length; i++) {
        variable* v = (variable*)List_GetElement(&s->variables, i)->data;
        if (strcmp(v->name, name) == 0) {
            toReturn.dest = v; toReturn.exists = true; toReturn.type = 1;
            return toReturn;
        }
    }
    for (int i = 0; i < s->customTypes.length; i++) {
        customType* ct = (customType*)List_GetElement(&s->customTypes, i)->data;
        if (strcmp(ct->value.name, name) == 0) {
            toReturn.dest = ct; toReturn.exists = true; toReturn.type = 3;
            return toReturn;
        }
    }
    return toReturn;
}
// Parses assignments (see assignment section in parseStatements())
ASTNode* parseAssignment(int* CI, int* nextDelim, List* tokens, scope* s) {
    // Initializes toReturn & other variables.
    ASTNode* toReturn = newASTNode();;
    toReturn->type = AST_ASSIGNMENT;
    int eqIndex = -1;
    int delimIndex = -1;
    customType* type = malloc(sizeof(customType));
    *type = (customType){.isArray = false, .value.t = PDT_NULL};
    bool typeAssigned = false;
    char* name = "";
    bool varExists = false;
    bool complexType = false;
    int typeIndex = -1;
    int varIndex = -1;
    bool isArray = false;
    List* specifiers = malloc(sizeof(List)); *specifiers = NewList(); // List of specifiers (such as const)
    // Iterates through the tokens given, assigning stuff to the above variables.
    for (int i = *CI; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        // If the token is a semicolon, break.
        if (token->type == AST_EXPREND) {
            delimIndex = i;
            break;
        }
        // If it's AST_IDENTIFIERSPECIFIER, then add it to the list of specifiers.
        if (token->type == AST_IDENTIFIERSPECIFIER) {
            List_AppendElement(specifiers, token);
        }
        // If it's an assignment operator, set eqIndex to that.
        if (token->type == AST_OPERATOR) {
            if (isOpAssignment(token->value.opVal)) {
                eqIndex = i;
            }
        }
        // If it's a AST_DATATYPE, set typeIndex = i, the type.value to the token's value, typeAssigned to true, & complexType = false.
        if (token->type == AST_DATATYPE) {typeIndex = i; type->value.t = token->value.numberVal; typeAssigned = true; complexType = false;}
        // If both the equals sign & variable name haven't been encountered, then this is an array.
        if (token->type == AST_BLOCKEND && eqIndex == -1 && varIndex == -1) {
            if (token->value.anyVal[0] == '[') isArray = true;
        }
        // If the identifier already exists, then it's an assignment.
        // If there's two identifiers right next to eachother, then it's a complex declaration
        // If there's one identifier, then it's a normal declaration.
        if (token->type == AST_IDENTIFIER) {
            if (eqIndex != -1) continue;
            struct identifierExistsResult ier = identifierExists(s, token->value.nameVal);
            if (ier.type == 1) {
                if (i > varIndex && varIndex != -1) continue;
                varIndex = i; name = token->value.nameVal; varExists = true;
            }
            else if (ier.type == 3 && !typeAssigned) {type->value.name = token->value.nameVal; typeIndex = i; complexType = true;} 
            else if (typeIndex != -1 || !varExists && eqIndex == -1) {
                if (i > varIndex && varIndex != -1) {
                    varIndex = typeIndex == -1 ? i-1 : typeIndex;
                    typeIndex = i;
                    type->value.name = name;
                    name = token->value.nameVal;
                    complexType = true;
                    typeAssigned = true;
                    continue;
                };
                name = token->value.nameVal; varIndex = i;
            }
            
        }
    }
    
    // Initialize variable, for storage in s.
    variable v;
    if (!varExists) {
        v.name = name;
        v.type = *type;
    }
    bool blockInvalid = eqIndex == -1;
    // get the eq node. If there's no assignment operator, then set it to a default value.
    ASTNode* eq = (ASTNode*)List_GetElement(tokens, eqIndex)->data;
    if (eqIndex == -1) eq = NULL;
    if (eq == NULL) {
        toReturn->children = malloc(sizeof(List));
        *toReturn->children = NewList();
        ASTNode* toAppend = newASTNode();
        toAppend->type = AST_NUMBER;
        ASTNode* typeNode = (ASTNode*)List_GetElement(tokens, typeIndex)->data;
        if (typeNode->type == AST_DATATYPE && !isArray) {
            if (isNumberType(typeNode->value.numberVal) ) {
                toAppend->type = AST_NUMBER;
                toAppend->value.numVal = (num){.type = pdtToNumtype(typeNode->value.numberVal), .value.bVal = 0};
            }
            else if (typeNode->value.numberVal == PDT_STRING) {
                toAppend->type = AST_STRING;
                toAppend->value.anyVal = strdup("");
            }
            List_AppendElement(toReturn->children, toAppend);
        }
        else if (typeNode->type == AST_COMPLEXDATATYPE) {
            free(toAppend);
        }
    }
    // If the eq operator is not ++ or --, then it's a normal assignment:
    else if (!blockInvalid && strcmp(eq->value.opVal, "++") != 0 && strcmp(eq->value.opVal, "--") != 0) {
        // initialize toReturn.children, & get the value sublist.
        toReturn->children = malloc(sizeof(List));
        *toReturn->children = NewList();
        List* block = makeSublist(*tokens, eqIndex+1, delimIndex-1);
        int a;
        ASTNode* toAppend = parseExpression(block, 0, &a, EQ);
        toAppend = evaluateExpression(toAppend);
        ASTNode* ta = toAppend;
        // if the eq isn't "=", then insert the non-assignment operator, and the variable onto the eqSide.
        // e.x t += 2 -> t = t + 2.
        if (strcmp(eq->value.opVal, "=") != 0) {
            ASTNode* opA = newASTNode();
            opA->type = AST_OPERATOR;

            List* SL = makeSublist(*tokens, varIndex, eqIndex-1);
            int a;
            ASTNode* n = parseExpression(SL, 0, &a, EQ);
            n = evaluateExpression(n);
            opA->value.opVal = malloc(strlen(eq->value.opVal));
            if (strcmp(eq->value.opVal, "+=") == 0) strcpy(opA->value.opVal, "+");
            if (strcmp(eq->value.opVal, "-=") == 0) strcpy(opA->value.opVal, "-");
            if (strcmp(eq->value.opVal, "*=") == 0) strcpy(opA->value.opVal, "*");
            if (strcmp(eq->value.opVal, "/=") == 0) strcpy(opA->value.opVal, "/");
            if (strcmp(eq->value.opVal, "//=") == 0) strcpy(opA->value.opVal, "//");
            if (strcmp(eq->value.opVal, "**=") == 0) strcpy(opA->value.opVal, "**");
            if (strcmp(eq->value.opVal, "^=") == 0) strcpy(opA->value.opVal, "^");
            if (strcmp(eq->value.opVal, "&=") == 0) strcpy(opA->value.opVal, "&");
            if (strcmp(eq->value.opVal, "|=") == 0) strcpy(opA->value.opVal, "|");
            if (strcmp(eq->value.opVal, "%=") == 0) strcpy(opA->value.opVal, "%");
            if (strcmp(eq->value.opVal, ">>=") == 0) strcpy(opA->value.opVal, ">>");
            if (strcmp(eq->value.opVal, "<<=") == 0) strcpy(opA->value.opVal, "<<");

            opA->left = n;
            opA->right = toAppend;
            ta = opA;
        }
        List_AppendElement(toReturn->children, ta);
    }
    // If the eq operator is ++ or --, then insert + or -, and 1.
    // e.x t-- -> t = t - 1;
    else if (strcmp(eq->value.opVal, "++") == 0 || strcmp(eq->value.opVal, "--") == 0) {
        toReturn->children = malloc(sizeof(List));
        *toReturn->children = NewList();
        ASTNode* toAppend = newASTNode();
        toAppend->type = AST_OPERATOR;
        toAppend->value.opVal = malloc(2);
        toAppend->value.opVal[1] = '\0';
        toAppend->value.opVal[0] = eq->value.opVal[0];
        toAppend->children = NULL;
        List* SL = makeSublist(*tokens, *CI, delimIndex-1);
        int a;
        ASTNode* n = parseExpression(SL, 0, &a, EQ);
        n = evaluateExpression(n);
        freeListKeepData(SL);
        toAppend->left = n;
        toAppend->right = newNumNode((num){.type = NUM_SHORT, .value.sVal = 1});
        List_AppendElement(toReturn->children, toAppend);
    }
    
    // Check if it's an array declaration.
    ASTNode* toCheck1 = (ASTNode*)List_GetElement(tokens, typeIndex + 1)->data;
    ASTNode* toCheck2 = (ASTNode*)List_GetElement(tokens, typeIndex + 2)->data;
    if (toCheck1->type == AST_BLOCKEND && toCheck2->type == AST_BLOCKEND && !varExists) {
        v.type.isArray = true;
        type->isArray = true;
        typeIndex += 2;
    } 
    
    // Finally, finish making toReturn.
    // .left is the type, so if it's a complex type, then its value is the complex type's name. Otherwise, it's the type's number.
    toReturn->left = newASTNode();
    *toReturn->left = (ASTNode){
        .children = NULL,
        .value.numberVal = 0.0,
        .type = complexType? AST_COMPLEXDATATYPE : AST_DATATYPE,
        .left = NULL,
        .right = NULL
    };
    if (complexType) toReturn->left->value.nameVal = v.type.value.name;
    if (!complexType) toReturn->left->value.numberVal = v.type.value.t;
    // If it's an array, append AST_ARRAYASSIGNMENT onto toReturn.left.
    if (type->isArray) {
        ASTNode* arrayChild = newASTNode();
        arrayChild->type = AST_ARRAYASSIGNMENT;
        toReturn->left->left = arrayChild;
    }
    // If there's no specifiers, free it. Otherwise, set toReturn.left.children to specifiers.
    if (specifiers->length == 0) {free(specifiers); specifiers = NULL;}
    if (specifiers != NULL) {
        toReturn->left->children = specifiers;
    }
    // Parse a sublist from [eqIndex, delim]
    eqIndex = eqIndex != -1 ? eqIndex : delimIndex;
    if (eqIndex < varIndex) eqIndex = varIndex+1;
    int a;
    List* SL = makeSublist(*tokens, varIndex, eqIndex-1);
    if (varExists || (eqIndex != varIndex+1 && varIndex+1 != typeIndex) ) {
        toReturn->right = parseExpression(SL, 0, &a, EQ);
        toReturn->right = evaluateExpression(toReturn->right);
    }
    else {
        toReturn->right = newASTNode();
        *toReturn->right = (ASTNode){
            .children = NULL,
            .type = AST_IDENTIFIER,
            .value.nameVal = name,
        };
    }
    // If the variable doesn't exist, then add it to the scope's variables.
    if (!varExists && toReturn->right->type == AST_IDENTIFIER) {
        toReturn->type = AST_VARDECLARATION;
        variable* tAp = malloc(sizeof(variable));
        tAp->name = malloc(strlen(name)+1);
        tAp->name[0] = '\0';
        strcpy(tAp->name, name);
        tAp->type = *type;
        List_AppendElement(&s->variables, tAp);
    }
    // Otherwise, set toReturn.left to 0.
    else {
        free(toReturn->left);
        toReturn->left = newNumNode((num){.type = NUM_BOOL, .value.bVal = false});
    }
    // Update the index
    *CI = *nextDelim+1;
    *nextDelim = findNextDelimiter(tokens, *CI);
    return toReturn;
}
// Parses function declarations (see function declaration section in parseStatements())
ASTNode* parseFunctionDeclaration(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s) {

    // Initialize some stuff
    ASTNode* toAppend = newASTNode();
    int currentIndex = *CI;
    toAppend->type = AST_FUNCDEL;
    toAppend->children = malloc(sizeof(List));
    *toAppend->children = NewList();
    int openBraceIndex = -1;
    int openParenIndex = -1;
    int arrowIndex = -1;
    ASTNode* returnTypeNode = NULL;
    List identifierIndices = NewList();
    // Iterate through the given tokens
    for (int i = currentIndex; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        // Grab the 1st open parentheses & 1st open brace
        if (token->type == AST_BLOCKEND) {
            if ( strcmp(token->value.anyVal, "(") == 0 && openParenIndex == -1) {openParenIndex = i;}
            if ( strcmp(token->value.anyVal, "{") == 0 && openBraceIndex == -1) {openBraceIndex = i;}
        }
        // if a open parentheses haven't been found, then it's the function name.
        if (token->type == AST_IDENTIFIER && openParenIndex != -1) {
            int* j = malloc(sizeof(int));
            *j = i;
            bool cdtContains = identifierExists(s, token->value.nameVal).type == 3;
            if (!cdtContains) List_AppendElement(&identifierIndices, j);
            else free(j);

        }
        // Grab the arrow index.
        if (token->type == AST_ARROW) {
            arrowIndex = i;
        }
        // If it's a datatype & the arrow index has been found, then it's the return type.
        if (token->type == AST_DATATYPE && arrowIndex != -1) {
            returnTypeNode = newASTNode();
            *returnTypeNode = (ASTNode){
                .children = NULL,
                .left = NULL,
                .right = NULL,
                .type = AST_DATATYPE,
                .value.numberVal = token->value.numberVal
            };
        }
        // If the token is AST_VOID, then that means the return type is void.
        if (token->type == AST_VOID) {
            returnTypeNode = newASTNode();
            *returnTypeNode = (ASTNode){
                .children = NULL,
                .left = NULL,
                .right = NULL,
                .type = AST_VOID,
                .value.numberVal = PDT_VOID
            };
        }
        // If an identifier has been found after it, then it's a complex type.
        if (token->type == AST_IDENTIFIER && arrowIndex != -1) {
            // complex datatype
            returnTypeNode = token;
            returnTypeNode->type = AST_COMPLEXDATATYPE;
            
        }
        // If a bracket was found after the arrow index, then that means it's returning an array.
        if (token->type == AST_BLOCKEND && arrowIndex != -1) {
            if (strcmp(token->value.anyVal,"[") == 0) {
                ASTNode* arrayChild = newASTNode();
                arrayChild->type = AST_ARRAYASSIGNMENT;
                returnTypeNode->left = arrayChild;
            }
        }
        if (openBraceIndex != -1 && openParenIndex != -1 && arrowIndex != -1) break;
    }
    if (returnTypeNode == NULL) {fatalError(0xF, "Function return type not resolved.", -1);}
    // Get the close brace & close paren
    int closeBraceIndex = findRightParen(*tokens, openBraceIndex, '{', '}');
    int closeParenIndex = findRightParen(*tokens, openParenIndex, '(', ')');

    // Get the function name
    ASTNode identifierName = *(ASTNode*)List_GetElement(tokens, openParenIndex-1)->data;
    ASTNode* funcNameNode = newIdentifierNode(identifierName.value.nameVal);
    
    toAppend->left = funcNameNode;
    // Search through the identifiers, seeing if the function exists already.
    for (int i = 0; i < s->functions.length; i++) {
        function* fc = (function*)List_GetElement(&s->functions, i)->data;
        if (strcmp(fc->name, funcNameNode->value.nameVal) == 0) fatalError(0x10, "Cannot have two identifiers within the same scope with the same name.", -1);
    }
    for (int i = 0; i < s->variables.length; i++) {
        variable* var = (variable*)List_GetElement(&s->variables, i)->data;
        if (strcmp(var->name, funcNameNode->value.nameVal) == 0) fatalError(0x10, "Cannot have two identifiers within the same scope with the same name.", -1);
    }
    // Initialize the function to add to the scope.
    function* fcAppend = malloc(sizeof(function));
    fcAppend->name = malloc(strlen(funcNameNode->value.nameVal)+1);
    fcAppend->name[0] = '\0';
    strcpy(fcAppend->name, funcNameNode->value.nameVal);
    List_AppendElement(&s->functions, fcAppend);

    // Set the return type, & look through identifierIndices for parameters.
    // Appends any parameters it finds as AST_PARAM to the function's children.
    toAppend->right = returnTypeNode;
    int c = openParenIndex+1;
    for (int i = 0; i < identifierIndices.length; i++) {
        
        int index = *(int*)List_GetElement(&identifierIndices, i)->data;
        ASTNode* param = newASTNode();
        param->children = malloc(sizeof(List));
        *param->children = NewList();
        param->type = AST_PARAM;
        
        for (int j = c; j <= index; j++) {
            ASTNode* token = (ASTNode*)List_GetElement(tokens, j)->data;
            
            ASTNode* paramToAppend = newASTNode();
            paramToAppend->type = token->type;
            if (token->type == AST_DATATYPE) {
                paramToAppend->value.numberVal = token->value.numberVal;
                if (j + 1 <= index-1) {
                    ASTNode* toCheck = (ASTNode*)List_GetElement(tokens, j+1)->data;
                    if (toCheck->type == AST_BLOCKEND) {
                        if (toCheck->value.anyVal[0] == '[') {
                            ASTNode* arraySpec = newASTNode();
                            arraySpec->type = AST_ARRAYACCESS;
                            paramToAppend->left = arraySpec;
                            j += 2;
                            
                        }
                    }
                }
            }
            else if (token->type == AST_IDENTIFIER) {
                bool isComplexDatatype = false;
                for (int i = 0; i < s->customTypes.length; i++) {
                    customType* ct = (customType*)List_GetElement(&s->customTypes, i)->data;
                    if (strcmp(ct->value.name, token->value.nameVal) == 0) {
                        paramToAppend->value.nameVal = ct->value.name;
                        paramToAppend->type = AST_COMPLEXDATATYPE;
                        if (j + 1 <= index-1) {
                            ASTNode* toCheck = (ASTNode*)List_GetElement(tokens, j+1)->data;
                            if (toCheck->type == AST_BLOCKEND) {
                                if (toCheck->value.anyVal[0] == '[') {
                                    ASTNode* arraySpec = newASTNode();
                                    arraySpec->type = AST_ARRAYACCESS;
                                    paramToAppend->left = arraySpec;
                                    j += 2;
                                }
                            }
                        }
                        isComplexDatatype = true;
                        break;
                    }

                }
                if (!isComplexDatatype) {
                    paramToAppend->value.nameVal = token->value.nameVal;
                }
            }
            List_AppendElement(param->children, paramToAppend);
            
        }
        c = index+1;
        if (((ASTNode*)List_GetElement(tokens, c)->data)->type == AST_COMMA) c++;
        List_AppendElement(toAppend->children, param);
    }
    // Gets the function block, and adds it as anotehr child to toAppend.
    ASTNode* block = newASTNode();
    block->children = malloc(sizeof(List));
    block->type = AST_BLOCK;
    *block->children = NewList();
    List* sublist = makeSublist(*tokens, openBraceIndex+1, closeBraceIndex-1);
    scope* ns = createScope(*s);
    parseStatements(sublist, block, ns);
    List_Free_List(&ns->customTypes);
    List_Free_List(&ns->functions);
    List_Free_List(&ns->variables);
    free(ns);
    
    List_AppendElement(toAppend->children, block);
    *CI = closeBraceIndex + 1;
    *nextDelim = findNextDelimiter(tokens, *CI);
    
    return toAppend;
}
// Parses while statements (see while loops section in parseStatements())
ASTNode* parseWhileStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s) {
    // Initialize stuff, and grab the openBraceIndex & openParenIndex.
    ASTNode* toAppend = newASTNode();
    int currentIndex = *CI;
    toAppend->type = AST_WHILE;
    int openBraceIndex = -1;
    int openParenIndex = -1;
    for (int i = currentIndex; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        if (token->type == AST_BLOCKEND) {
        if ( strcmp(token->value.anyVal, "(") == 0 && openParenIndex == -1) {openParenIndex = i;}
        if ( strcmp(token->value.anyVal, "{") == 0 && openBraceIndex == -1) {openBraceIndex = i;}
        if (openBraceIndex != -1 && openParenIndex != -1) break;
        }
    }
    // get closeBrace & closeParen.
    int closeBraceIndex = findRightParen(*tokens, openBraceIndex, '{', '}');
    int closeParenIndex = findRightParen(*tokens, openParenIndex, '(', ')');
    int a;
    // The condition is the sublist from openparenIndex+1 to closeParenIndex-1.
    List* condNode = makeSublist(*tokens, openParenIndex+1, closeParenIndex-1);
    a = 0;
    // Sets toAppend.left to the parsed condition.
    toAppend->left = parseExpression(condNode, 0, &a, EQ);
    // Sets toAppend.right to an AST_BLOCK, and parses the while block.
    toAppend->right = newASTNode();
    toAppend->right->type = AST_BLOCK;
    toAppend->right->children = malloc(sizeof(List));
    toAppend->right->left = NULL;
    toAppend->right->right = NULL;
    *(toAppend->right->children) = NewList();
    scope* ns = createScope(*s);
    List* SL = makeSublist(*tokens, openBraceIndex+1, closeBraceIndex-1);
    parseStatements(SL, toAppend->right, ns);
    List_Free_List(&ns->customTypes);
    List_Free_List(&ns->functions);
    List_Free_List(&ns->variables);
    free(ns);
    *CI = currentIndex+(closeBraceIndex-openParenIndex)+2;
    *nextDelim = findNextDelimiter(tokens, *CI);
    return toAppend;
}
// Parses for loops (see for loop section in parseStatements())
ASTNode* parseForStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s) {
    // Initializes stuff
    int openBraceIndex = -1;
    int openParenIndex = -1;
    int firstColon = -1;
    int secondColon = -1;
    int currentIndex = *CI;
    ASTNode* toReturn = newASTNode();
    toReturn->children = malloc(sizeof(List));
    *toReturn->children = NewList();
    toReturn->type = AST_FOR;
    // Iterates through the tokens given, finding openparen, openBrace, and 1st & 2nd colons.
    for (int i = currentIndex; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        if (token->type == AST_BLOCKEND) {
            if (strcmp(token->value.anyVal, "(") == 0 && openParenIndex == -1) {openParenIndex = i;}
            if (strcmp(token->value.anyVal, "{") == 0 && openBraceIndex == -1) {openBraceIndex = i;}
        }
        else if (token->type == AST_EXPREND) {
            if (firstColon == -1) {firstColon = i; continue;}
            if (secondColon == -1 && firstColon != -1) secondColon = i;
        }
        
        if (openBraceIndex != -1 && openParenIndex != -1 && firstColon != -1 && secondColon != -1) break;
    }
    // Finds the closeBrace & closeParen indices.
    int closeBraceIndex = findRightParen(*tokens, openBraceIndex, '{', '}');
    int closeParenIndex = findRightParen(*tokens, openParenIndex, '(', ')');
    // openParen - first semicolon = initializer;
    // first semicolon - 2nd semicolon = for condition
    // 2nd semicolon - closeParenIndex = update
    // openBrace - closeBrace = block
    List* initializerTokens = makeSublist(*tokens, openParenIndex+1, firstColon);
    List* conditionTokens = makeSublist(*tokens, firstColon+1, secondColon);
    List* updateTokens = makeSublist(*tokens, secondColon+1, closeParenIndex-1);
    // Appends a semicolon to updateTokens, so that parseStatements() parses it.
    ASTNode* semiappend = newASTNode();
    semiappend->type = AST_EXPREND;
    List_AppendElement(updateTokens, semiappend);
    List* blockTokens = makeSublist(*tokens, openBraceIndex+1, closeBraceIndex-1);
    // Parses initializer, condition, update, & block, appending them to the for loop.
    ASTNode* initializer = parseStatements(initializerTokens, prgmNode, s);
    
    ASTNode* condition = parseStatements(conditionTokens, prgmNode, s);
    ASTNode* update = parseStatements(updateTokens, prgmNode, s);
    ASTNode* block = newASTNode();
    // Finally, parses the actual loop.
    block->type = AST_BLOCK;
    block->children = malloc(sizeof(List));
    *(block->children) = NewList();
    scope* ns = createScope(*s);
    parseStatements(blockTokens, block, ns);
    List_Free_List(&ns->customTypes);
    List_Free_List(&ns->functions);
    List_Free_List(&ns->variables);
    free(ns);
    List_AppendElement(prgmNode->children, block);
    *CI = closeBraceIndex+1;
    *nextDelim = findNextDelimiter(tokens, *CI);
    return toReturn;

}
// Parses if statements (see if statement section in parseStatements())
ASTNode* parseIfStatement(int* CI, int* nextDelim, List* tokens, ASTNode* prgmNode, scope* s) {
    // Initializes stuff
    ASTNode* toAppend = newASTNode();
    int currentIndex = *CI;
    toAppend->type = AST_CONDITIONAL;
    int openBraceIndex = -1;
    int openParenIndex = -1;
    // Finds openParen & openBrace.
    for (int i = currentIndex; i < tokens->length; i++) {
        ASTNode* token = (ASTNode*)List_GetElement(tokens, i)->data;
        if (token->type == AST_BLOCKEND) {
            if ( strcmp(token->value.anyVal, "(") == 0 && openParenIndex == -1) {openParenIndex = i;}
            if ( strcmp(token->value.anyVal, "{") == 0 && openBraceIndex == -1) {openBraceIndex = i;}
            if (openBraceIndex != -1 && openParenIndex != -1) break;
        }
    }
    // Finds closeBrace & closeParen.
    int closeBraceIndex = findRightParen(*tokens, openBraceIndex, '{', '}');
    int closeParenIndex = findRightParen(*tokens, openParenIndex, '(', ')');
    // Parses the condition, setting toAppend.left to it.
    int a;
    List* condNode = makeSublist(*tokens, openParenIndex+1, closeParenIndex-1);
    a = 0;
    toAppend->left = parseExpression(condNode, 0, &a, EQ);
    // Sets toAppend.right to the 1st block of it.
    toAppend->right = newASTNode();
    toAppend->right->type = AST_BLOCK;
    toAppend->right->children = malloc(sizeof(List));
    toAppend->right->left = NULL;
    toAppend->right->right = NULL;
    *(toAppend->right->children) = NewList();
    List* SL = makeSublist(*tokens, openBraceIndex+1, closeBraceIndex-1);
    // Creates a new scope, and parses the block.
    scope* ns = createScope(*s);
    parseStatements(SL, toAppend->right, ns);
    List_Free_List(&ns->customTypes);
    List_Free_List(&ns->functions);
    List_Free_List(&ns->variables);
    free(ns);
    List_AppendElement(prgmNode->children, toAppend);
    *CI = closeBraceIndex+1;
    *nextDelim = findNextDelimiter(tokens, *CI);
    // Then, if an elif or else is after that next token, then parse that block too, adding it to toAppend.
    if (*nextDelim >= tokens->length || closeBraceIndex+1 > tokens->length) return toAppend;
    ASTNode* nextToken = (ASTNode*)List_GetElement(tokens, closeBraceIndex+1 )->data;
    if (nextToken->type == AST_ELIF) {
        parseIfStatement(CI, nextDelim, tokens, prgmNode, s);
    }
    else if (nextToken->type == AST_ELSE) {
        ASTNode* elseNode = newASTNode();
        elseNode->type = AST_ELSE;
        elseNode->right = newASTNode();
        elseNode->right->type = AST_BLOCK;
        elseNode->right->children = malloc(sizeof(List));
        *elseNode->right->children = NewList();
        int startIndex = closeBraceIndex+2;
        int endIndex = findRightParen(*tokens, startIndex, '{', '}');
        List* SL = makeSublist(*tokens, startIndex+1, endIndex-1);
        parseStatements(SL, elseNode->right, s);
        List_AppendElement(prgmNode->children, elseNode);
        *CI = endIndex + 1;
        *nextDelim = findNextDelimiter(tokens, *CI);
    }
    
    return toAppend;
} 

// Evaluates expressions to the best it can (If an operator node has two number nodes, then actually does that operation)
ASTNode* evaluateExpression(ASTNode* n) {
    
    if (n == NULL) return n;
    if (n->type == AST_OPERATOR && n->left != NULL && n->right != NULL) {
        // if n.type is operator, then recurse, evaluating the left & right.
        ASTNode* left = evaluateExpression(n->left);
        ASTNode* right = evaluateExpression(n->right);
        // If the resulting left & right are numbers, then doBinaryOperation() on left & right.
        if (left->type == AST_NUMBER && right->type == AST_NUMBER) {
            num dor = doBinaryOperation(n->value.opVal, left->value.numVal, right->value.numVal);
            ASTNode* toR = newNumNode(dor);
            toR->type = AST_NUMBER;
            return toR;
        }
        else {
            return newOperatorNode(n->value.opVal, left, right);
        }
    }
    else if (n->type == AST_OPERATOR && n->right != NULL) {
        // If the left isn't null, then evaluate the right.
        ASTNode* right = evaluateExpression(n->right);
        if (right->type == AST_NUMBER) {
            num unOp = doUnaryOperation(n->value.opVal, right->value.numVal);
            return newNumNode(unOp);
        }
        else {
            return newOperatorNode(n->value.opVal, NULL, right);
        }
    }
    // If it's an identifier, just return it.
    else if (n->type == AST_IDENTIFIER) {
        return newIdentifierNode(n->value.nameVal);
    }
    else if (n->type == AST_NUMBER) {
        // If it's a number, return it.
        ASTNode* toReturn = malloc(sizeof(ASTNode));
        *toReturn = (ASTNode){
            .children = NULL,
            .left = NULL,
            .right = NULL,
            .type = AST_NUMBER,
            .value.numVal = n->value.numVal
        };
        return toReturn;
    }
    // Otherwise, return it.
    ASTNode* cpy = malloc(sizeof(ASTNode));
    *cpy = *n;
    cpy->left = evaluateExpression(n->left);
    cpy->right = evaluateExpression(n->right);
    if (n->children != NULL) {
    cpy->children = malloc(sizeof(List));
    *cpy->children = NewList();
    for (int i = 0; i < n->children->length; i++) {
        ASTNode* toCpy = (ASTNode*)List_GetElement(n->children, i)->data;
        List_AppendElement(cpy->children, evaluateExpression(toCpy) );
    }} else cpy->children = NULL;
    cpy->value = n->value;
    return cpy;
}
void printScope(scope s) {
    for (int i = 0; i < s.variables.length; i++) {
        variable v = *(variable*)List_GetElement(&s.variables, i)->data;
        printf("Variable: %s\n", v.name);
    }
    for (int i = 0; i < s.functions.length; i++) {
        function v = *(function*)List_GetElement(&s.functions, i)->data;
        printf("Function: %s\n", v.name);
    }
    for (int i = 0; i < s.customTypes.length; i++) {
        customType ct = *(customType*)List_GetElement(&s.customTypes, i)->data;
        printf("Custom Type: %s", ct.value.name);
    }
}
// Finally, parseTokenList is the interface for main.c to itneract with for parsing.
ASTNode* parseTokenList(List* n) {

    
    // Declares programNode;  programNode.children is the list of program lines.
    ASTNode* programNode = newASTNode();
    *programNode = (ASTNode){
        .children = malloc(sizeof(List)),
        .left = NULL,
        .right = NULL,
        .type = AST_PRGM,
        .value.numberVal = 0
    };
    *(programNode->children) = NewList();
    
    // Convert all tokens to ASTNodes
    List* new = malloc(sizeof(List));
    *new = NewList();
    for (int i = 0; i < n->length; i++) {
        token t = *(token*)List_GetElement(n, i)->data;
        ASTNode* converted = tokenToASTNode(t);
        char* toFree = ASTToString(converted);
        free(toFree);
        List_AppendElement(new, converted);
    }
    // Make a new scope
    scope* s = malloc(sizeof(scope));
    s->functions = NewList();
    s->variables = NewList();
    s->customTypes = NewList();
    // Parse the statements
    parseStatements(new, programNode, s);
    // free & return programNode.
    List_Free_List(&s->customTypes);
    List_Free_List(&s->functions);
    List_Free_List(&s->variables);
    free(s);
    for (int i = n->length-1; i >= 0; i--) {
        struct List_Node* ln = List_GetElement(n, i);
        free(ln->data);
        ln->data = NULL;
        free(ln);
    }
    return programNode;
}
