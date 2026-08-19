#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef struct {
    void** data;
    int length;
    int capacity;
} array;
int getCapacity(int length) {
    int c = 2;
    while (c < length) {
        c *= 2;
    }
    return c;
}

// elementSize parameter removed because it is always sizeof(void*)
array* mallocArray(int length) {
    int capacity = getCapacity(length);
    array* toReturn = malloc(sizeof(array));
    *toReturn = (array){
        .capacity = capacity,
        .data = malloc(sizeof(void*) * capacity),
        .length = 0
    };
    return toReturn;
}

array* newArray(void** data, int length) {
    int capacity = getCapacity(length);
    array* toReturn = malloc(sizeof(array));
    *toReturn = (array){
        .capacity = capacity,
        .data = data,
        .length = length   
    };
    return toReturn;
}

void resizeArray(array* arr) {
    arr->capacity *= 2;
    arr->data = realloc(arr->data, (sizeof(void*) * arr->capacity));
    if (arr->data == NULL) { printf("Error: Unable to reallocate array\n"); exit(1); }
}

void appendArray(array* arr, void* toAppend) {
    if (arr->length >= arr->capacity) {
        resizeArray(arr);
    }
    arr->data[arr->length] = toAppend; // Direct assignment of the pointer
    arr->length++;
}

void insertArray(array* arr, int index, void* toInsert) {
    if (index > arr->length || index < 0) { printf("Invalid insert\n"); exit(1); }
    if (arr->length + 1 > arr->capacity) resizeArray(arr);
    
    if (index < arr->length) {
        memmove(&arr->data[index + 1], 
                &arr->data[index], 
                (arr->length - index) * sizeof(void*));
    }
    
    arr->data[index] = toInsert;
    arr->length++;
}

void* removeArray(array* arr, int index) {
    if (index >= arr->length || index < 0) { 
        printf("Invalid array access\n"); exit(1); }
    
    void* removedElement = arr->data[index]; // Save pointer to return it
    
    if (index < arr->length - 1) {
        memmove(&arr->data[index], 
                &arr->data[index + 1], 
                (arr->length - index - 1) * sizeof(void*));
    }
    arr->length--;
    return removedElement;
}

void* getArray(array* arr, int index) {
    if (index < 0 || index >= arr->length) {
        printf("Array Access Error\n"); 
        exit(1);
    }
    return arr->data[index];
}

void* popArray(array* arr) {
    if (arr->length == 0) return NULL;
    return removeArray(arr, arr->length - 1);
}

void pushArray(array* arr, void* toInsert) {
    appendArray(arr, toInsert);
}

void freeArray(array* arr, bool freeElements) {
    if (freeElements) {
        for (int i = 0; i < arr->length; i++) {
            if (arr->data[i] != NULL) {
                free(arr->data[i]);
            }
        }
    }
    free(arr->data);
    free(arr);
}

void replaceArray(array* arr, int idx, void* newValue) {
    if (idx >= arr->length || idx < 0) { printf("Invalid array access\n"); exit(1); }
    arr->data[idx] = newValue;
}