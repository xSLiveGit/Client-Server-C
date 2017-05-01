#ifndef _MY_HEAP_H_
#define _MY_HEAP_H_
#include "Status.h"
#include "DynamicVector.h"
#include <stdlib.h>
typedef struct _HEAP {
	PDYNAMIC_VECTOR vector;
	int size;
	int(*Compare)(LPVOID item1, LPVOID item2);
	STATUS(*Add)(struct _HEAP* heap, LPVOID value);
	STATUS(*Delete)(struct _HEAP* heap, LPVOID* value);
	STATUS(*GetTop)(struct _HEAP* heap,LPVOID* value);
	STATUS(*GetSize)(struct _HEAP* heap, int* size);
} MY_HEAP, *PMY_HEAP;

STATUS MyHeapCreate(PMY_HEAP* heap, int(*Compare)(LPVOID item1, LPVOID item2));

STATUS MyHeapDestroy(PMY_HEAP *heap);

STATUS MyHeapAdd(MY_HEAP *heap, LPVOID value);

STATUS MyHeapDelete(MY_HEAP *heap, LPVOID* returned);

#endif // !_HEAP_H_

