/* 
--- pool.c ---
This is what's used for allocating typedValues. It's much more efficient than using malloc() and free() because it reduces the amount of recursion & overhead.
*/

#include "pool.h"
#include "bytecoder.h"

pool* createPool(int capacity) {
    pool* toReturn = malloc(sizeof(pool));
    toReturn->buffer = malloc(sizeof(poolNode) * capacity);
    toReturn->capacity = capacity;
    for (int i = 0; i < capacity - 1; i++) {
        toReturn->buffer[i].next = &toReturn->buffer[i+1];
    }
    toReturn->buffer[capacity-1].next = NULL;

    toReturn->freeList = &toReturn->buffer[0];
    toReturn->currentTotal = 0;
    return toReturn;
};
typedValue* poolAlloc(pool* p) {
    if (p->freeList == NULL) {printf("Out of typedValue slots!"); exit(1);}
    poolNode* node = p->freeList;
    p->freeList = p->freeList->next;
    p->currentTotal++;
    return (typedValue*)node;
};
void poolFree(pool* p, typedValue* tv) {
    if (tv == NULL) return;
    poolNode* node = (poolNode*)tv;
    node->next = p->freeList;
    p->freeList = node;
    p->currentTotal--;
};
void destroyPool(pool* p) {
    free(p->buffer);
    free(p);
};