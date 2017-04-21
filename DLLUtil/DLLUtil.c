// DLLUtil.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "DllUtil.h"
#include <string.h>

 STATUS MemoryCopy(const void* source, DWORD size, void* destination)
{
	DWORD i;
	STATUS status = SUCCESS;
	char *csource;
	char *cdestination;

	if(NULL == source || NULL == destination)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	csource = (char *)source;
	cdestination = (char *)destination;

	for (i = 0; i < size;i++)
	{
		*(cdestination + i) = *(csource + i);
	}
Exit:
	return status;
	
}
 STATUS StringToInt(char * s1, int *numar)
{
	int index;
	STATUS status;

	status = SUCCESS;
	*numar = 0;
	index = 0;

	if (NULL == s1 || NULL == numar)
	{
		status = NULL_POINTER_ERROR;;
		goto CleanUp;
	}

	if ('-' == s1[0])
	{
		index++;
	}
	while ('\0' != s1[index])
	{
		*numar = *numar * 10 + (s1[index] - '0');
		index++;
	}
	if ('-' == s1[0])
	{
		*numar = *numar * (-1);
	}
CleanUp:
	return status;
}
 STATUS IntToString(int number, char * result)
{
	int size;
	int miroredNumber;
	int i;
	STATUS exitCode;

	exitCode = SUCCESS;
	size = 0;
	miroredNumber = 0;

	if (NULL == result)
	{
		exitCode = NULL_POINTER_ERROR;
		goto CleanUp;
	}

	do {
		size++;
		miroredNumber = miroredNumber * 10 + number % 10;
		number = number / 10;
	} while (0 != number);


	for (i = 0; i < size; i++)
	{
		result[i] = '0' + (miroredNumber % 10);
		miroredNumber = miroredNumber / 10;
	}

	result[size] = '\0';
CleanUp:
	return exitCode;
}

 BOOL ExportFirstNCharacters(const char* string, FILE* outputStream,unsigned int exportedNumerCharacters)
{
	unsigned int i;
	BOOL success;

	i = 0;
	success = TRUE;

	while( i < exportedNumerCharacters)
	{
		success = (EOF != fputc(string[i], outputStream));
		if(!success)
		{
			goto Exit;
		}
	}

Exit:
	return success;
}
