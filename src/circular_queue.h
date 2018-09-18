/*
 * circular_queue.h
 *
 *  Created on: 2017. 11. 15.
 *      Author: canpc815
 */

#ifndef SRC_CIRCULAR_QUEUE_H_
#define SRC_CIRCULAR_QUEUE_H_

#include "dict.h"
#include <stdlib.h>

#define DEFAULT_ARRAY_QUEUE_SIZE 4

typedef struct _arrayQueue {
    int32_t rear;
    int32_t front;
    int32_t max;
    int32_t key_offset;
    int32_t size;
    int32_t extend;
    dictEntry **buf;
} Queue;

Queue *createArrayQueue();
//void enqueue(Queue *queue, dictEntry *entry);
int enqueue(Queue *queue, dictEntry *entry);
void *dequeue(Queue *queue);
void *dequeueForFlush(Queue *queue);
void *chooseBestKeyFromQueue(Queue *queue);
void *chooseBestKeyFromQueue_(Queue *queue);
void *chooseClearKeyFromQueue_(Queue *queue);
int checkQueueFordeQueue(int dbnum, Queue *queue);
//int checkQueueFordeQueue(Queue *queue);
void *forceDequeue(Queue *queue);


//dictEntry *dequeue(Queue *queue);
int isEmpty(Queue *queue);
void initializeQueue(Queue *queue);


#endif /* SRC_CIRCULAR_QUEUE_H_ */
