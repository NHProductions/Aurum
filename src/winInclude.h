#ifndef WININCLUDE
#define WININCLUDE

#define isDebug false
#define extension ".aur"
int getACP();
void setConsoleOutput(int n);
void fatalError(int code, char* n, int line);
#endif