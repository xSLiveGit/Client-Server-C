#include "ParametersLoader.h"
#include <strsafe.h>

#define LOGGER_DEFAULT "logger.txt"
#define DEFAULT_PIPE "numepipe"
#define MAX_PARAMETER_SIZE 99

BOOL IsNumber(CHAR* string)
{
	unsigned size;
	unsigned index;

	size = strlen(string);
	for (index = 0; index < size;index++)
	{
		if (string[index] < '0' || string[index] > '9')
			return FALSE;
	}
	return TRUE;
}

BOOL VerySize(CHAR* string)
{
	HRESULT result;
	size_t length;

	result = S_OK;
	length = 0;

	result = StringCchLengthA(string, 99, &length);
	if((S_OK!=result) || (length > 99))
	{
		return FALSE;
	}
	return TRUE;
}

STATUS LoadParameters(
	_In_ char** argv, 
	_In_ int argc, 
	_In_ CHAR** nWorkers,
	_In_ CHAR** nMaxClients, 
	_In_ CHAR** pipeName,
	_In_ CHAR** logger)
{
	STATUS status;
	BOOL isNWorkers;
	BOOL isNMaxClients;
	BOOL isPipe;
	BOOL isLogger;
	BOOL validSize;
	INT iParameter;
	HRESULT res;
	UINT size;

	size = 0;
	validSize = TRUE;
	res = TRUE;
	status = SUCCESS;
	isNWorkers = FALSE;
	isNMaxClients = FALSE;
	isPipe = FALSE;
	isLogger = FALSE;


	if ((NULL == argv) || (NULL == nWorkers) || (NULL == nMaxClients) || (NULL == pipeName) ||(NULL == logger))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	*pipeName = NULL;
	*nMaxClients = NULL;
	*nWorkers = NULL;
	*logger = NULL;

	for (iParameter = 1; iParameter < argc; iParameter++)
	{
		validSize = VerySize(argv[iParameter]);
		if(!validSize)
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
	}

	for (iParameter = 1; iParameter < argc; iParameter++)
	{
		res = StringCchLengthA(argv[iParameter], MAX_PARAMETER_SIZE, &size);
		if (argv[iParameter][0] != '-' || argv[iParameter][2] != '=')
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
		
		else if (argv[iParameter][1] == 'w')
		{
			if (isNWorkers == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isNWorkers = TRUE;
			*nWorkers = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*nWorkers, size - 2, argv[iParameter] + 3);
			if (res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'n')
		{
			if (isPipe == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isPipe = TRUE;
			*pipeName = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*pipeName, size - 2, argv[iParameter] + 3);
			if (res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'c')
		{
			if (isNMaxClients == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isNMaxClients = TRUE;
			*nMaxClients = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*nMaxClients, size - 2, argv[iParameter] + 3);
			if (res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'l')
		{
			if (isLogger == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isLogger = TRUE;
			*logger = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*logger, size - 2, argv[iParameter] + 3);
			if (res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
	}

	if ((TRUE != isNWorkers) || (TRUE != isNMaxClients))
	{
		status = WRONG_ARGUMENTS_STRUCTURE;
		goto Exit;
	}
	if(!IsNumber(*nWorkers) || !IsNumber(*nMaxClients))
	{
		status = WRONG_ARGUMENTS_STRUCTURE;
		goto Exit;
	}
	if (!isPipe)
	{
		*pipeName = (CHAR*)malloc(sizeof(DEFAULT_PIPE) + sizeof(CHAR));
		StringCchCopyA(*pipeName, sizeof(DEFAULT_PIPE) + 1, DEFAULT_PIPE);
	}
	if(!isLogger)
	{
		*logger = (CHAR*)malloc(sizeof(LOGGER_DEFAULT) + sizeof(CHAR));
		StringCchCopyA(*logger, sizeof(LOGGER_DEFAULT) + 1, LOGGER_DEFAULT);
	}
Exit:
	if (SUCCESS != status)
	{
		free(*nWorkers);
		free(*nMaxClients);
		free(*pipeName);
		free(*logger);
		*logger = NULL;
		*nWorkers = NULL;
		*nMaxClients = NULL;
		*pipeName = NULL;
	}
	return status;
}

