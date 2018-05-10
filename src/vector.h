/*
 * C STL Implementation
 */

#ifndef __ADDB_STL_H
#define __ADDB_STL_H
#include <stddef.h>

typedef struct vector_ {
    void **data;
    size_t size;
    size_t count;
} vector;

void vector_init(vector *v);
size_t vector_count(vector *v);
void vector_add(vector *v, void *datum);
void vector_set(vector *v, size_t index, void *datum);
void *vector_get(vector *v, size_t index);
void vector_delete(vector *v, size_t index);
void vector_free(vector *v);

#endif
