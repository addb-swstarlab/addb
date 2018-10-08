/*
 * circular_queue.c
 *
 *  Created on: 2017. 11. 15.
 *      Author: canpc815
 */

#include "server.h"
#include "dict.h"
#include "circular_queue.h"
#include <assert.h>

#define QUEUE_MAX INT32_MAX-1
Queue *createArrayQueue(int32_t capacity) {
    Queue *queue = (Queue *)zmalloc(sizeof(Queue));
    queue->rear = 0;
    queue->front = 0;
    queue->max = capacity;
    queue->key_offset = 0;
    queue->size = 0;
    queue->buf = (dictEntry **)zmalloc(sizeof(dictEntry *) * queue->max);
    return queue;
}

int enqueue(Queue *queue, dictEntry *entry) {
	int result = 0;
	if(entry == NULL){
		serverLog(LL_DEBUG, "ENQUEUE ENTRY IS NULL");
		return result;
	}
	else {
		serverLog(LL_DEBUG, "ENQUEUE ENTRY IS NOT NULL");

		    /* If it is full, extend queue size */
		    if(((queue->front+1)%queue->max) == queue->rear) {
		        serverLog(LL_DEBUG, "[EXTEND BEFORE] queue->rear : %d, queue->front : %d, queue->max : %d",
		                        queue->rear, queue->front, queue->max);

		        int32_t newCapacity = 0;
		        if(queue->max == QUEUE_MAX) {
		            serverLog(LL_NOTICE, "Could not more extend array.");
		            serverAssert(0);
		        } else if((QUEUE_MAX - queue->max) < queue->max) {
		            newCapacity = QUEUE_MAX;
		        } else {
		            newCapacity = queue->max * 2;
		        }
		        queue->buf =
		                (dictEntry **)zrealloc(queue->buf, sizeof(dictEntry *) * newCapacity);
		        queue->front = queue->max - 1;
		        queue->max = newCapacity;
		        queue->rear = queue->rear % queue->max;

		        serverLog(LL_DEBUG, "[EXTEND AFTER] queue->rear : %d, queue->front : %d, queue->max : %d",
		                                queue->rear, queue->front, queue->max);
		    }

		    queue->buf[queue->front] = entry;

		    queue->front = (queue->front + 1) % queue->max;
		    queue->size++;

		    result = 1;
		    return result;
	}
}

void *dequeue(Queue *queue) {
    dictEntry *retVal = NULL;
    /* If it is empty, return null */
    if(queue->rear == queue->front) {
        return NULL;
    }
    retVal = queue->buf[queue->rear];
    if(retVal == NULL) {
        serverLog(LL_VERBOSE, "dequeue : queue->rear : %d, queue->front : %d, queue->max : %d",
                queue->rear, queue->front, queue->max);
    	return NULL;
    }
    robj *obj = dictGetVal(retVal);
    serverAssert(obj != NULL);

    queue->buf[queue->rear] = NULL;
    queue->rear = (queue->rear + 1) % queue->max;
    queue->size--;
    return retVal;
}

void *dequeueForClear(Queue *queue) {
    dictEntry *retVal = NULL;
    /* If it is empty, return null */
    retVal = queue->buf[queue->rear];
    if(retVal == NULL) {
        serverLog(LL_VERBOSE, "dequeue : queue->rear : %d, queue->front : %d, queue->max : %d",
                queue->rear, queue->front, queue->max);
    	return NULL;
    }
    robj *obj = dictGetVal(retVal);
    serverAssert(obj != NULL);

	if (obj->location == LOCATION_PERSISTED) {
		queue->buf[queue->rear] = NULL;
		queue->rear = (queue->rear + 1) % queue->max;
		queue->size--;
	}
    return retVal;
}

void *forceDequeue(Queue *queue) {
    dictEntry *retVal = NULL;
    /* If it is empty, return null */
    if(queue->rear == queue->front) {
        return NULL;
    }
    retVal = queue->buf[queue->rear];
    if(retVal == NULL) {
        serverLog(LL_DEBUG, "queue->rear : %d, queue->front : %d, queue->max : %d",
                queue->rear, queue->front, queue->max);
        serverAssert(0);
    }
    robj *obj = dictGetVal(retVal);
    serverAssert(obj != NULL);

    queue->buf[queue->rear] = NULL;
    queue->rear = (queue->rear + 1) % queue->max;
    queue->size--;
    return retVal;
}


int isEmpty(Queue *queue) {
    return queue->rear == queue->front;
}

//TODO - Implement later

void initializeQueue(Queue *queue){
	//int start = queue->rear;
	int end = queue->front;

	for(int start = queue->rear; start <= end; start++){
		serverLog(LL_DEBUG, "Rear : %d(start : %d), Front : %d(end : %d)", queue->rear, start, queue->front, end);
		forceDequeue(queue);
	}

	queue->rear = 0;
	queue->key_offset = 0;
	queue->front = 0;
}

void *chooseBestKeyFromQueue(Queue *queue){
	dictEntry *bestEntry = NULL;

	if(queue->front == queue->rear){
		serverLog(LL_DEBUG, "ENQUEUE ENTRY IS NULL");
		return NULL;
	}
	else{

		bestEntry = queue->buf[queue->key_offset];  //queue->rear
		robj *obj = dictGetVal(bestEntry);

		if(obj->location == LOCATION_PERSISTED){
			if(queue->key_offset != queue->front){

				queue->key_offset = (queue->key_offset + 1) % queue->max;
				bestEntry = queue->buf[queue->key_offset];
				obj = dictGetVal(bestEntry);
			}
		}

		if((obj->location != LOCATION_PERSISTED) & (obj->location != LOCATION_REDIS_ONLY)){
			serverLog(LL_WARNING, "Unknown Object Location Information : %d", obj->location);
			serverAssert(0);
		}

		assert(obj->location == LOCATION_REDIS_ONLY);
		return bestEntry;

	}
}

int checkQueueFordeQueue(int dbnum, Queue *queue){

	dictEntry *de = NULL;
	if(queue->rear == queue->front){
		return 0;
	}
	serverLog(LL_VERBOSE, "[QUEUE Rear : %d, Key_offset : %d, Front : %d, QUEUE_SIZE : %d]",
			queue->rear, queue->key_offset, queue->front, queue->max);

	if(queue->rear == queue->key_offset){

		de = queue->buf[queue->key_offset];
		robj *val = dictGetVal(de);

		if(val->location == LOCATION_REDIS_ONLY){
			serverLog(LL_VERBOSE, "No Entry to dequeue");
			return 2;
		}

		if(val->location == LOCATION_PERSISTED){
			queue->key_offset = (queue->key_offset + 1) % queue->max;
			de = dequeue(queue);

			sds dequeuekey = dictGetKey(de);
			robj *keyobj = createStringObject(dequeuekey,sdslen(dequeuekey));
			redisDb *db = server.db + dbnum;

			int Flushed = dbPersistOrClear(db, keyobj);
			if(!Flushed){
				serverAssert(0);
			}
			return 1;
		}
	}
	else {
		if(queue->rear != queue->key_offset){
			serverLog(LL_VERBOSE, "Dequeue Multiple Entries");
			dictEntry *dequeueEntry = NULL;
			int isFlushed = 0;
			while(queue->rear != queue->key_offset){
				if(queue->rear != queue->key_offset){
					dequeueEntry = dequeue(queue);
					sds key = dictGetKey(dequeueEntry);
					robj *hashval = dictGetVal(dequeueEntry);
					robj *keyobj = createStringObject(key,sdslen(key));
					redisDb *db = server.db + dbnum;
					serverLog(LL_VERBOSE, "DEQUEUEENTRY DB NUM : %d", dbnum);
					serverLog(LL_VERBOSE, "[DB: %d]DEQUEUEENTRY KEY : %s [Front : %d, Key_Offset : %d, Rear : %d]",
							dbnum, key, queue->front, queue->key_offset, queue->rear);
					isFlushed = dbPersistOrClear(db, keyobj);
				}
				else {
					serverLog(LL_VERBOSE, "Break!!! [QUEUE Rear : %d, Key_offset : %d, Front : %d]",
							queue->rear, queue->key_offset, queue->front);
					break;
				}
			}
		}
		else {
			serverLog(LL_WARNING, "checkQueueFordeQueue Error");
			serverAssert(0);
		}
	}
	return 1;
}

void *chooseClearKeyFromQueue_(Queue *queue) {
	dictEntry *clearEntry = NULL;

   if (isEmpty(queue)){
		serverLog(LL_DEBUG,"[chooseClearKeyFromQueue_] : NO KEY TO FREE");
		return NULL;
	} else {
		clearEntry = dequeueForClear(queue);
		if (clearEntry == NULL) {
			serverLog(LL_DEBUG,"[chooseClearKeyFromQueue_] : FREE QUEUE HAS NULL");
			/* Because of Queue extension, there can exists null entry */
			return NULL;
		}
		robj * clearobj = dictGetVal(clearEntry);
		if (clearobj->location == LOCATION_REDIS_ONLY ) {
			serverAssert(0);
		} else if ( clearobj->location == LOCATION_FLUSH ) {
			serverLog(LL_DEBUG,"[chooseClearKeyFromQueue_] : %s  [%d]: WAIT ROCKSDB FLUSH", dictGetKey(clearEntry), queue->rear);
			return NULL;
		} else if ( clearobj->location == LOCATION_PERSISTED ) {
					/* determine victim to clean data structure to RocksDB,
			 * because LOCATION_PERSISTED shows that tiering is end
			 */
			serverLog(LL_DEBUG, "[chooseClearKeyFromQueue_] : CLEAR CANDIDATE : %s,  rear : %d",
					dictGetKey(clearEntry), queue->rear);
			return clearEntry;
		}
	}
	return NULL;
}

void *chooseBestKeyFromQueue_(Queue *queue, Queue *freequeue) {
	dictEntry *bestEntryCandidate = NULL;
	int result = 0;
	serverLog(LL_DEBUG, "[chooseBestKeyFromQueue_] : front : %d, rear : %d, size : %d",
			queue->front, queue->rear, queue->max);

	if(isEmpty(queue)) {
		/* no Entry to Clear */
		return NULL;
	} else {
		bestEntryCandidate = dequeue(queue);
		serverLog(LL_DEBUG, "[chooseBestKeyFromQueue_] : EVICT KEY CANDIDATE : %s" , dictGetKey(bestEntryCandidate));

		robj * obj = dictGetVal(bestEntryCandidate);
		if (obj->location == LOCATION_PERSISTED) {
			serverAssert(0);
		}
		obj->location = LOCATION_FLUSH;
		result = enqueue(freequeue, bestEntryCandidate);
		return bestEntryCandidate;
	}
	serverAssert(0);
	return NULL;
}
