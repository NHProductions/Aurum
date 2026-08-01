#ifndef POOL_H
#define POOL_H

#include "../processing/bytecoder.h"
typedef union poolNode poolNode;
union poolNode {
    union poolNode* next;
    typedValue val; 
};
typedef struct {
    union poolNode* buffer;
    union poolNode* freeList;
    size_t capacity;
    int currentTotal;
} pool;
pool* createPool(int capacity);
typedValue* poolAlloc(pool* p);
void poolFree(pool* p, typedValue* tv);
void destroyPool(pool* p);

#endif