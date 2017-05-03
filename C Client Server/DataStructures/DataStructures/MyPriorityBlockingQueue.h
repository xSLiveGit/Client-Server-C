#ifndef _MY_PRIORITY_BLOCKING_QUEUE_H_
#define _MY_PRIORITY_BLOCKING_QUEUE_H_
#include "MyHeap.h"

typedef struct _MY_PRIORITY_BLOCKING_QUEUE
{
	MY_HEAP heap;
	CRITICAL_SECTION criticalSection;
	int size;
	int(*Compare)(LPVOID item1, LPVOID item2);
	STATUS(*Take)(struct _MY_PRIORITY_BLOCKING_QUEUE *priorityQueue, LPVOID *returnedVal);
	STATUS(*Add)(struct _MY_PRIORITY_BLOCKING_QUEUE *priorityQueue, LPVOID value);
	
} MY_PRIORITY_QUEUE,*PMY_PRIORITY_QUEUE;

STATUS CreatePriorityBlockingQueue(PMY_PRIORITY_QUEUE *priorityQueue);
STATUS RemovePriorityBlockingQueue(PMY_PRIORITY_QUEUE *priorityQueue);

#endif //!_MY_PRIORITY_BLOCKING_QUEUE_H_