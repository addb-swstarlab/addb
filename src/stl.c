#include "server.h"
#include "zmalloc.h"
#include "stl.h"
#include "sds.h"

void vectorInit(Vector *v) {
    v->type = VECTOR_TYPE_DEFAULT;
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

void vectorTypeInit(Vector *v, int type) {
    vectorInit(v);
    v->type = type;
}

size_t _vectorGetDatumSize(Vector *v) {
    if (v->type == VECTOR_TYPE_DEFAULT) {
        return sizeof(void *);
    } else if (v->type == VECTOR_TYPE_LONG) {
        return sizeof(long);
    } else if (v->type == VECTOR_TYPE_SDS) {
        return sizeof(sds);
    } else {
        serverLog(LL_DEBUG, "FATAL ERROR: Wrong vector type [%d]", v->type);
        serverPanic("FATAL ERROR: Wrong vector type");
        return -1;
    }
}

void _vectorResizeIfNeeded(Vector *v) {
    if (v->size == 0) {
        vectorFreeDeep(v);
        v->size = INIT_VECTOR_SIZE;
        v->data = (void **) zmalloc(_vectorGetDatumSize(v) * v->size);
    }

    if (v->size <= v->count) {
        v->size += INIT_VECTOR_SIZE;
        v->data = (void **) zrealloc(v->data,
                                     _vectorGetDatumSize(v) * v->size);
    }
}

size_t vectorCount(Vector *v) {
    return v->count;
}

int vectorAdd(Vector *v, void *datum) {
    _vectorResizeIfNeeded(v);
    size_t index = v->count;
    v->count++;
    if (vectorSet(v, index, datum) == C_ERR) {
        return C_ERR;
    }
    return C_OK;
}

int vectorSet(Vector *v, size_t index, void *datum) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to set element that index overflows vector");
        return C_ERR;
    }

    if (v->type == VECTOR_TYPE_DEFAULT) {
        v->data[index] = datum;
    } else if (v->type == VECTOR_TYPE_LONG) {
        long *dataLong = (long *) v->data;
        dataLong[index] = (long) datum;
    } else if (v->type == VECTOR_TYPE_SDS) {
        sds *dataSds = (sds *) v->data;
        dataSds[index] = (sds) sdsdup(datum);
    } else {
        serverLog(LL_DEBUG, "FATAL ERROR: Wrong vector type [%d]", v->type);
        return C_ERR;
    }

    return C_OK;
}

void *vectorGet(Vector *v, size_t index) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return NULL;
    }

    return v->data[index];
}

int vectorDelete(Vector *v, size_t index) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return C_ERR;
    }

    void **newArray = (void **) zmalloc(_vectorGetDatumSize(v) * v->size);
    for (size_t i = 0, j = 0; i < v->count; ++i) {
        if (i == index) {
            vectorFreeDatum(v, vectorGet(v, i));
            continue;
        }
        newArray[j] = v->data[i];
        ++j;
    }
    zfree(v->data);
    v->data = newArray;
    v->count--;
    return C_OK;
}

int vectorFreeDatum(Vector *v, void *datum) {
    if (v->type == VECTOR_TYPE_DEFAULT) {
        zfree(datum);
    } else if (v->type == VECTOR_TYPE_SDS) {
        sdsfree(datum);
    }
    return C_OK;
}

int vectorFree(Vector *v) {
    if (v->data == NULL)
        return C_ERR;

    zfree(v->data);
    v->data = NULL;
    v->count = 0;
    v->size = 0;
    return C_OK;
}

int vectorFreeDeep(Vector *v) {
    if (v->data == NULL)
        return C_ERR;

    for (size_t i = 0; i < v->count; ++i) {
        vectorFreeDatum(v, vectorGet(v, i));
    }
    vectorFree(v);
    return C_OK;
}

