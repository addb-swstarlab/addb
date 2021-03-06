/*
 * C STL Implementation
 */

#ifndef __ADDB_STL_H
#define __ADDB_STL_H

/*
 * STL type
 *  - DEFAULT: Allows all pointer types(Polymorphism)
 *  - SDS: sds
 *  - LONG: long
 *  - ROBJ: robj
 */
#define STL_TYPE_DEFAULT 0
#define STL_TYPE_SDS 1
#define STL_TYPE_LONG 2
#define STL_TYPE_ROBJ 3

#define INIT_VECTOR_SIZE 10

#include <stddef.h>
#include "sds.h"

/* Vector */
typedef struct _Vector {
    unsigned type:2;
    void **data;
    size_t size;
    size_t count;
} Vector;

void vectorInit(Vector *v);
void vectorTypeInit(Vector *v, int type);
void vectorTypeInitWithSize(Vector *v, int type, size_t size);
Vector *vectorCreate(int type, size_t size);
size_t vectorCount(Vector *v);
int vectorAdd(Vector *v, void *datum);
int vectorSet(Vector *v, size_t index, void *datum);
void *vectorGet(Vector *v, size_t index);
int vectorDelete(Vector *v, size_t index);
void *vectorUnlink(Vector *v, size_t index);
void *vectorPop(Vector *v);
int vectorFreeDatum(Vector *v, void *datum);
int vectorFree(Vector *v);
int vectorFreeDeep(Vector *v);
sds vectorToSds(Vector *v);
char *vectorSerialize(void *o);
int vectorDeserialize(sds rawVector, Vector **result);
// Deprecated
Vector *VectordeSerialize(char *VectorString);

// Implement like C++ style
typedef struct _ColumnVectorIter {
    sds col_v;
    unsigned type:2;
    size_t count;
    size_t i;
    size_t _pos;    // Internal index in rocksdb vector.
} ColumnVectorIter;

int makeColumnVectorIter(const sds col_v, ColumnVectorIter *begin,
                         ColumnVectorIter *end);
bool columnVectorIterIsEqual(const ColumnVectorIter first, const ColumnVectorIter second);
int columnVectorIterNext(ColumnVectorIter *it, int *eoi);
sds columnVectorIterGet(const ColumnVectorIter it);
int columnVectorIterGetNoCopy(const ColumnVectorIter it, char **start,
                              size_t *size);

/* Stack */
/* Implemented by using Vector */
typedef struct _Stack {
    unsigned type:2;
    Vector data;
} Stack;

void stackInit(Stack *s);
void stackTypeInit(Stack *s, int type);
size_t stackCount(Stack *s);
int stackPush(Stack *s, void *datum);
void *stackPop(Stack *s);
int stackFree(Stack *s);
int stackFreeDeep(Stack *s);
#endif
