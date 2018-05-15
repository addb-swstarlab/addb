/*
 * C STL Implementation
 */

#ifndef __ADDB_STL_H
#define __ADDB_STL_H

#define INIT_VECTOR_SIZE 10

#define VECTOR_TYPE_DEFAULT 0
#define VECTOR_TYPE_SDS 1
#define VECTOR_TYPE_LONG 2

#include <stddef.h>
#include "sds.h"

typedef struct Vector_ {
    /*
     * Vector type
     *  - DEFAULT: Allows all pointer types(Polymorphism)
     *  - SDS: sds
     *  - LONG: long type
     */
    unsigned type:2;
    void **data;
    size_t size;
    size_t count;
} Vector;

void vectorInit(Vector *v);
void vectorTypeInit(Vector *v, int type);
size_t vectorCount(Vector *v);
int vectorAdd(Vector *v, void *datum);
int vectorSet(Vector *v, size_t index, void *datum);
void *vectorGet(Vector *v, size_t index);
int vectorDelete(Vector *v, size_t index);
void *vectorUnlink(Vector *v, size_t index);
int vectorFreeDatum(Vector *v, void *datum);
int vectorFree(Vector *v);
int vectorFreeDeep(Vector *v);

#endif
