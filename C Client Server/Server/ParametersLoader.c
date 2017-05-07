#include "ParametersLoader.h"
#include <strsafe.h>

#define LOGGER_DEFAULT "logger.txt"
#define DEFAULT_PIPE "numepipe"
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

STATUS LoadParameters(char** argv, int argc, CHAR** nWorkers, CHAR** nMaxClients, CHAR** pipeName,CHAR** logger)
{
	STATUS status;
	BOOL isNWorkers;
	BOOL isNMaxClients;
	BOOL isPipe;
	BOOL isLogger;
	INT iParameter;
	HRESULT res;

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
			*nWorkers = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*nWorkers, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
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
			*pipeName = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*pipeName, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
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
			*nMaxClients = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*nMaxClients, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
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
			*logger = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*logger, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
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
		*pipeName = (CHAR*)malloc(sizeof(LOGGER_DEFAULT) + sizeof(CHAR));
		StringCchCopyA(*pipeName, sizeof(LOGGER_DEFAULT) + 1, LOGGER_DEFAULT);

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

