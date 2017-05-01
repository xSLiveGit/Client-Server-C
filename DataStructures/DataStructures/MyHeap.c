#include "MyHeap.h"

STATUS MyHeapCreate(PMY_HEAP* heap, int(*Compare)(LPVOID item1, LPVOID item2));
STATUS MyHeapDestroy(PMY_HEAP *heap);
STATUS MyHeapAdd(MY_HEAP *heap, LPVOID value);
STATUS MyHeapDelete(MY_HEAP *heap, LPVOID* returned);
STATUS GetTop (PMY_HEAP heap, LPVOID* value);
STATUS GetSize(PMY_HEAP heap, int* size);


STATUS GetTop(PMY_HEAP heap, LPVOID* value)
{
	STATUS status;
	
	status = SUCCESS;

	if(NULL == heap || NULL == value)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	if(heap->size > 0)
	{
		status = VectorGet(*(heap->vector), 0, value);
		goto Exit;
	}
	status = INDEX_OUT_OF_BOUNDS;
Exit:
	return status;
}

STATUS GetSize(PMY_HEAP heap, int* size)
{
	STATUS status;

	status = SUCCESS;

	if(NULL == heap || NULL == size)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	*size = heap->size;
Exit:
	return status;
}

STATUS MyHeapCreate(PMY_HEAP* heap, int(* Compare)(LPVOID item1, LPVOID item2))
{
	STATUS statusCode;
	statusCode = SUCCESS;

	if (NULL == heap)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	*heap = (PMY_HEAP)malloc(sizeof(MY_HEAP));
	if (NULL == *heap)
	{
		statusCode = MALLOC_FAILED_ERROR;
		goto CleanUp;
	}
	statusCode = VectorCreate(&((*heap)->vector));
	if (SUCCESS != statusCode)
	{
		goto CleanUp;
	}
	(*heap)->size = 0;
	(*heap)->Compare = Compare;
	(*heap)->Add = &MyHeapAdd;
	(*heap)->Delete = &MyHeapDelete;
	(*heap)->GetSize = &GetSize;
	(*heap)->GetTop = &GetTop;
CleanUp:
	if (SUCCESS != statusCode)
	{
		if (*heap != NULL)
		{
			free((*heap)->vector);
			(*heap)->vector = NULL;
		}
		free(*heap);
		*heap = NULL;
	}
	return statusCode;
}

STATUS MyHeapDestroy(PMY_HEAP * heap)
{
	STATUS statusCode;

	statusCode = SUCCESS;
	if (NULL == heap)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}
	if (NULL != (*heap)->vector)
	{
		statusCode = VectorDestroy(&(*heap)->vector);
		(*heap)->vector = NULL;
	}
CleanUp:
	return statusCode;
}

inline int RightSon(int poz)
{
	return (2 * poz + 2);
}

inline int LeftSon(int poz)
{
	return (2 * poz + 1);
}

inline int Father(int poz)
{
	return ((poz - 1) / 2);
}

STATUS Up(MY_HEAP *heap, int poz)
{
	LPVOID key;
	int father;
	LPVOID fatherEl;

	STATUS statusCode = SUCCESS;


	if (NULL == heap)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}
	if (poz < 0 || poz >= heap->vector->size)
	{
		statusCode = INDEX_OUT_OF_BOUNDS;
		goto CleanUp;
	}

	father = Father(poz);
	VectorGet(*heap->vector, poz, &key);
	VectorGet(*heap->vector, father, &fatherEl);

	while ((poz > 0) && heap->Compare(fatherEl,key) > 0) {
		heap->vector->v[poz] = fatherEl;
		poz = father;
		father = Father(poz);
		VectorGet(*(heap->vector), father, &fatherEl);
	}

	heap->vector->v[poz] = key;

CleanUp:
	return statusCode;
}

STATUS Down(MY_HEAP *heap, int k)
{
	STATUS statusCode;
	LPVOID key;
	int size;
	int leftSonPosition;
	int rightSonPosition;
	int minSonPosition;
	LPVOID temp;
	LPVOID leftSonElement;
	LPVOID rightSonElement;
	LPVOID minSonElement;
	LPVOID currentElement;
	statusCode = SUCCESS;

	if (NULL == heap)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	size = VectorLength(*(heap->vector));
	VectorGet(*heap->vector, k, &key);

	leftSonPosition = LeftSon(k);
	rightSonPosition = RightSon(k);
	minSonPosition = leftSonPosition;
	VectorGet(*heap->vector, leftSonPosition, &leftSonElement);
	VectorGet(*heap->vector, rightSonPosition, &rightSonElement);

	do {
		minSonPosition = -1;
		leftSonPosition = LeftSon(k);
		rightSonPosition = RightSon(k);
		VectorGet(*(heap->vector), leftSonPosition, &leftSonElement);
		VectorGet(*(heap->vector), rightSonPosition, &rightSonElement);
		// Alege un fiu mai mic ca tatal.
		if (leftSonPosition < heap->vector->size) {
			if (rightSonPosition <= heap->vector->size && heap->Compare(rightSonElement,leftSonElement) < 0)
			{
				minSonPosition = rightSonPosition;
			}
			else
			{
				minSonPosition = leftSonPosition;
			}
			VectorGet(*(heap->vector), minSonPosition, &minSonElement);
			VectorGet(*(heap->vector), k, &currentElement);
			if (heap->Compare(minSonElement , currentElement) > 0) {
				minSonPosition = -1;
			}
		}
		if (minSonPosition != -1) {
			temp = heap->vector->v[minSonPosition];
			heap->vector->v[minSonPosition] = heap->vector->v[k];
			heap->vector->v[k] = temp;
			k = minSonPosition;
		}
	} while (minSonPosition != -1);

CleanUp:
	return statusCode;
}

STATUS MyHeapAdd(MY_HEAP *heap, LPVOID value)
{
	STATUS statusCode;

	statusCode = SUCCESS;

	statusCode = VectorAdd(heap->vector, value);

	if (SUCCESS != statusCode)
	{
		goto CleanUp;
	}

	statusCode = Up(heap, heap->vector->size - 1);
	heap->size++;
CleanUp:
	return statusCode;
}

STATUS MyHeapDelete(MY_HEAP * heap, LPVOID* returned)
{
	STATUS statusCode;

	statusCode = SUCCESS;

	if (NULL == heap)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	if (0 == heap->size)
	{
		statusCode = ELEMENT_NOT_FOUND;
		goto CleanUp;
	}

	if (heap->size >= 1)
	{
		VectorGet(*(heap->vector), 0, returned);
		heap->vector->v[0] = heap->vector->v[heap->vector->size - 1];
		heap->vector->size--;
		Down(heap, 0);

		heap->size--;
	}
	else {//
		statusCode = INDEX_OUT_OF_BOUNDS;
	}
CleanUp:
	return statusCode;
}



