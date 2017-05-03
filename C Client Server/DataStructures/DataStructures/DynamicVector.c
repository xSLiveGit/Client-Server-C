
#include "DynamicVector.h"
#include "Status.h"
#include <stdio.h> 
#include <stdlib.h>
#define INIT_VECTOR_CAPACITY 1

STATUS
VectorCreate(PDYNAMIC_VECTOR *dynamicVector)
{
	DYNAMIC_VECTOR* dv;
	STATUS statusCode;

	statusCode = SUCCESS;
	dv = NULL;
	if (NULL == dynamicVector)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}
	dv = (DYNAMIC_VECTOR*)malloc(sizeof(DYNAMIC_VECTOR));

	if (NULL == dv)
	{
		statusCode = MALLOC_FAILED_ERROR;//malloc error
		goto CleanUp;
	}

	(*dv).capacity = INIT_VECTOR_CAPACITY;
	(*dv).size = 0;
	(*dv).v = (LPVOID*)malloc(INIT_VECTOR_CAPACITY * sizeof(LPVOID));
	//dv->v = NULL;
	if (NULL == (*dv).v)
	{
		statusCode = MALLOC_FAILED_ERROR;
	}
	*dynamicVector = dv;
CleanUp:
	if (MALLOC_FAILED_ERROR == statusCode)
	{
		free(dv);
	}
	return statusCode;
}

STATUS
VectorDestroy(PDYNAMIC_VECTOR *v)
{
	STATUS statusCode;

	statusCode = SUCCESS;
	DYNAMIC_VECTOR* wp;

	if (NULL == v)
	{
		return NULL_POINTER_ERROR;
	}

	wp = *v;
	//if (NULL == *v)
	//{
	//	return NULL_POINTER_ERROR;


	if (NULL != wp && wp->v != NULL)
		free(wp->v);
	free(wp);
	*v = NULL;
	return SUCCESS;//succes
}

int Resize(DYNAMIC_VECTOR *v) {
	LPVOID* nv;
	STATUS statusCode;
	int i;

	statusCode = SUCCESS;
	nv = NULL;

	if (NULL == v)
	{
		statusCode = NULL_POINTER_ERROR;//invalid vector pointer
		goto CleanUp;
	}

	nv = (LPVOID*)malloc((v->capacity * 2)*sizeof(LPVOID));
	if (NULL == nv)
	{
		statusCode = MALLOC_FAILED_ERROR;
		goto CleanUp;
	}

	for (i = 0; i < v->size; i++)
	{
		nv[i] = v->v[i];
	}
	free(v->v);
	v->v = NULL;
	v->v = nv;
	v->capacity *= 2;
CleanUp:

	return statusCode;
}

STATUS VectorAdd(PDYNAMIC_VECTOR v, LPVOID el)
{
	STATUS statusCode;

	statusCode = SUCCESS;

	if (NULL == v)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	if (v->size == v->capacity)
	{
		Resize(v);
	}
	v->v[v->size] = el;
	v->size++;
CleanUp:
	return statusCode;
}

STATUS VectorLength(DYNAMIC_VECTOR v)
{
	return v.size;
}

STATUS VectorRemovePosition(PDYNAMIC_VECTOR v, int pos)
{
	STATUS statusCode;

	statusCode = SUCCESS;

	int i;

	if (NULL == v)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	if (pos < 0 || pos >= v->size)
	{
		statusCode = INDEX_OUT_OF_BOUNDS;
		goto CleanUp;
	}

	else
	{
		for (i = pos; i < v->size - 1; i++)
		{
			v->v[i] = v->v[i + 1];
		}
		v->size--;
	}

CleanUp:
	return statusCode;
}

STATUS VectorRemoveValue(PDYNAMIC_VECTOR v, LPVOID value)
{
	int i, j;
	STATUS statusCode;

	statusCode = ELEMENT_NOT_FOUND;

	if (NULL == v)
	{
		statusCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	for (i = 0; i < v->size;)
	{
		if (v->v[i] == value)
		{
			statusCode = SUCCESS;
			for (j = i; j < v->size - 1; j++)
			{
				v->v[j] = v->v[j + 1];
			}
			v->size--;
		}
		else
		{
			i++;
		}
	}
CleanUp:
	return statusCode;
}

STATUS VectorGet(DYNAMIC_VECTOR v, int position, LPVOID *returned) {
	STATUS statusCode;

	statusCode = SUCCESS;

	if (position < 0 || position > v.size)
	{
		statusCode = INDEX_OUT_OF_BOUNDS;
		goto CleanUp;
	}
	*returned = v.v[position];
CleanUp:
	return statusCode;
}


STATUS VectorSearch(PDYNAMIC_VECTOR v, LPVOID value, int *position)
{
	STATUS status;
	int i;

	status = ELEMENT_NOT_FOUND;

	if (NULL == v)
	{
		status = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	for (i = 0; i < v->size; i++)
	{
		if (v->v[i] == value)
		{
			*position = i;
			status = SUCCESS;
			goto CleanUp;
		}
	}

CleanUp:
	return status;
}

