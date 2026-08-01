#ifndef ARRAY_H
#define ARRAY_H
#include <stdbool.h>
typedef struct {
    void* data;
    int length;
    int capacity;
    int elementSize;
} array;
int getCapacity(int len);
array* mallocArray(int len);
array* newArray(void* data, int len);
void resizeArray(array* arr);
void appendArray(array* arr, void* toAppend);
void insertArray(array* arr, int index, void* toInsert);
void* removeArray(array* arr, int index);
void* popArray(array* arr);
void pushArray(array* arr, void* toInsert);
void* getArray(array* arr, int index);
void freeArray(array* arr, bool freeElements);
void replaceArray(array* arr, int idx, void* newValue);
#endif