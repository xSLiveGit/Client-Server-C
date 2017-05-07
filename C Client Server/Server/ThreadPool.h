#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "MyBlockingQueue.h"

typedef struct _THREAD_POOL {
	PMY_BLOCKING_QUEUE queue;
	INT nWorkers;
	HANDLE* threadsHandle;

	STATUS(*ProcessElement)(
		_In_ LPVOID elementToProcess);

	STATUS(*Start)(
		_In_ struct _THREAD_POOL *threadPool, 
		_In_ INT nWorkers);

	STATUS(*Add)(
		_In_ struct _THREAD_POOL *threadPool,
		_In_ LPVOID value);
} THREAD_POOL, *PTHREAD_POOL;

/**
* @brief: create threadpool
* @see: all workers threads will be created here and they will run until you call the destroy function
* @params:
*			-	_Inout_		PTHREAD_POOL*	threadPool
*			-	_In_		STATUS(*ProcessElement)( _In_ LPVOID elementToProcess) - function's will be invoke by worker
*/
STATUS CreateThreadPool(_Inout_ PTHREAD_POOL *threadPool, _In_ STATUS(*ProcessElement)(_In_ LPVOID elementToProcess));

/**
* @brief: destroy threadpool
* @see: all workers threads will be close at this step
* @params:
*			-	_Inout_		PTHREAD_POOL*	threadPool
*
*/
STATUS DestroyThreadPool(_Inout_ PTHREAD_POOL *threadPool);

#endif// !_THREAD_POOL_H_