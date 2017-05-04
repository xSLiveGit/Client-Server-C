#include "ThreadPool.h"
#include <stdio.h>
STATUS CreateThreadPool(PTHREAD_POOL *threadPool, STATUS(*ProcessElement)(_In_ LPVOID elementToProcess));
STATUS DestroyThreadPool(PTHREAD_POOL *threadPool);
STATUS Start(PTHREAD_POOL threadPool, INT nWorkers);
DWORD WINAPI Worker(LPVOID params);
STATUS Add(PTHREAD_POOL threadPool, LPVOID value);
typedef struct _PARAMS_
{
	PMY_BLOCKING_QUEUE pBlockingQueue;
	STATUS(*ProcessElement)(_In_ LPVOID elementToProcess);
} PARAMS_THREAD_POOL;

/**
* @brief: create threadpool
* @see: all workers threads will be created here and they will run until you call the destroy function
* @params:
*			-	_Inout_		PTHREAD_POOL*	threadPool
*			-	_In_		STATUS(*ProcessElement)( _In_ LPVOID elementToProcess) - function's will be invoke by worker
*/
STATUS CreateThreadPool(PTHREAD_POOL *threadPool, STATUS(*ProcessElement)(_In_ LPVOID elementToProcess))
{
	STATUS status;
	PTHREAD_POOL _threadPool;

	status = SUCCESS;
	_threadPool = NULL;

	if (NULL == threadPool)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_threadPool = (PTHREAD_POOL)malloc( sizeof(THREAD_POOL));
	if (NULL == _threadPool)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	status = CreateMyBlockingQueue(&(_threadPool->queue));
	if (SUCCESS != status)
	{
		free(_threadPool);
		_threadPool = NULL;
		goto Exit;
	}
	_threadPool->ProcessElement = ProcessElement;
	_threadPool->Start = &Start;
	_threadPool->Add = &Add;
	_threadPool->nWorkers = 0;
	_threadPool->threadsHandle = NULL;
Exit:
	if (NULL != threadPool)
	{
		*threadPool = _threadPool;
	}
	return status;
}

STATUS Add(PTHREAD_POOL threadPool, LPVOID value)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == threadPool)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status = threadPool->queue->Add(threadPool->queue, value);
Exit:
	return status;
}

/**
* @brief: destroy threadpool
* @see: all workers threads will be close at this step
* @params:
*			-	_Inout_		PTHREAD_POOL*	threadPool
*
*/
STATUS
DestroyThreadPool(PTHREAD_POOL *threadPool)
{
	STATUS status;
	PTHREAD_POOL _threadPool;
	STATUS tempStatus;

	status = SUCCESS;
	_threadPool = NULL;
	tempStatus = SUCCESS;
	if ((NULL == threadPool) || (NULL == *threadPool))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_threadPool = *threadPool;
	for (int i = 0; i < _threadPool->nWorkers; i++)
	{

		TerminateThread(_threadPool->threadsHandle[i], tempStatus);
		_threadPool->threadsHandle[i] = INVALID_HANDLE_VALUE;
	}
	DestroyBlockingQueue(&(_threadPool->queue));
	free(_threadPool->threadsHandle);
	_threadPool->threadsHandle = NULL;
	free(_threadPool);
	_threadPool->threadsHandle = NULL;
Exit:
	if (SUCCESS == status)
	{
		*threadPool = NULL;
	}
	return status;
}



STATUS
Start(PTHREAD_POOL threadPool, INT nWorkers)
{
	STATUS status;
	INT iThread;
	INT iThreadErr;
	PARAMS_THREAD_POOL *params;

	status = SUCCESS;
	iThread = 0;
	iThreadErr = 0;

	if (NULL == threadPool)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	threadPool->threadsHandle = (HANDLE*)malloc( nWorkers * sizeof(HANDLE));
	if (NULL == threadPool->threadsHandle)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	threadPool->nWorkers = nWorkers;


	for (iThread = 0; iThread < nWorkers; ++iThread)
	{
		params = (PARAMS_THREAD_POOL*)malloc( sizeof(PARAMS_THREAD_POOL));
		params->pBlockingQueue = threadPool->queue;
		params->ProcessElement = threadPool->ProcessElement;
		threadPool->threadsHandle[iThread] = CreateThread(
			NULL,				// no security attribute 
			0,					// default stack size 
			Worker,				// thread proc
			(LPVOID)(params),	// thread parameter 
			0,					// not suspended 
			NULL				// returns thread ID
			);
		if (INVALID_HANDLE_VALUE == threadPool->threadsHandle[iThread])
		{
			for (iThreadErr = 0; iThreadErr < iThread; ++iThreadErr)
			{
				TerminateThread(threadPool->threadsHandle[iThreadErr], 0);
			}
			status = THREAD_ERROR;
			goto Exit;
		}
	}
	//	Sleep(100);
Exit:
	return status;
}

DWORD WINAPI
Worker(LPVOID params)
{
	DWORD status;
	PMY_BLOCKING_QUEUE pBlockingQueue;
	LPVOID value;
	PARAMS_THREAD_POOL parameter;

	status = SUCCESS;
	value = NULL;
	pBlockingQueue = NULL;


	parameter.pBlockingQueue = ((PARAMS_THREAD_POOL*)params)->pBlockingQueue;
	parameter.ProcessElement = ((PARAMS_THREAD_POOL*)params)->ProcessElement;
	pBlockingQueue = parameter.pBlockingQueue;
	free((PARAMS_THREAD_POOL*)params);
	printf_s("Pblocking queue este: %p\n", parameter.pBlockingQueue);
	while (TRUE)
	{
		status = pBlockingQueue->Take(pBlockingQueue, &value);
		if (SUCCESS != status)
		{
			goto Exit;
		}
		parameter.ProcessElement(value);
	}
Exit:
	ExitThread(status);
}