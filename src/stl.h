/*
 * C STL Implementation
 */

#ifndef __ADDB_STL_H
#define __ADDB_STL_H

#define INIT_VECTOR_SIZE 10

#include <stddef.h>

typedef struct vector_ {
    void **data;
    size_t size;
    size_t count;
} vector;

void vectorInit(vector *v);
size_t vectorCount(vector *v);
void vectorAdd(vector *v, void *datum);
void vectorAddInt(vector *v, int datum);
int vectorSet(vector *v, size_t index, void *datum);
void *vectorGet(vector *v, size_t index);
int vectorDelete(vector *v, size_t index);
int vectorFree(vector *v);
int vectorFreeDeep(vector *v);

#endif
