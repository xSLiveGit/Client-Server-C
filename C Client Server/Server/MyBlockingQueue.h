#ifndef _MY_BLOCKING_QUEUE_H_
#define _MY_BLOCKING_QUEUE_H_

#include <Windows.h>
#include "Status.h"

typedef struct _Node
{
	struct _Node *next;
	LPVOID value;
}NODE, *PNODE;

typedef struct _MY_BLOCKING_QUEUE
{
	PNODE head;
	PNODE tail;
	DWORD size;
	STATUS(*Take)(struct _MY_BLOCKING_QUEUE *thisQueue, LPVOID *value);
	STATUS(*Add) (struct _MY_BLOCKING_QUEUE *thisQueue, LPVOID value);
	CRITICAL_SECTION criticalSection;
}MY_BLOCKING_QUEUE, *PMY_BLOCKING_QUEUE;

STATUS CreateMyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue);
STATUS DestroyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue);

#endif//!_MY_BLOCKING_QUEUE_H_
