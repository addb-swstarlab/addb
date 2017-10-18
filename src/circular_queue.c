/*
 * circular_queue.c
 *
 *  Created on: 2017. 11. 15.
 *      Author: canpc815
 */
#include "circular_queue.h"

Queue *createArrayQueue() {
    Queue *queue = (Queue *)zmalloc(sizeof(Queue));
    queue->rear = 0;
    queue->front = 0;
    queue->max = DEFAULT_ARRAY_QUEUE_SIZE;
    queue->buf = (dictEntry **)zmalloc(sizeof(dictEntry *) * DEFAULT_ARRAY_QUEUE_SIZE);
    return queue;
}

void enqueue(Queue *queue, dictEntry *entry) {
    /* If it is full, extend queue size */
    if(((queue->front+1)%queue->max) == queue->rear) {
        queue->buf =
                (dictEntry **)zrealloc(queue->buf,
                        sizeof(dictEntry *) * queue->max * 2);
        queue->rear = 0;
        queue->front = queue->max;
        queue->max = queue->max * 2;
    }

    queue->buf[queue->front] = entry;

    queue->front = (queue->front + 1) % queue->max;
}

dictEntry *dequeue(Queue *queue) {
    dictEntry *retVal = NULL;
    /* If it is empty, return null */
    if(queue->rear == queue->front) {
        return NULL;
    }
    retVal = queue->buf[queue->rear];
    robj *obj = dictGetVal(retVal);
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
