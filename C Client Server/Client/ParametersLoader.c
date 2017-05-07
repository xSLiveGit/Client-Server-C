#include "ParametersLoader.h"
#include <strsafe.h>

#define DEFAULT_KEY "key"
#define MAX_PARAMETER_SIZE 99
#define DEFAULT_NAMED_PIPE "numepipe"

BOOL VerySize(CHAR* string)
{
	HRESULT result;
	size_t length;

	result = S_OK;
	length = 0;

	result = StringCchLengthA(string, MAX_PARAMETER_SIZE, &length);
	if ((S_OK != result) || (length > MAX_PARAMETER_SIZE))
	{
		return FALSE;
	}
	return TRUE;
}
/**
* All output parameters will be NULL if something will be wrong
*/
STATUS LoadParameters(
	_In_ char** argv, 
	_In_ int argc, 
	_In_ CHAR** inputFilePath,
	_In_ CHAR** outputFilePath, 
	_In_ CHAR** encryptionKey, 
	_In_ CHAR** username, 
	_In_ CHAR** password, 
	_In_ CHAR** pipeName)
{
	STATUS status;
	BOOL isInputFile;
	BOOL isEncryptionKey;
	BOOL isUsername;
	BOOL isPassword;
	BOOL isOutputFile;
	BOOL isPipe;
	INT iParameter;
	HRESULT res;
	BOOL validSize;
	UINT size;

	size = 0;
	validSize = TRUE;
	res = TRUE;
	status = SUCCESS;
	isInputFile = FALSE;
	isEncryptionKey = FALSE;
	isUsername = FALSE;
	isPassword = FALSE;

	isOutputFile = FALSE;
	isPipe = FALSE;
	
	if((NULL == argv) || (NULL == inputFilePath) || (NULL == outputFilePath) || (NULL == encryptionKey) || (NULL == username) || (NULL == password) || (NULL == pipeName))
	{
		status = NULL_POINTER_ERROR;
	}

	*inputFilePath = NULL;
	*outputFilePath = NULL;
	*encryptionKey = NULL;
	*username = NULL;
	*password = NULL;
	*pipeName = NULL;

	for (iParameter = 1; iParameter < argc; iParameter++)
	{
		validSize = VerySize(argv[iParameter]);
		if (!validSize)
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
	}

	for (iParameter = 1; iParameter < argc;iParameter++)
	{
		res = StringCchLengthA(argv[iParameter], 99, &size);
		if(S_OK != res)
		{
			status = STRING_ERROR;
			goto Exit;
		}
		if(argv[iParameter][0]!='-' || argv[iParameter][2]!='=')
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
		else if(argv[iParameter][1] == 'i')
		{
			if(isInputFile == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isInputFile = TRUE;
			
			*inputFilePath = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*inputFilePath, size - 2, argv[iParameter] + 3);
			if(res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'o')
		{
			if (isOutputFile == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isOutputFile = TRUE;
			*outputFilePath = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*outputFilePath,size - 2, argv[iParameter] + 3);
			if(res != S_OK)
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
			if(res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if(argv[iParameter][1] == 'u')
		{
			if (isUsername == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isUsername = TRUE;
			*username = (CHAR*)malloc(size *sizeof(CHAR));
			res = StringCchCopyA(*username, size - 2, argv[iParameter] + 3);
			if(res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'p')
		{
			if (isPassword == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isPassword = TRUE;
			*password = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*password, size - 2, argv[iParameter] + 3);
			if(res != S_OK)
			{
				status = STRING_ERROR;
				goto Exit;
			}
		}
		else if (argv[iParameter][1] == 'k')
		{
			if (isEncryptionKey == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isEncryptionKey = TRUE;
			*encryptionKey = (CHAR*)malloc(size*sizeof(CHAR));
			res = StringCchCopyA(*encryptionKey, size - 2, argv[iParameter] + 3);
			if(res != S_OK)
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

	if((TRUE != isInputFile) || (TRUE != isUsername) || (TRUE != isPassword) )
	{
		status = WRONG_ARGUMENTS_STRUCTURE;
		goto Exit;
	}

	if(!isOutputFile)
	{
		StringCchLengthA(*inputFilePath, MAX_PARAMETER_SIZE, &size);
		*outputFilePath = (CHAR*)malloc(size*sizeof(CHAR) + 2*sizeof(CHAR));
		res = StringCchCopyA(*outputFilePath, size + 1, *inputFilePath);
		if(res != S_OK)
		{
				status = STRING_ERROR;
				goto Exit;
		}
	}
	if(!isPipe)
	{
		*pipeName = (CHAR*)malloc(sizeof(DEFAULT_NAMED_PIPE) * sizeof(CHAR) + sizeof(CHAR));
		res = StringCchCopyA(*pipeName, sizeof(DEFAULT_NAMED_PIPE) + 1, DEFAULT_NAMED_PIPE);
		if (res != S_OK)
		{
			status = STRING_ERROR;
			goto Exit;
		}
	}
	if(!isEncryptionKey)
	{
		*encryptionKey = (CHAR*)malloc(sizeof(DEFAULT_KEY) * sizeof(CHAR) + sizeof(CHAR));
		res = StringCchCopyA(*encryptionKey, sizeof(DEFAULT_KEY) + 1, DEFAULT_KEY);
		if (res != S_OK)
		{
			status = STRING_ERROR;
			goto Exit;
		}
	}
Exit:
	if(SUCCESS != status)
	{
		free(*inputFilePath);
		free(*outputFilePath);
		free(*encryptionKey);
		free(*username);
		free(*password);
		free(*pipeName);
		*inputFilePath = NULL;
		*outputFilePath = NULL;
		*encryptionKey = NULL;
		*username = NULL;
		*password = NULL;
		*pipeName = NULL;
	}
	return status;
}

/**
 *	-i="InputFile";
 *	-o="OutputFile";
 *	-n="Pipe Name"
 *	-u="Username"
 *	-p="Password"
 *	-k="encryption kye"
 *
 */