#include "ParametersLoader.h"
#include <strsafe.h>

/**
 *
 *	@params:
 *				-
 *
 *
 *
 */

/**
 * All outout parameters will be NULL if something will be wrong
 */
STATUS LoadParameters(char** argv, int argc, CHAR** inputFilePath, CHAR** outputFilePath, CHAR** encryptionKey, CHAR **username, CHAR** password, CHAR** pipeName)
{
	STATUS status;
	BOOL isInputFile;
	BOOL isEncryptionKey;
	BOOL isUsername;
	BOOL isPassword;
	BOOL isOutputFile;
	BOOL isPipe;
	INT iParameter;
	BOOL res;

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

	for (iParameter = 1; iParameter < argc;iParameter++)
	{
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
			*inputFilePath = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*inputFilePath,strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
//			if(!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
		}
		else if (argv[iParameter][1] == 'o')
		{
			if (isOutputFile == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isOutputFile = TRUE;
			*outputFilePath = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*outputFilePath, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
//			if (!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
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
//			if (!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
		}
		else if(argv[iParameter][1] == 'u')
		{
			if (isUsername == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isUsername = TRUE;
			*username = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*username, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
//			if (!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
		}
		else if (argv[iParameter][1] == 'p')
		{
			if (isPassword == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isPassword = TRUE;
			*password = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*password, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
//			if (!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
		}
		else if (argv[iParameter][1] == 'k')
		{
			if (isEncryptionKey == TRUE)
			{
				status = WRONG_ARGUMENTS_STRUCTURE;
				goto Exit;
			}
			isEncryptionKey = TRUE;
			*encryptionKey = (CHAR*)malloc(strlen(argv[iParameter])*sizeof(CHAR));
			res = StringCchCopyA(*encryptionKey, strlen(argv[iParameter]) - 2, argv[iParameter] + 3);
//			if (!res)
//			{
//				status = STRING_ERROR;
//				goto Exit;
//			}
		}
		else
		{
			status = WRONG_ARGUMENTS_STRUCTURE;
			goto Exit;
		}
	}

	if((TRUE != isInputFile) || (TRUE != isEncryptionKey)|| (TRUE != isUsername) || (TRUE != isPassword) )
	{
		status = WRONG_ARGUMENTS_STRUCTURE;
		goto Exit;
	}

	if(!isOutputFile)
	{
		*outputFilePath = (CHAR*)malloc(strlen(*inputFilePath)*sizeof(CHAR) + 2*sizeof(CHAR));
		res = StringCchCopyA(*outputFilePath, strlen(*inputFilePath) + 1, *inputFilePath);
//		if(!res)
//		{
//			status = STRING_ERROR;
//			goto Exit;
//		}
	}
	if(!isPipe)
	{
		*pipeName = (CHAR*)malloc(sizeof(DEFAULT_NAMED_PIPE) + sizeof(CHAR));
		StringCchCopyA(*pipeName, sizeof(DEFAULT_NAMED_PIPE) + 1, DEFAULT_NAMED_PIPE);
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