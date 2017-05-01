#include "MyBlockingQueue.h"
#include <stdio.h>
#include <conio.h>

#include "MyHeap.h"

int Compare(LPVOID item1,LPVOID item2)
{
	int item_1 = *((int*)item1);
	int item_2 = *((int*)item2);
	return item_1 - item_2;
}


int main()
{
	BLOCKING_QUEUE blockingQueue;
	STATUS status;
	status = CreateBlockingQueue(&blockingQueue);
	int i;
	for (i = 0; i < 4;i++)
	{
		LPVOID value = (LPVOID)(&i);
		blockingQueue.Add(&blockingQueue, value);
	}
	printf_s("%d", sizeof(DWORD));
	for (int j = 0; j < 4;j++)
	{
		LPVOID value = blockingQueue.Take(&blockingQueue, &status);
		int val = *((int*)value);
		printf("%d ", val);
	}
	PMY_HEAP heap = (PMY_HEAP)malloc(sizeof(MY_HEAP));
	MyHeapCreate(&heap, &Compare);
	LPVOID value;
	int* v[10];
	int extractedNr;
	printf_s("\n");

	for (i = 0; i < 10;i++)
	{
		v[i] = (int*)malloc(sizeof(int));
		*(v[i]) = i % 3;
		printf_s("Afost adaugat: %d\n", *(v[i]));
		value = (LPVOID)v[i];
		heap->Add(heap,value);
	}
	printf_s("\n");

	status = heap->GetTop(heap, &value);
	extractedNr = *((int*)value);
	printf("Nr de sus este: %d\n", extractedNr);
	for (i = 0; i < 10; i++)
	{
		status = heap->Delete(heap, &value);
		extractedNr = *((int*)value);
		printf("Nr de sus este: %d\n", extractedNr);
		free(value);
	}
	_getch();
	return 0;
}
