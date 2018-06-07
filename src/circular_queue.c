/*
 * circular_queue.c
 *
 *  Created on: 2017. 11. 15.
 *      Author: canpc815
 */

#include "server.h"
#include "dict.h"
#include "circular_queue.h"

#define QUEUE_MAX INT32_MAX-1
Queue *createArrayQueue() {
    Queue *queue = (Queue *)zmalloc(sizeof(Queue));
    queue->rear = 0;
    queue->front = 0;
    queue->max = DEFAULT_ARRAY_QUEUE_SIZE;
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
		        queue->rear = 0;
		        queue->front = queue->max - 1;
		        queue->max = newCapacity;
		        serverLog(LL_DEBUG, "[EXTEND AFTER] queue->rear : %d, queue->front : %d, queue->max : %d",
		                                queue->rear, queue->front, queue->max);
		    }

		    queue->buf[queue->front] = entry;

		    queue->front = (queue->front + 1) % queue->max;
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
        serverLog(LL_DEBUG, "queue->rear : %d, queue->front : %d, queue->max : %d",
                queue->rear, queue->front, queue->max);
        serverAssert(0);
    }
    robj *obj = dictGetVal(retVal);
    serverAssert(obj != NULL);
    //TODO- MODIFY LATER
    if(obj->location != LOCATION_PERSISTED) {
        return NULL;
    }
    queue->buf[queue->rear] = NULL;
    queue->rear = (queue->rear + 1) % queue->max;
    return retVal;
}

int isEmpty(Queue *queue) {
    return queue->rear == queue->front;
}

//TODO - Implement later
void initializeQueue(Queue *queue){
	queue->rear = 0;
	queue->front = 0;
}

void *chooseBestKeyFromQueue(Queue *queue){
	dictEntry *bestEntry = NULL;

	if(queue->front == queue->rear){
		serverLog(LL_DEBUG, "ENQUEUE ENTRY IS NULL");
		return NULL;
	}
	else{
		bestEntry = queue->buf[queue->rear];
		return bestEntry;
	}
}
