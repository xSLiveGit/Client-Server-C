#include <stdio.h>
#include <conio.h>
#include "MyHeap.h"
#include "Threadpool.h"
int Compare(LPVOID item1,LPVOID item2)
{
	int item_1 = *((int*)item1);
	int item_2 = *((int*)item2);
	return item_1 - item_2;
}

STATUS mw (LPVOID value)
{
	int nr = *((int*)value);
	printf_s("Numaraul afisat este: %d\n", nr);
	return SUCCESS;
}

int main()
{
//	PMY_BLOCKING_QUEUE blockingQueue;
	int maxim = 1000;
	STATUS status;
	int vector[1000];
	int i;
	PTHREAD_POOL threadPool;
	LPVOID val;
	status = CreateThreadPool(&threadPool, mw);
	
	threadPool->Start(threadPool, 3);
	for (i = 0; i < maxim;i++)
	{
		vector[i] = i;
		val = (LPVOID)(&(vector[i]));
		threadPool->Add(threadPool, val);
	}
	Sleep(20000);
	DestroyThreadPool(&threadPool);
//	status = CreateMyBlockingQueue(&blockingQueue);
//	int i;
//	for (i = 0; i < 4;i++)
//	{
//		LPVOID value = (LPVOID)(&i);
//		blockingQueue->BlockingQueueAdd(blockingQueue, value);
//	}
//	printf_s("%d", sizeof(DWORD));
//	for (int j = 0; j < 4;j++)
//	{
//		int val;
//		LPVOID value;
//		status = blockingQueue->Take(blockingQueue,&value);
//		val = *((int*)value);
//		printf("%d ", val);
//	}
//	PMY_HEAP heap = (PMY_HEAP)malloc(sizeof(MY_HEAP));
//	MyHeapCreate(&heap, &Compare);
//	LPVOID value;
//	int* v[10];
//	int extractedNr;
//	printf_s("\n");
//
//	for (i = 0; i < 10;i++)
//	{
//		v[i] = (int*)malloc(sizeof(int));
//		*(v[i]) = i % 3;
//		printf_s("Afost adaugat: %d\n", *(v[i]));
//		value = (LPVOID)v[i];
//		heap->Add(heap,value);
//	}
//	printf_s("\n");
//
//	status = heap->GetTop(heap, &value);
//	extractedNr = *((int*)value);
//	printf("Nr de sus este: %d\n", extractedNr);
//	for (i = 0; i < 10; i++)
//	{
//		status = heap->Delete(heap, &value);
//		extractedNr = *((int*)value);
//		printf("Nr de sus este: %d\n", extractedNr);
//		free(value);
//	}
	_getch();
	return 0;
}
