
#ifndef _DYNAMICVECTOR_H_
#define _DYNAMICVECTOR_H_
#include "Status.h"
typedef struct {
	LPVOID *v;
	int size;
	int capacity;
} DYNAMIC_VECTOR, *PDYNAMIC_VECTOR;

/**
*Returns STATUS_CODE: MALLOC_ERROR
SUCCES
*/
STATUS VectorCreate(PDYNAMIC_VECTOR *dynamicVector);

/**
*Returns STATUS_CODE: INVALID_VECTOR_POINTER
INVALID_PVECTOR_POINTER
SUCCES
*/
STATUS VectorDestroy(PDYNAMIC_VECTOR *, STATUS(*DestroyValue)(LPVOID val));

/**
*Returns STATUS_CODE : SUCCES
INVALID_VECTOR_POINTER
*/
STATUS VectorAdd(PDYNAMIC_VECTOR, LPVOID);

/**
*Returns INT: size of vector
*/
STATUS VectorLength(DYNAMIC_VECTOR);

/**
* Returns STATUS_CODE: INDEX_OUT_OF_BOUNDS
SUCCES
INVALID_VECTOR_POINTER
*/
STATUS VectorRemovePosition(PDYNAMIC_VECTOR, int);

/**
* Return STATUS_CODE: ELEMENT_NOT_FOUND
INVALID_VECTOR_POINTER
SUCCES
*/
STATUS VectorRemoveValue(PDYNAMIC_VECTOR v, LPVOID value, BOOL(IsThis)(LPVOID el1, LPVOID el2), STATUS(*DestroyValue)(LPVOID val));


STATUS VectorGet(DYNAMIC_VECTOR, int, LPVOID*);



/**
* @Returns: STATUS_VALUE: SUCCES - find element
*						   NOT_FOUND
*						   INVALID_VECTOR_POINTER
*/
STATUS VectorSearch(PDYNAMIC_VECTOR v, LPVOID value, int *position, BOOL(IsThis)(LPVOID el1, LPVOID el2));
#endif // !_HEADER_H_