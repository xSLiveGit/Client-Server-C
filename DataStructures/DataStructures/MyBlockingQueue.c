#include "MyBlockingQueue.h"
#include <stdio.h>

LPVOID Take(PBLOCKING_QUEUE thisQueue, STATUS *status);
STATUS Add (PBLOCKING_QUEUE thisQueue, LPVOID value);
STATUS CreateBlockingQueue(PBLOCKING_QUEUE blockingQueue);
STATUS DestroyBlockingQueue(PBLOCKING_QUEUE blockingQueue);

STATUS CreateBlockingQueue(PBLOCKING_QUEUE blockingQueue)
{
	STATUS status ;

	status = SUCCESS;

	if(NULL == blockingQueue)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	blockingQueue->Add = &Add;
	blockingQueue->Take = &Take;
	blockingQueue->head = NULL;
	blockingQueue->tail = NULL;
	blockingQueue->size = 0;
	InitializeCriticalSection(&(blockingQueue->criticalSection));

Exit:
	return status;
}

STATUS DestroyBlockingQueue(PBLOCKING_QUEUE blockingQueue)
{
	STATUS status;

	status = SUCCESS;

	if(NULL ==  blockingQueue)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	blockingQueue->Add = NULL;
	blockingQueue->Take = NULL;
	blockingQueue->head = NULL;
	blockingQueue->tail = NULL;
	blockingQueue->size = 0;
	DeleteCriticalSection(&blockingQueue->criticalSection);
Exit:
	return status;
}

STATUS Add(PBLOCKING_QUEUE thisQueue, LPVOID value)
{
	STATUS status;
	PNODE node;
	
	status = SUCCESS;
	node = NULL;
	
	if(NULL == thisQueue)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	
	node = (PNODE)malloc(sizeof(NODE));
	if(NULL == node)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	node->next = NULL;
	node->value = value;

	EnterCriticalSection(&thisQueue->criticalSection);
	if(NULL == thisQueue->head)
	{
		thisQueue->head = node;
		thisQueue->tail = node;
		thisQueue->size = 1;
	}
	else
	{
		thisQueue->tail->next = node;
		thisQueue->size++;
		thisQueue->tail = node;
	}
	LeaveCriticalSection(&thisQueue->criticalSection);

//	free(node);

Exit:
	return status;
}

/***
 * !Assume that status != NULL
 *
 */
LPVOID Take(PBLOCKING_QUEUE thisQueue, STATUS *status)
{
	PNODE tempNode;
	LPVOID value;

	value = NULL;
	tempNode = NULL;
	status = SUCCESS;

	if(NULL == thisQueue)
	{
		*status = NULL_POINTER_ERROR;
		goto Exit;
	}

	while (TRUE)
	{
		EnterCriticalSection(&thisQueue->criticalSection);
		if(thisQueue->size > 0)
		{
			tempNode = thisQueue->head;
			thisQueue->head = thisQueue->head->next;
			thisQueue->size--;
			if(0 == thisQueue->size)
			{
				thisQueue->tail = NULL;//thisQuete is allready NULL from thisQueue->head = thisQueue->head->next;
			}
			LeaveCriticalSection(&thisQueue->criticalSection);
			value = tempNode->value;
			free(tempNode);
			return value;
			goto Exit;
		}
		else
		{
			LeaveCriticalSection(&thisQueue->criticalSection);
			printf_s("Am stat odata\n");
			Sleep(250);
		}
	}

Exit:
	return status;
}