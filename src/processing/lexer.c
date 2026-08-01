/*
--- Lexer.c --- 
Lexing is the process of turning a string/file into a list of tokens.
This implementation does that by going line-by-line, emitting tokens that it finds.
During this stage, comments get removed (lines starting with `).
In addition, hexadecimal/octal literals get turned into decimal equivalents, & escape characters are parsed.

I'm sure you're familiar with fizzbuzz, but if you're not, it's a test used for new-programmers, and in this case, it's going to be used as a test for this language.
The rules for fizzbuzz are follows:
    - Count sequentially, starting from 1.
    - If a number is divisible by 3, say "fizz".
    - If a number is divisible by 5, say "buzz".
    - If a number is divisible by both 3 & 5, say "fizzbuzz".
    - Otherwise, just say the number.
Below is the code I used (See the Standard document, if you want to know about the syntax of this language)
function fizzbuzz(int amt) -> void {
    for (int n = 1; n <= amt; n++) {
        if ( (n % 3 == 0) && (n % 5 != 0) ) {
            print("fizz\n");
        }
        elif ( (n % 5 == 0) && (n % 3 != 0) ) {
            print("buzz\n");
        }
        elif ( (n % 5 == 0) && (n % 3 == 0) ) {
            print("fizzbuzz\n");
        }
        else {
            print("%i\n", n);
        }
    }
}

Here's a tokenized version of this:
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

A trend that you'll probably see in this stage, and in the other stages, is that the data being used gets simpler and simpler (Just now, it went from a list of some hundred characters to just a list of like 50 tokens)


Files to check next: lexer.h, parser.c
*/

// standard C library imports:
#include <stdio.h>
// Aurum imports:
#include "../dataStorage/Tundora.h"
#include "lexer.h"
#include "../misc/winInclude.h"
#include <locale.h>


// inclusiveRange() is kinda self explanatory
bool inclusiveRange(int n, int min, int max) {
    return (n >= min && n <= max);
}
// I've chosen to define identifier characters as being A-Z, a-z, 0-9, and _.
bool isIdentifierChar(char c) {
    return (inclusiveRange(c, 'a', 'z') ||
    inclusiveRange(c, 'A', 'Z') ||
    inclusiveRange(c, '0', '9') ||
    c == '_');
}
// This is the set of characters I've decided to be symbol characters. These are characters that are either used in numbers, operators, or some keywords.
bool isSymbolChar(char c) {
    // {}()[]<>=%-~!|&^*/-+
    if (c >= 33 && c <= 62) return true; // ! " # $ % & ' ( ) * + , - . / 0123456789 : ; < = >
    if (c >= 91 && c <= 95) return true; // [ \ ] ^ _
    if (c >= 123 && c <= 126) return true; // { | }
    if (c == 64) return true; // @
    return false;
}
// Self-explanatory; all valid chars are either identifiers or symbols.
bool isValidChar(char n) {
    return (isIdentifierChar(n) || isSymbolChar(n) );
}
// Checks if a string contains a character.
bool containsChar(char* str, char delim) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == delim) return true;
    }
    return false;
}

// When given a string, an index, and an array of tokens, returns if it contains a token.
// If it contains that token, returns the token found & its length.
// If nonIDCharsNearby is true, only finds tokens with non-identifier characters nearby.
struct tokenFound findTypedToken(char* n, int index,const char** tokSet, int tokSetLength, bool nonIDCharsNearby) { 
    int nlen = strlen(n); 
    char currentMatch[500]; 
    currentMatch[0] = '\0'; 
    for (int i = 0; i < tokSetLength; i++) { 
        const char* currentToken = tokSet[i]; 
        if (strlen(currentToken) > nlen - index) continue;
        int cTokLen = strlen(currentToken); 
        char toCheck[50]; toCheck[0] = '\0'; 
        for (int j = 0; j < cTokLen; j++) { 
            int currentLen = strlen(toCheck);
             toCheck[currentLen] = n[index+j];
             toCheck[currentLen+1] = '\0'; 
        } 
        if (strcmp(toCheck, currentToken) == 0) { 
            if (nonIDCharsNearby) { 
                char before = (index == 0) ? '\0' : n[index - 1];
                char after = n[index + cTokLen]; 
                bool validBefore = !isIdentifierChar(before); 
                bool validAfter = !isIdentifierChar(after); 
                if (validBefore && validAfter && strlen(currentToken) >= strlen(currentMatch)) { 
                    sprintf(currentMatch, "%s", currentToken); 
                } 
            } else if (strlen(currentToken) >= strlen(currentMatch)) { 
                sprintf(currentMatch, "%s", currentToken); 
            } 
        } 
    } 
    return (struct tokenFound){strdup(currentMatch), strlen(currentMatch)}; 
}
// Forces letters to lowercase.
char toLower(char n) {
    if (n >= 'A' && n <= 'Z') return n ^ ' ';
    return n;
}
// For numbers, only base 2, 16, 8, and 10 are valid for literals (There's a function to switch bases in the standard library).
// This functions returns if a character is a valid number in a base.
bool isAcceptableBase(int base, char n) {
    if (base == 16) return inclusiveRange(n, '0', '9') || inclusiveRange(n, 'A', 'F');
    if (base == 8) return inclusiveRange(n, '0', '7');
    if (base == 2) return inclusiveRange(n, '0', '1');
    if (base == 10) return inclusiveRange(n, '0', '9');
    return false;
}
/*
Parses a number literal.
First, it checks the starting character for a base specifier, which is either 0x___, 0d___, 0o___, or 0b___ for hex, decimal, octal, and binary respectively.
Then, it parses the actualy number, grabbing any number tokens.
Next, after it parsed all numbers, it checks the characters after it for a size indicator (c, s, i, l, f, and d, which are char, short, int, long, float, & double).
Concurrent with that, it also checks if there's a sign indicator (uc, us, ui, or ul), and a long-double indicator (ld).

Finally, it returns the token found.
*/
struct tokenFound findNumberToken(char* n, int index) {
    if (!inclusiveRange(n[index], '0', '9')) return (struct tokenFound){.amtToSkip = 0, .token = strdup(""), .type = 0};
    int i = index;
    struct tokenFound toReturn = (struct tokenFound){.amtToSkip = 0, .token = NULL, .type = T_NUMBER };
    char tostrdup[500];
    tostrdup[0] = '\0';
    int base = 10;
    if (n[i] == '0' && i < strlen(n)-1) {
        switch (toLower(n[i+1]) ) {
            case 'x': {base = 16; i += 2; toReturn.amtToSkip += 2; break;}
            case 'o': {base = 8; i += 2;  toReturn.amtToSkip += 2; break;}
            case 'b': {base = 2; i += 2;  toReturn.amtToSkip += 2; break;}
            case 'd': {base = 10; i += 2; toReturn.amtToSkip += 2; break;}
        }
    }
    while (isAcceptableBase(base, n[i]) || n[i] == '.') {
        if (base != 10 && n[i] == '.') fatalError(0x40, "", -1);
        int currentLen = strlen(tostrdup);
        tostrdup[currentLen] = n[i];
        tostrdup[currentLen+1] = '\0';
        i++;
        toReturn.amtToSkip++; 
    }
    if (n[i] >= 'A' && n[i] <= 'Z') {
        n[i] = n[i] ^ 32;
    }
    while (n[i] == 'f' || n[i] == 'c' || n[i] == 'i' || n[i] == 'u' || n[i] == 'l' || n[i] == 'd' || n[i] == 's') {
        
        int l = strlen(tostrdup);
        tostrdup[l] = n[i];
        tostrdup[l+1] = '\0';
        i++;
        if (n[i] >= 'A' && n[i] <= 'Z') {
            n[i] = n[i] ^ 32;
        }
        toReturn.amtToSkip++;
    }
    // if base != 10, convert it to base 10.
    if (base != 10) {
        // use strtoll() to convert it into a base.
        // convert it back into a string using sprintf
        // then it's base 10, usable for parsing.
        long long ll = strtoll(tostrdup, NULL, base);
        sprintf(tostrdup, "%lld", ll);
    }
    toReturn.token = strdup(tostrdup);
    return toReturn;
}
/*
This function takes in a string, and returns if it has an escape character. (\\, \n, etc)
I've also added some addition escape chars for 1252 page.
if it finds no escape characters, returns 0.
Also has an option to grab characters directly using their hex code
e.x \xF7
*/
char getEscapeChar(char* es) {
    
    if (strcmp(es, "\\n") == 0) return '\n'; // newline
    if (strcmp(es, "\\t") == 0) return '\t'; // tab
    if (strcmp(es, "\\\"") == 0) return '\"'; // double quotes
    if (strcmp(es, "\\\'") == 0) return '\''; // single quotes
    if (strcmp(es, "\\0") == 0) return '\0'; // null terminator
    if (strcmp(es, "\\x") == 0) return 'x'; // x (needed for ASCII-grab)
    if (strcmp(es, "\\\\") == 0) return '\\'; // \\ 
    int ansiPage = getACP(); // todo: add localization for other escape chars.
    char* locale = setlocale(LC_CTYPE, "");
    if (strcmp(locale, "English_United States.1252") == 0) { // Todo: add more ansi page types for other localizations.
        if (strcmp(es, "\\e") == 0) return 128; // euro
        if (strcmp(es, "\\tm") == 0) return 153; // trademark
        if (strcmp(es, "\\em") == 0) return 151; // em dash
        if (strcmp(es, "\\en") == 0) return 150; // en dash
        if (strcmp(es, "\\c") == 0) return 162; // cent
        if (strcmp(es, "\\p") == 0) return 163; // pound
        if (strcmp(es, "\\y") == 0) return 165; // yen
        if (strcmp(es, "\\s") == 0) return 167; // section
        if (strcmp(es, "\\cp") == 0) return 169; // copyright
        if (strcmp(es, "\\r") == 0) return 174; // rights
        if (strcmp(es, "\\pm") == 0) return 177; // plus-minus
        if (strcmp(es, "\\mi") == 0) return 181; // micro (mu)
        if (strcmp(es, "\\pa") == 0) return 182; // paragraph
    }
    if (es[1] == 'x') {
        // custom-grab
        char substr[10]; // +1 for term, -1 for backslash
        substr[0] = '\0';
        for (int i = 2; i < strlen(es); i++) {
            substr[i-2] = es[i];
            substr[i-1] = '\0';
        }
        char toconv[100]; sprintf(toconv, "0x%s", substr);
        long converted = strtoll(toconv, NULL, 16);
        return (char)converted;
    }
    return 0;
}
/*
This function is responsible for finding string tokens. It also processes character literals (e.x '1')
*/
struct tokenFound findStringToken(char* n, int index) {
    if (n[index] == '\'' && index < strlen(n)-2) {
        if (n[index+1] != '\\') {
            if (n[index+2] == '\'') {
                char* tok = malloc(2);
                tok[0] = n[index+1];
                tok[1] = '\0';
                return (struct tokenFound){.amtToSkip = strlen(tok)+2, .token = tok, .type = T_CHAR };
            }
        }
        else {
            char t[5]; t[0] = '\0';
            for (int i = 0; i < 5; i++) {
                if (n[index+i+1] == '\'') break;
                int k = strlen(t);
                t[k] = n[index+i+1];
                t[k+1] = '\0';
            }
            char e = getEscapeChar(t);
            char* tok = malloc(strlen(t));
            sprintf(tok, "%c", e);
            return (struct tokenFound){.amtToSkip = strlen(t)+2, .token = tok, .type = T_CHAR};
        }
    }
    if (n[index] == '\"') {
        int i = index+1;
        char str[500];
        str[0] = '\0';
        int totalToSkip = 0;
        while (i < strlen(n) && n[i] != '\"') {
            char newChar = n[i];
            if (newChar != '\\') {
                int len = strlen(str);
                str[len] = n[i];
                str[len+1] = '\0';
                i++;
                totalToSkip++;
            }
            else {
                char t[5]; t[0] = '\0';
                for (int j = 0; j < 5 && i+j < strlen(n); j++) {
                    if (n[i+j] == '\\' && i+j != i && j > 0) {sprintf(t, "%s%c", t, '\\'); break;}
                    if ((j > 0 && ( (j >= 1 && n[i+j] == '\"') || n[i+j] == ' ' || n[i+j] == '\\') ) ) break;
                    int k = strlen(t);
                    if (j > 0) {
                        char tmp[5]; tmp[0] = '\0'; sprintf(tmp, "%s%c", t, n[i+j]);
                        if (getEscapeChar(tmp) == 0 && n[i+j] != '0') {
                            break;
                        }
                    }
                    t[k] = n[i+j];
                    t[k+1] = '\0';
                    if (j >= 1 && n[i+j] == '\"') break;
                }

                char escp = getEscapeChar(t);
                int len = strlen(str);;
                str[len] = escp;
                str[len+1] = '\0';
                i += strlen(t);
                totalToSkip += strlen(t);
            }
            if (n[i] == '\"') break;
        }
        return (struct tokenFound){.amtToSkip = totalToSkip+2, .token = strdup(str), .type = T_STRING};
    }
    return (struct tokenFound){.amtToSkip = 0, .token = strdup(""), .type = T_NONE};
}
/* 
This finds library literals (e.x @math)
Essentially, if it starts with a @, then it's a library.

*/
struct tokenFound findLibraryToken(char* n, int index) {
    if (n[index] == '@') {
        char str[500];
        str[0] = '\0';
        for (int i = index; i < strlen(n); i++) {
            if (n[i] == '\n' || n[i] == '\0' || n[i] == ';') {
                break;
            }
            else {
                int k = strlen(str);
                str[k] = n[i];
                str[k+1] = '\0';
            };
        }
        return (struct tokenFound){.amtToSkip = strlen(str)+1, .token = strdup(str), .type = T_LIBRARY};
    }
    return (struct tokenFound){.amtToSkip = 0, .token = strdup(""), .type = T_NONE};
}
// If no other tokens have been found, then this grabs as many identifier characters as it can, and makes those chars into an identifier token.
struct tokenFound findIdentifierToken(char* n, int index) {
    

    char toReturn[500];
    toReturn[0] = '\0';
    int i = index;
    while (isIdentifierChar(n[i]) && i < strlen(n)) {
        int len = strlen(toReturn);
        toReturn[len] = n[i];
        toReturn[len+1] = '\0';
        i++;
    }
    if (strlen(toReturn) == 0) {
        return (struct tokenFound){.amtToSkip = 0, .token = strdup(""), .type = T_NONE};
    }
    return (struct tokenFound){.amtToSkip = strlen(toReturn), .token = strdup(toReturn), .type = T_IDENTIFIER};
}
// This function combines the other findToken functions so that it's only one call.
struct tokenFound findAnyToken(char* n, int index) {
    struct tokenFound currentRes = findTypedToken(n, index, auKeywords, KW_LEN, true);
    if (currentRes.amtToSkip != 0) {
        return (struct tokenFound){.amtToSkip = currentRes.amtToSkip, .token = currentRes.token, .type = T_KEYWORD};
    }
    free(currentRes.token);
    currentRes = findTypedToken(n, index, auComma, CO_LEN, false);
    if (currentRes.amtToSkip != 0) {
        return (struct tokenFound){.amtToSkip = currentRes.amtToSkip, .token = currentRes.token, .type = T_KEYWORD};
    }
    free(currentRes.token);
    currentRes = findTypedToken(n, index, auOperators, OP_LEN, false) ;
    if (currentRes.amtToSkip != 0) {
        return (struct tokenFound){.amtToSkip = currentRes.amtToSkip, .token = currentRes.token, .type = T_OPERATOR};
    }
    free(currentRes.token);
    currentRes = findTypedToken(n, index, auGrouping, GR_LEN, false);
    if (currentRes.amtToSkip != 0) {
        return (struct tokenFound){.amtToSkip = currentRes.amtToSkip, .token = currentRes.token, .type = T_GROUPING};
    }
    free(currentRes.token);
    currentRes = findTypedToken(n, index, auDelimiter, DE_LEN, false);
    if (currentRes.amtToSkip != 0) {
        return (struct tokenFound){.amtToSkip = currentRes.amtToSkip, .token = currentRes.token, .type = T_DELIMITER};
    }
    free(currentRes.token);
    currentRes = findStringToken(n, index);
    if (currentRes.amtToSkip != 0) return currentRes;
    free(currentRes.token);
    currentRes = findLibraryToken(n, index);
    if (currentRes.amtToSkip != 0) return currentRes;
    free(currentRes.token);
    currentRes = findNumberToken(n, index);
    if (currentRes.amtToSkip != 0) return currentRes;
    free(currentRes.token);
    if (currentRes.amtToSkip == 0) {
        currentRes = findIdentifierToken(n, index);
        return currentRes;
    }
    free(currentRes.token);
    if (isDebug) printf("No token found %s", n);
    return currentRes;
}
// Converts tokens to strings.
char* tokenTypeToString(int type) {
    switch (type) {
        case T_NONE: {return "NONE";}
        case T_IDENTIFIER: {return "IDENTIFIER";}
        case T_OPERATOR: {return "OPERATOR";}
        case T_NUMBER: {return "NUMBER";}
        case T_STRING: {return "STRING";}
        case T_CHAR: {return "CHAR";}
        case T_KEYWORD: {return "KW";}
        case T_DELIMITER: {return "DELIMITER";}
        case T_GROUPING: {return "GROUP";}
        case T_LIBRARY: {return "LIBRARY";}
    }
    return "N/A";
}
// Prints an array of tokens. Basically jsut a debug function.
void printTokenList(List* lst) {
    printf("TOKEN LENGTH: %d", lst->length);
    for (int i = 0; i < lst->length; i++) {
        token index = *(token*)List_GetElement(lst, i)->data;
        printf("%s(%s)\n", tokenTypeToString(index.type), index.value);
    }
}
/*
This code actually tokenizes the string
First, it splits the code into lines (splitting by \n)
Next, it any line starts with '`', then it skips it
Then, it parses any tokens it finds, skipping that token's length, and appending the token into the list to return.
*/
List* tokenize(char* source) {
    List* toReturn = malloc(sizeof(List));
    *toReturn = NewList();
    
    char* copy = strdup(source);
    char* line = strtok(copy, "\n\r");

    while (line != NULL) {

        // Remove comment from THIS line only
        char* commentStart = strchr(line, '`');
        if (commentStart != NULL) {
            *commentStart = '\0';
        }

        int len = strlen(line);

        for (int i = 0; i < len; i++) {
            if (!isValidChar(line[i])) {
                continue;
            }

            struct tokenFound tkf = findAnyToken(line, i);

            if (tkf.amtToSkip > 0) {
                token* toAppend = malloc(sizeof(token));

                *toAppend = (token){
                    .type = tkf.type,
                    .value = tkf.token
                };

                List_AppendElement(toReturn, toAppend);

                i += tkf.amtToSkip - 1;
            }
        }

        line = strtok(NULL, "\n\r");
    }

    free(copy);
    return toReturn;
}











