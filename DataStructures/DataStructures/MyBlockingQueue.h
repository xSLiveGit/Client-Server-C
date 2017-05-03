#ifndef _MY_BLOCKING_QUEUE_H_

#define _MY_BLOCKING_QUEUE_H_


#include <Windows.h>
#include "Status.h"
typedef struct _Node
{
	struct _Node *next;
	LPVOID value;
}NODE,*PNODE;

typedef struct _BLOCKING_QUEUE
{
	PNODE head;
	PNODE tail;
	DWORD size;
	STATUS (*Take)(struct _BLOCKING_QUEUE *thisQueue, LPVOID *status);
	STATUS (*Add) (struct _BLOCKING_QUEUE *thisQueue, LPVOID value);
	CRITICAL_SECTION criticalSection;
}BLOCKING_QUEUE,*PBLOCKING_QUEUE;

STATUS CreateBlockingQueue(PBLOCKING_QUEUE blockingQueue);
#endif//!_MY_BLOCKING_QUEUE_H_
