#define _CRT_SECURE_NO_WARNINGS

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "client.h"
#include <stdio.h>
#include <strsafe.h>
#include "ParametersLoader.h"
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

void ThreatError(STATUS status)
{
	if (MALLOC_FAILED_ERROR == status)
	{
		printf_s("Malloc error\n");
		return;
	}
	if(STRING_ERROR == status)
	{
		printf_s("String error\n");
		return;
	}
	if(WRONG_ARGUMENTS_STRUCTURE == status)
	{
		printf_s("Wrong arguments. The arguments are:\n"
				"\tMandatory arguments:\n"
				"\t\t-i=\"val\" where val is input file path\n"
				"\t\t-u=\"val\" where val us username\n"
				"\t\t-p=\"val\" where val is password\n"
				"\tOptional argument :\n"
				"\t\t-k=\"val\" where val is encryption key\n"
				"\t\t-o=\"val\" where val is output file path\n"
				"\t\t-n=\"val\" where val is pipe name\n"
			"Obs: arguments must be by next form: -k=val\n"
			);
	}
}

STATUS main(int argc, char** argv)
{
	STATUS status;
	CLIENT client;
	CHAR* inputFilePath;
	CHAR* outputFilePath;
	CHAR* pipeName;
	CHAR* username;
	CHAR* password;
	CHAR* encryptionKey;
	CHAR auxFile[200];
	CHAR auxFile2[100];
	BOOL same;
	HANDLE inputFileHandle;
	HANDLE outputFileHandle;
	DWORD nReadedBytes;
	CHAR* copyBuffer;
	DWORD totalBytes;
	DWORD nWritedBytes;
	BOOL res;

	res = TRUE;
	nWritedBytes = 0;
	totalBytes = 0;
	copyBuffer = NULL;
	nReadedBytes = 0;
	outputFileHandle = INVALID_HANDLE_VALUE;
	inputFileHandle = INVALID_HANDLE_VALUE;
	same = FALSE;
	status = SUCCESS;
	inputFilePath = NULL;
	outputFilePath = NULL;
	pipeName = NULL;
	username = NULL;
	password = NULL;
	encryptionKey = NULL;

	status = LoadParameters(argv, argc, &inputFilePath, &outputFilePath, &encryptionKey, &username, &password, &pipeName);
	if(SUCCESS != status)
	{
		ThreatError(status);
		goto Exit;
	}

	printf_s("Username: %s\nPassword:%s\nInput: %s\nOutput: %s\nKey: %s\nPipe: %s\n", username, password, inputFilePath, outputFilePath, encryptionKey, pipeName);
	if(0 == strcmp(inputFilePath,outputFilePath))
	{
		GetTempPathA(40, auxFile2);
		GetTempFileNameA(auxFile2, "temporary", 0, auxFile);
		printf_s("Err: %d", GetLastError());
		free(outputFilePath);
		outputFilePath = (CHAR*)malloc(100 * sizeof(CHAR));
		StringCchCopyA(outputFilePath, 90, auxFile);
		same = TRUE;
	}
	status |= CreateClient(&client, pipeName);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.OpenConnexion(&client);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.Run(&client, inputFilePath, outputFilePath,encryptionKey,username,password);
	if(SUCCESS == status && same)
	{
		totalBytes = (MAX_BUFFER_SIZE + 1);
		inputFileHandle = CreateFileA(
			outputFilePath,			//	_In_     LPCTSTR               lpFileName,
			GENERIC_READ,			//	_In_     DWORD                 dwDesiredAccess,
			FILE_SHARE_READ,		//	_In_     DWORD                 dwShareMode,
			NULL,					//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			OPEN_EXISTING,			//	_In_     DWORD                 dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,	//	_In_     DWORD                 dwFlagsAndAttributes,
			NULL					//_In_opt_ HANDLE                hTemplateFile
			);
		if (NULL == inputFileHandle || INVALID_HANDLE_VALUE == inputFileHandle)
		{
			status = FILE_ERROR;
			printf_s("inputFileHandle is invalid");
			goto Exit;
		}


		outputFileHandle = CreateFileA(
			inputFilePath,							//	_In_     LPCTSTR               lpFileName,
			GENERIC_WRITE,							//	_In_     DWORD                 dwDesiredAccess,
			0,										//	_In_     DWORD                 dwShareMode,
			NULL,									//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			OPEN_ALWAYS,							//	_In_     DWORD                 dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,					//	_In_     DWORD                 dwFlagsAndAttributes,
			NULL									//	_In_opt_ HANDLE                hTemplateFile
			);
		if (NULL == outputFileHandle || INVALID_HANDLE_VALUE == outputFileHandle)
		{
			status = FILE_ERROR;
			goto Exit;
		}
		copyBuffer = (CHAR*)malloc(totalBytes * sizeof(CHAR));
		while(TRUE)
		{
			res = ReadFile(inputFileHandle, copyBuffer, totalBytes, &nReadedBytes, NULL);
			if(!res)
			{
				printf_s("Failed\n");
				goto Exit;
			}
			res = WriteFile(outputFileHandle, copyBuffer, nReadedBytes, &nWritedBytes, NULL);
			if (!res || (nReadedBytes != nWritedBytes))
			{
				printf_s("Failed\n");
				goto Exit;
			}
			if(nWritedBytes != totalBytes)
			{
				break;
			}
		}
		FlushFileBuffers(outputFileHandle);
		printf_s("Successfully\n");
		CloseHandle(inputFileHandle);
		CloseHandle(outputFileHandle);
		res = DeleteFile(auxFile);
	}
	
Exit:
	status |= client.RemoveClient(&client);
	printf_s("Press enter...\n");
	getchar();
	free(inputFilePath);
	free(outputFilePath);
	free(encryptionKey);
	free(username);
	free(password);
	free(pipeName);
	free(copyBuffer);
	_CrtDumpMemoryLeaks();
	return status;
}