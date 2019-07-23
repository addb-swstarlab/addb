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
#define INIT_PROTO_VECTOR_SIZE 10

#include <stddef.h>
#include "sds.h"
#include "proto/stl.pb-c.h"

/* Vector */
typedef struct _Vector {
    unsigned type:2;
    void **data;
    size_t size;
    size_t count;
} Vector;

void vectorInit(Vector *v);
void vecotrInitWithSize(Vector *v, size_t size);
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
char *VectorSerialize(void *o);
int vectorDeserialize(sds rawVector, Vector **result);
// Deprecated
Vector *VectordeSerialize(char *VectorString);
void CheckVectorsds(Vector *v);

/* Proto Vector Helper */
void protoVectorTypeInit(ProtoVector *v, int type);
int protoVectorAdd(ProtoVector *v, void *datum);
int protoVectorSet(ProtoVector *v, size_t i, void *datum);
void *protoVectorGet(ProtoVector *v, size_t i);
int protoVectorDelete(ProtoVector *v, size_t i);
void *protoVectorPop(ProtoVector *v);
int protoVectorFree(ProtoVector *v);
int protoVectorFreeDeep(ProtoVector *v);
char *protoVectorSerialize(ProtoVector *v, size_t *len);
int protoVectorDeserialize(const char *serialized, ProtoVector **result);
int protoVectorFreeDeserialized(ProtoVector *deserialized);
#endif
