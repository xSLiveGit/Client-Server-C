#include "MyBlockingQueue.h"
#include <stdio.h>

STATUS Take(PMY_BLOCKING_QUEUE thisQueue, LPVOID *value);
//STATUS BlockingQueueAdd(PMY_BLOCKING_QUEUE thisQueue, LPVOID value);
//STATUS CreateMyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue);
//STATUS DestroyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue);

STATUS BlockingQueueAdd(PMY_BLOCKING_QUEUE thisQueue, LPVOID value)
{
	STATUS status;
	PNODE node;

	status = SUCCESS;
	node = NULL;

	if (NULL == thisQueue)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	node = (PNODE)malloc(sizeof(NODE));
	if (NULL == node)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	node->next = NULL;
	node->value = value;

	EnterCriticalSection(&thisQueue->criticalSection);
	if (NULL == thisQueue->head)
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

STATUS CreateMyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue)
{
	STATUS status;
	PMY_BLOCKING_QUEUE _blockingQueue;

	_blockingQueue = NULL;
	status = SUCCESS;

	if (NULL == blockingQueue)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_blockingQueue = (PMY_BLOCKING_QUEUE)malloc(sizeof(MY_BLOCKING_QUEUE));
	if (NULL == _blockingQueue)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	_blockingQueue->Add = &BlockingQueueAdd;
	_blockingQueue->Take = Take;
	_blockingQueue->head = NULL;
	_blockingQueue->tail = NULL;
	_blockingQueue->size = 0;
	InitializeCriticalSection(&(_blockingQueue->criticalSection));

Exit:
	*blockingQueue = _blockingQueue;
	return status;
}

STATUS DestroyBlockingQueue(PMY_BLOCKING_QUEUE *blockingQueue)
{
	STATUS status;
	PMY_BLOCKING_QUEUE _blockingQueue;
	status = SUCCESS;

	if ((NULL == blockingQueue) || (NULL == *blockingQueue))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	_blockingQueue = *blockingQueue;

	_blockingQueue->Add = NULL;
	_blockingQueue->Take = NULL;
	_blockingQueue->head = NULL;
	_blockingQueue->tail = NULL;
	_blockingQueue->size = 0;
	DeleteCriticalSection(&(_blockingQueue->criticalSection));
	free(_blockingQueue);
	_blockingQueue = NULL;
Exit:
	*blockingQueue = NULL;
	return status;
}



/***
* !Assume that status != NULL
*
*/
STATUS Take(PMY_BLOCKING_QUEUE thisQueue, LPVOID *value)
{
	PNODE tempNode;
	STATUS status;
	DWORD timeToStay = 10;

	tempNode = NULL;
	status = SUCCESS;

	if ((NULL == thisQueue) || (NULL == value))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	while (TRUE)
	{
		EnterCriticalSection(&thisQueue->criticalSection);
		if (thisQueue->size > 0)
		{
			timeToStay = 10;
			tempNode = thisQueue->head;
			thisQueue->head = thisQueue->head->next;
			thisQueue->size--;
			if (0 == thisQueue->size)
			{
				thisQueue->tail = NULL;//thisQuete is allready NULL from thisQueue->head = thisQueue->head->next;
			}
			LeaveCriticalSection(&thisQueue->criticalSection);
			*value = tempNode->value;
			free(tempNode);
			goto Exit;
		}
		else
		{
			if (timeToStay < 500)
			{
				timeToStay += 5;
			}
			LeaveCriticalSection(&thisQueue->criticalSection);
			printf_s("Am stat odata\n");
			Sleep(timeToStay);
		}
	}

Exit:
	return status;
}