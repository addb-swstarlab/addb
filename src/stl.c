#include "server.h"
#include "zmalloc.h"
#include "stl.h"
#include "sds.h"

void vectorInit(vector *v) {
    v->type = VECTOR_TYPE_DEFAULT;
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

void vectorTypeInit(vector *v, int type) {
    vectorInit(v);
    v->type = type;
}

size_t vectorCount(vector *v) {
    return v->count;
}

int vectorAdd(vector *v, void *datum) {
    if (v->size == 0) {
        vectorFreeDeep(v);
        v->size = INIT_VECTOR_SIZE;
        v->data = (void **) zmalloc(sizeof(void *) * v->size);
        memset(v->data, '\0', sizeof(void *) * v->size);
    }

    if (v->size <= v->count) {
        v->size += INIT_VECTOR_SIZE;
        v->data = (void **) zrealloc(v->data, sizeof(void *) * v->size);
    }

    v->data[v->count] = datum;
    v->count++;
    return C_OK;
}

int vectorAddInt(vector *v, int datum) {
    if (v->type != VECTOR_TYPE_INT) {
        serverLog(LL_DEBUG,
                  "FATAL ERROR: Try to add wrong element type to vector");
        serverPanic("Try to add wrong type element to vector");
        return C_ERR;
    }

    int *datum_ptr = (int *) zmalloc(sizeof(int));
    *datum_ptr = datum;
    return vectorAdd(v, (void *) datum_ptr);
}

int vectorAddSds(vector *v, sds datum) {
    if (v->type != VECTOR_TYPE_SDS) {
        serverLog(LL_DEBUG,
                  "FATAL ERROR: Try to add wrong element type to vector");
        serverPanic("Try to add wrong type element to vector");
        return C_ERR;
    }

    return vectorAdd(v, datum);
}

int vectorSet(vector *v, size_t index, void *datum) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to set element that index overflows vector");
        return C_ERR;
    }
    
    v->data[index] = datum;
    return C_OK;
}

int vectorSetSds(vector *v, size_t index, sds datum) {
    if (v->type != VECTOR_TYPE_SDS) {
        serverLog(LL_DEBUG,
                  "FATAL ERROR: Try to set wrong element type to vector");
        serverPanic("Try to set wrong type element to vector");
        return C_ERR;
    }

    if (index >= v->count) {
        serverLog(LL_DEBUG,
              "ERROR: Try to set element that index overflows vector");
        return C_ERR;
    }

    v->data[index] = (void *) datum;
    return C_OK;
}

int vectorSetInt(vector *v, size_t index, int datum) {
    if (v->type != VECTOR_TYPE_INT) {
        serverLog(LL_DEBUG,
                  "FATAL ERROR: Try to set wrong element type to vector");
        serverPanic("Try to set wrong type element to vector");
        return C_ERR;
    }

    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to set element that index overflows vector");
        return C_ERR;
    }

    zfree(v->data[index]);
    int *datumPtr = (int *) zmalloc(sizeof(int));
    *datumPtr = datum;
    v->data[index] = datumPtr;
    return C_OK;
}

void *vectorGet(vector *v, size_t index) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return NULL;
    }

    return v->data[index];
}

int vectorGetInt(vector *v, size_t index) {
    if (v->type != VECTOR_TYPE_INT) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get integer element from wrong type vector");
        return C_ERR;
    }

    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return C_ERR;
    }

    return *((int *)v->data[index]);
}

sds vectorGetSds(vector *v, size_t index) {
    if (v->type != VECTOR_TYPE_SDS) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get sds element from wrong type vector");
        return NULL;
    }

    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return NULL;
    }

    return (sds) v->data[index];
}

int vectorDelete(vector *v, size_t index) {
    if (index >= v->count)
        return C_ERR;

    v->data[index] = NULL;
    
    void **new_array = (void **) zmalloc(sizeof(void *) * v->size);
    for (size_t i = 0, j = 0; i < v->count; ++i) {
        if (v->data[i] == NULL)
            continue;
        new_array[j] = v->data[i];
        ++j;
    }
    zfree(v->data);
    
    v->data = new_array;
    v->count--;
    return C_OK;
}

int vectorFree(vector *v) {
    if (v->data == NULL)
        return C_ERR;

    zfree(v->data);
    v->data = NULL;
    return C_OK;
}

int vectorFreeDeep(vector *v) {
    if (v->data == NULL)
        return C_ERR;

    if (v->count == 0) {
        zfree(v->data);
        return C_OK;
    }

    for (size_t i = 0; i < v->count; ++i) {
        void *datum = vectorGet(v, i);
        if (v->type == VECTOR_TYPE_SDS)
            sdsfree(datum);
        else
            zfree(datum);
    }
    vectorFree(v);
    return C_OK;
}

