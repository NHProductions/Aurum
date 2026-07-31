#ifndef POOL_H
#define POOL_H

#include "bytecoder.h"
typedef union poolNode poolNode;
union poolNode {
    poolNode* next;
    typedValue val; 
};
typedef struct {
    poolNode* buffer;
    poolNode* freeList;
    size_t capacity;
    int currentTotal;
} pool;
pool* createPool(int capacity);
typedValue* poolAlloc(pool* p);
void poolFree(pool* p, typedValue* tv);
void destroyPool(pool* p);

#endif