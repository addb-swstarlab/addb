#include "server.h"
#include "zmalloc.h"
#include "stl.h"
#include "sds.h"
#include "global.h"
#include "assert.h"

size_t _vectorGetDatumSize(Vector *v) {
    if (v->type == STL_TYPE_DEFAULT) {
        return sizeof(void *);
    } else if (v->type == STL_TYPE_LONG) {
        return sizeof(long);
    } else if (v->type == STL_TYPE_SDS) {
        return sizeof(sds);
    } else if (v->type == STL_TYPE_ROBJ) {
        return sizeof(robj *);
    } else {
        serverLog(LL_DEBUG, "FATAL ERROR: Wrong vector type [%d]", v->type);
        serverPanic("FATAL ERROR: Wrong vector type");
        return -1;
    }
}

void vectorInit(Vector *v) {
    v->type = STL_TYPE_DEFAULT;
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

void vectorInitWithSize(Vector *v, size_t size) {
    vectorInit(v);
    v->size = size;
    v->data = (void **) zmalloc(_vectorGetDatumSize(v) * v->size);
}

void vectorTypeInit(Vector *v, int type) {
    vectorInit(v);
    v->type = type;
}

void vectorTypeInitWithSize(Vector *v, int type, size_t size) {
    vectorTypeInit(v, type);
    v->size = size;
    v->data = (void **) zmalloc(_vectorGetDatumSize(v) * v->size);
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

    if (v->type == STL_TYPE_DEFAULT) {
        v->data[index] = datum;
    } else if (v->type == STL_TYPE_LONG) {
        long *dataLong = (long *) v->data;
        dataLong[index] = (long) datum;
    } else if (v->type == STL_TYPE_SDS) {
        sds *dataSds = (sds *) v->data;
        dataSds[index] = (sds) datum;
    } else if (v->type == STL_TYPE_ROBJ) {
        robj **dataRobj = (robj **) v->data;
        dataRobj[index] = (robj *) datum;
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

void *vectorUnlink(Vector *v, size_t index) {
    if (index >= v->count) {
        serverLog(LL_DEBUG,
                  "ERROR: Try to get element that index overflows vector");
        return NULL;
    }

    void **newArray = (void **) zmalloc(_vectorGetDatumSize(v) * v->size);
    void *target;
    for (size_t i = 0, j = 0; i < v->count; ++i) {
        if (i == index) {
            target = v->data[i];
            continue;
        }
        newArray[j] = v->data[i];
        ++j;
    }
    zfree(v->data);
    v->data = newArray;
    v->count--;
    return target;
}

void *vectorPop(Vector *v) {
    void *target = vectorGet(v, v->count - 1);
    vectorSet(v, v->count - 1, NULL);
    v->count--;
    return target;
}

int vectorFreeDatum(Vector *v, void *datum) {
    if (v->type == STL_TYPE_DEFAULT) {
        zfree(datum);
    } else if (v->type == STL_TYPE_SDS) {
        sdsfree((sds) datum);
    } else if (v->type == STL_TYPE_ROBJ) {
        decrRefCount((robj *) datum);
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

sds vectorToSds(Vector *v) {
    sds vectorSds = sdsempty();
    for(int i = 0; i < vectorCount(v); ++i) {
        vectorSds = sdscatsds(vectorSds, vectorGet(v, i));
        if (i == vectorCount(v) - 1) {
            break;
        }
        vectorSds = sdscat(vectorSds, " ");
    }
    return vectorSds;
}

void stackInit(Stack *s) {
    s->type = STL_TYPE_DEFAULT;
    vectorInit(&s->data);
}

void stackTypeInit(Stack *s, int type) {
    stackInit(s);
    s->type = type;
    s->data.type = type;
}

size_t stackCount(Stack *s) {
    return s->data.count;
}

int stackPush(Stack *s, void *datum) {
    return vectorAdd(&s->data, datum);
}

void *stackPop(Stack *s) {
    return vectorPop(&s->data);
}

int stackFree(Stack *s) {
    return vectorFree(&s->data);
}

int stackFreeDeep(Stack *s) {
    return vectorFreeDeep(&s->data);
}

char *VectorSerialize(robj *o){

	Vector *v = (Vector *)o->ptr;
	int v_type = v->type;
	int v_count = v->count;

		sds serial_buf = sdscatfmt(sdsempty(), "%s{%s%i:%s%i}:%s:%s",RELMODEL_VECTOR_PREFIX, RELMODEL_VECTOR_TYPE_PREFIX,
				v_type, RELMODEL_VECTOR_COUNT_PREFIX, v_count, RELMODEL_DATA_PREFIX,VECTOR_DATA_PREFIX);

		int i;
		for(i=0; i < v_count; i++){
			sds element = (sds)vectorGet(v,i);
			serial_buf= sdscatsds(serial_buf, element);

			if(i < (v_count -1)){
				serial_buf = sdscat(serial_buf,RELMODEL_DELIMITER);
			}
		}
		serial_buf = sdscat(serial_buf, VECTOR_DATA_SUFFIX);

		serverLog(LL_DEBUG, "(char version)SERIALIZE VECTOR : %s", serial_buf);

		char *string_buf = zmalloc(sizeof(char) * (sdslen(serial_buf) + 1));
		memcpy(string_buf, serial_buf, sdslen(serial_buf));
		string_buf[sdslen(serial_buf)] = '\0';
		sdsfree(serial_buf);
		return string_buf;

}

Vector *VectordeSerialize(char *VectorString){
	serverLog(LL_VERBOSE, "DESERIALIZE : %s", VectorString);
	assert(VectorString != NULL);
    serverLog(LL_VERBOSE, "Pass assert");
	char str_buf[1024];
	char *token = NULL;
	char *saveptr = NULL;
	size_t size = strlen(VectorString) + 1;

	memcpy(str_buf, VectorString, size);

	if ((token = strtok_r(str_buf, RELMODEL_VECTOR_PREFIX, &saveptr)) == NULL){
		//parsing the vector prefix
		serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
		serverAssert(0);
	}

	/*skip vector prefix ("V") */
	if (strcasecmp(token, RELMODEL_VECTOR_PREFIX) == 0 ) {
		// skip idxType field
		strtok_r(NULL, RELMODEL_BRACE_PREFIX, &saveptr);
	}


	// parsing Vector type
	if ((token = strtok_r(NULL, RELMODEL_VECTOR_TYPE_PREFIX, &saveptr)) == NULL){
		serverLog(LL_WARNING, "Fatal:  Vector deSerialize broken Error: [%s]", str_buf);
		serverAssert(0);
	}

	int vector_type = atoi(token);


	//parsing Vector count
	if ((token = strtok_r(NULL, RELMODEL_VECTOR_COUNT_PREFIX, &saveptr)) == NULL){
		serverLog(LL_WARNING, "Fatal:  Vector deSerialize broken Error: [%s]", str_buf);
		serverAssert(0);
	}

	int vector_count = atoi(token);

	serverLog(LL_VERBOSE, "vector type : %d, count : %d", vector_type, vector_count);

	//create vector
	Vector *v = zmalloc(sizeof(Vector));
    vectorTypeInitWithSize(v, vector_type, vector_count);
	assert(v->size == vector_count);
	assert(v->count == 0);

	if(vector_count == 1){

		if ((token = strtok_r(NULL, RELMODEL_VECTOR_DATA_PREFIX, &saveptr)) == NULL){
			serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
			serverAssert(0);
		}

		if((token = strtok_r(token, VECTOR_DATA_SUFFIX, saveptr)) == NULL){
			serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
			serverAssert(0);
		}
		serverLog(LL_VERBOSE, "idx : %d, value : %s", vector_count-1, token);
		vectorAdd(v, sdsnew(token));
        serverLog(LL_VERBOSE, "Vector deserialize finished");
        return v;
	}

    int i;

    for(i=0; i < vector_count; i++){
        if (i == (vector_count - 1)) {
            serverLog(LL_VERBOSE, "Final round");
        }
        serverLog(LL_VERBOSE, "[%d] remain: %s", i, saveptr);
        if(i == 0){
            if ((token = strtok_r(NULL, RELMODEL_VECTOR_DATA_PREFIX, &saveptr)) == NULL){
                serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
                serverAssert(0);
            }

            serverLog(LL_VERBOSE, "idx : %d, value : %s", i, token);
            vectorAdd(v, sdsnew(token));

        }
        else if(i == (vector_count -1)){
            if ((token = strtok_r(NULL, VECTOR_DATA_SUFFIX, &saveptr)) == NULL){
                serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
                serverAssert(0);
            }

            serverLog(LL_VERBOSE, "idx : %d, value : %s", i, token);
            vectorAdd(v, sdsnew(token));

        }
        else {
            //parsing Vector Data
            if ((token = strtok_r(NULL, RELMODEL_DELIMITER, &saveptr)) == NULL){
                serverLog(LL_WARNING, "Fatal: Vector deSerialize broken Error: [%s]", str_buf);
                serverAssert(0);
            }

            serverLog(LL_VERBOSE, "idx : %d, value : %s", i, token);
            vectorAdd(v, sdsnew(token));

        }
    }

    serverLog(LL_VERBOSE, "Vector deserialize finished");
	return v;
}

int vectorDeserialize(sds rawRocksDBVector, Vector **result) {
	if (rawRocksDBVector == NULL) {
        return C_ERR;
    }

	char *token = NULL;
	char *saveptr = NULL;
    int vector_type = -1;
    int vector_count = -1;

    token = strtok_r(rawRocksDBVector, RELMODEL_VECTOR_PREFIX, &saveptr);
	if (token == NULL) {
		//parsing the vector prefix
		serverLog(
            LL_WARNING,
            "Fatal: Vector deSerialize broken Error: [%s]",
            rawRocksDBVector);
		return C_ERR;
	}

	/*skip vector prefix ("V") */
	if (strcmp(token, RELMODEL_VECTOR_PREFIX) == 0) {
		// skip idxType field
		strtok_r(NULL, RELMODEL_BRACE_PREFIX, &saveptr);
	}

	// parsing Vector type
    token = strtok_r(NULL, RELMODEL_VECTOR_TYPE_PREFIX, &saveptr);
	if (token == NULL) {
		serverLog(
            LL_WARNING,
            "Fatal:  Vector deSerialize broken Error: [%s]",
            rawRocksDBVector);
		return C_ERR;
	}

	vector_type = atoi(token);
	//parsing Vector count
    token = strtok_r(NULL, RELMODEL_VECTOR_COUNT_PREFIX, &saveptr);
	if (token == NULL) {
		serverLog(
            LL_WARNING,
            "Fatal:  Vector deSerialize broken Error: [%s]",
            rawRocksDBVector);
		return C_ERR;
	}

	vector_count = atoi(token);
	serverLog(LL_VERBOSE, "vector type : %d, count : %d", vector_type, vector_count);

	//create vector
    *result = zmalloc(sizeof(Vector));
    vectorTypeInitWithSize(*result, vector_type, vector_count);

	if (vector_count == 1) {
        token = strtok_r(NULL, RELMODEL_VECTOR_DATA_PREFIX, &saveptr);
		if (token == NULL) {
			serverLog(
                LL_WARNING,
                "Fatal: Vector deSerialize broken Error: [%s]",
                rawRocksDBVector);
            return C_ERR;
		}

        token = strtok_r(token, VECTOR_DATA_SUFFIX, saveptr);
		if (token == NULL) {
			serverLog(
                LL_WARNING,
                "Fatal: Vector deSerialize broken Error: [%s]",
                rawRocksDBVector);
			return C_ERR;
		}
		serverLog(LL_DEBUG, "idx : %d, value : %s", vector_count - 1, token);
		vectorAdd(*result, sdsnew(token));
        serverLog(LL_VERBOSE, "Vector deserialize finished");
        return C_OK;
	}

    int last_index = vector_count - 1;
    for (int i = 0; i < vector_count; i++) {
        if (i == last_index) {
            serverLog(LL_DEBUG, "Final round");
        }
        serverLog(LL_DEBUG, "[%d] remain: %s", i, saveptr);
        if (i == 0) {
            token = strtok_r(NULL, RELMODEL_VECTOR_DATA_PREFIX, &saveptr);
            if (token == NULL) {
                serverLog(
                    LL_WARNING,
                    "Fatal: Vector deSerialize broken Error: [%s]",
                    rawRocksDBVector);
                return C_ERR;
            }

            serverLog(LL_DEBUG, "idx : %d, value : %s", i, token);
            vectorAdd(*result, sdsnew(token));

        } else if (i == last_index) {
            token = strtok_r(NULL, VECTOR_DATA_SUFFIX, &saveptr);
            if (token == NULL){
                serverLog(
                    LL_WARNING,
                    "Fatal: Vector deSerialize broken Error: [%s]",
                    rawRocksDBVector);
                return C_ERR;
            }

            serverLog(LL_DEBUG, "idx : %d, value : %s", i, token);
            vectorAdd(*result, sdsnew(token));

        } else {
            //parsing Vector Data
            token = strtok_r(NULL, RELMODEL_DELIMITER, &saveptr);
            if (token == NULL) {
                serverLog(
                    LL_WARNING,
                    "Fatal: Vector deSerialize broken Error: [%s]",
                    rawRocksDBVector);
                return C_ERR;
            }

            serverLog(LL_DEBUG, "idx : %d, value : %s", i, token);
            vectorAdd(*result, sdsnew(token));
        }
    }

    serverLog(LL_VERBOSE, "Vector deserialize finished");
	return C_OK;
}

void CheckVectorsds(Vector *v){

	size_t size= vectorCount(v);
	for(size_t i =0; i< size; i++){
		serverLog(LL_VERBOSE, "VECTOR CHECK SDS : [i : %zu, value : %s]", i, vectorGet(v, i));
	}

}
