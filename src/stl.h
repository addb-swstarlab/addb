/*
 * C STL Implementation
 */

#ifndef __ADDB_STL_H
#define __ADDB_STL_H

#define INIT_VECTOR_SIZE 10

#define VECTOR_TYPE_DEFAULT 0
#define VECTOR_TYPE_SDS 1
#define VECTOR_TYPE_INT 2

#include <stddef.h>
#include "sds.h"

typedef struct vector_ {
    /*
     * Vector type
     *  - DEFAULT: Allows all pointer types(Polymorphism)
     *  - SDS: sds
     *  - INT: integer
     */
    unsigned type:2;
    void **data;
    size_t size;
    size_t count;
} vector;

void vectorInit(vector *v);
void vectorTypeInit(vector *v, int type);
size_t vectorCount(vector *v);
int vectorAdd(vector *v, void *datum);
int vectorAddInt(vector *v, int datum);
int vectorAddSds(vector *v, sds datum);
int vectorSet(vector *v, size_t index, void *datum);
void *vectorGet(vector *v, size_t index);
int vectorDelete(vector *v, size_t index);
int vectorFree(vector *v);
int vectorFreeDeep(vector *v);

#endif
