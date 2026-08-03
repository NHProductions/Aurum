#ifndef WININCLUDE
#define WININCLUDE

#define isDebug false 
#define isDebugX false 
#define saveAsAurX false 
#define extension ".aur"
#define executableExtension ".aurx"
int getACP();
void setConsoleOutput(int n);
void fatalError(int code, char* n, int line);
#endif
