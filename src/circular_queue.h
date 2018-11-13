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

#define DEFAULT_ARRAY_QUEUE_SIZE 100000
#define DEFAULT_FREE_QUEUE_SIZE 1000

typedef struct _arrayQueue {
    int32_t rear;
    int32_t front;
    int32_t max;
    int32_t key_offset;
    int32_t size;
    dictEntry **buf;
} Queue;

enum {
	EXTEND,
	COMPLETED
};

Queue *createArrayQueue(int32_t capacity);
//void enqueue(Queue *queue, dictEntry *entry);
int enqueue(Queue *queue, dictEntry *entry);
void *dequeue(Queue *queue);
void *dequeueForClear(Queue *queue);
void *chooseBestKeyFromQueue(Queue *queue);
void *chooseBestKeyFromQueue_(Queue *queue, Queue *freequeue);
void *chooseClearKeyFromQueue_(Queue *queue);
int checkQueueFordeQueue(int dbnum, Queue *queue);
//int checkQueueFordeQueue(Queue *queue);
void *forceDequeue(Queue *queue);


//dictEntry *dequeue(Queue *queue);
int isEmpty(Queue *queue);
void initializeQueue(Queue *queue);


#endif /* SRC_CIRCULAR_QUEUE_H_ */
