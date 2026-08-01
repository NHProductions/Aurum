#ifndef SYSFUNCTIONS_H
#define SYSFUNCTIONS_H


#include "../dataStorage/Tundora.h"
#include "../processing/parser.h"
#include "../processing/vm.h"
#include "../processing/bytecoder.h"
#include "float.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../misc/winInclude.h"
#include <time.h>
#include <complex.h>
#include <errno.h>
#include <Windows.h>
#include <direct.h>
#include "../dataStorage/num.h"
#include "../dataStorage/array.h"
#define auFunc virtualMachineState* vms, char* identifier, array* args
#define auFuncCall vms, identifier, args

char* sPrint(auFunc, bool);
void* srealloc(void* mem, int size);
void systemCall(bcFunction* bcDef, virtualMachineState* vms, int argc);
char* strArrToChar(typedValue* tv);
typedValue* numToTV(num n);
typedValue* newTVArray(int len, int type);
void appendTypedValue(typedValue* arr, typedValue* toAppend);
#endif