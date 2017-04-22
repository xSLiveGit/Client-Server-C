#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>

#ifndef PACKAGE_SIZE
#define PACKAGE_SIZE 4096
#endif //!PACKAGE_SIZE

STATUS OpenConnexion(PCLIENT pclient);
STATUS RemoveClient(PCLIENT pclient);
STATUS Run(PCLIENT pclient,char*,char*);
STATUS PrintAllMessages(PPACKAGE list, unsigned long size, HANDLE outputFileHandle);
STATUS ConstructPackage(PPACKAGE *packageList, DWORD *packageListSize, HANDLE openedInputFileHandle, DWORD totalBytesSize);

STATUS CreateClient(PCLIENT pclient, char* pipeName)
{
	STATUS status = 0;
	if (NULL == pclient)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	if(NULL == pipeName)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	pclient->pipeName = pipeName;
	pclient->clientProtocol = (PCLIENT_PROTOCOL)malloc(sizeof(CLIENT_PROTOCOL));
	CreateProtocol(pclient->clientProtocol);
	//pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, pipeName);
	pclient->OpenConnexion = &OpenConnexion;
	pclient->RemoveClient = &RemoveClient;
	pclient->Run = &Run;
Exit:
	return status;
}

STATUS OpenConnexion(PCLIENT pclient)
{
	STATUS status;

	status = pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, pclient->pipeName);

	return status;
}

STATUS RemoveClient(PCLIENT pclient)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---

	status |= pclient->clientProtocol->CloseConnexion(pclient->clientProtocol);
	free(pclient->clientProtocol);
	// --- Exit/CleanUp ---
	return status;
}

STATUS Run(PCLIENT pclient,char* inputFile,char* outputFile)
{
	// --- Declarations ---
	STATUS status;
	int packetsNumber;
	PPACKAGE packageList ;
	char buffer[4096];
	size_t readedCharacters;
	HANDLE inputFileHandle;
	HANDLE outputFileHandle;
	DWORD stringSize;
	DWORD numberOfPackages;
	// --- End declarations ---

	// --- Initializations ---
	status = SUCCESS;
	packageList = NULL;
	strcpy(buffer, "");
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	readedCharacters = 0;
	numberOfPackages = 0;
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	// --- End initializations ---

	//Open file for processing them 
	inputFileHandle = CreateFile(
		inputFile,				//	_In_     LPCTSTR               lpFileName,
		GENERIC_READ,			//	_In_     DWORD                 dwDesiredAccess,
		FILE_SHARE_READ,		//	_In_     DWORD                 dwShareMode,
		NULL,					//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,			//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,	//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL					//_In_opt_ HANDLE                hTemplateFile
		);
	if(NULL == inputFileHandle)
	{
		status = FILE_ERROR;
		goto Exit;
	}
	outputFileHandle = CreateFile(
		inputFile,								//	_In_     LPCTSTR               lpFileName,
		GENERIC_WRITE,							//	_In_     DWORD                 dwDesiredAccess,
		ERROR_FILE_SHARE_RESOURCE_CONFLICT,		//	_In_     DWORD                 dwShareMode,
		NULL,									//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		CREATE_ALWAYS,							//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,					//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL									//	_In_opt_ HANDLE                hTemplateFile
		);
	if (NULL == outputFileHandle)
	{
		status = FILE_ERROR;
		goto Exit;
	}
	stringSize = GetFileSize(inputFileHandle, NULL);

	// --- Process --
	status |= pclient->OpenConnexion(pclient);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	
	ConstructPackage(&packageList, &numberOfPackages,inputFileHandle, stringSize);

	status |= pclient->clientProtocol->SendNetworkMessage(pclient->clientProtocol, numberOfPackages, packageList,FALSE);
	//@TODO must dezalloc because ReadNetworkMessage construct other packageList
	if(SUCCESS != status)
	{
		printf_s("Can't sent.\n");
		goto Exit;
	}
//	printf("client string before encryption process: %s", list[0].buffer);
	pclient->clientProtocol->ReadNetworkMessage(pclient->clientProtocol, &packetsNumber, &packageList, FALSE);
//	printf("client string after encryption process: %s", list[0].buffer);

	PrintAllMessages(packageList, packetsNumber, outputFileHandle);

	if (SUCCESS != status)
	{
		printf_s("Can't receive.\n");
		goto Exit;
	}
	//printf_s("%s", readedList[0].buffer);
	// --- Exit/CleanUp ---
Exit:
	if(NULL != inputFileHandle)
	{
		CloseHandle(inputFileHandle);
		inputFileHandle = NULL;
	}
	if (NULL != outputFileHandle)
	{
		CloseHandle(outputFileHandle);
		outputFileHandle = NULL;
	}
	return status;
}

STATUS PrintAllMessages(PPACKAGE list, unsigned long size, HANDLE outputFileHandle)
{
	STATUS status;
	unsigned int i;
	int writedCharacters;
	BOOL res;

	res = TRUE;
	status = SUCCESS;
	writedCharacters = 0;

	for (i = 0; i < size && (SUCCESS == status); i++)
	{
		//fwrite(list[i].buffer, 1, list[i].size, outputFileHandle);
		res = WriteFile(
			outputFileHandle,					//	_In_        HANDLE       hFile,
			list[i].buffer,						//	_In_        LPCVOID      lpBuffer,
			list[i].size,						//	_In_        DWORD        nNumberOfBytesToWrite,
			(DWORD*)&writedCharacters,			//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL								//	_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
		
		/*if(writedCharacters != list[i].size)
		{
			status = FILE_ERROR;
		}*/
	}
	FlushFileBuffers(
		outputFileHandle					//	_In_ HANDLE hFile
	);
	return status;
}

/** 
 * This function alloc memory for packageList
 * 
 *
 *
 */
STATUS ConstructPackage(PPACKAGE *packageList,DWORD *packageListSize,HANDLE openedInputFileHandle,DWORD totalBytesSize)
{
	STATUS status ;
	DWORD totalPackagesNumber;
	unsigned int iPackage;
	DWORD readedBytesNumber;
	PPACKAGE packageListWrapper;

	if (NULL == packageList || NULL == openedInputFileHandle)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status = SUCCESS;
	totalPackagesNumber = totalBytesSize / 4096;
	if(totalBytesSize % 4096 > 0)
	{
		totalPackagesNumber++;
	}
	packageListWrapper = (PACKAGE*)malloc(totalPackagesNumber * sizeof(PACKAGE));

	for (iPackage = 0; iPackage < totalPackagesNumber && (SUCCESS == status);iPackage++)
	{
		//readedBytesNumber = fread(packageListWrapper[iPackage].buffer, sizeof(char), PACKAGE_SIZE, openedInputFileHandle);
		ReadFile(
			openedInputFileHandle,						//	_In_        HANDLE       hFile,
			packageListWrapper[iPackage].buffer,		//	_Out_       LPVOID       lpBuffer,
			PACKAGE_SIZE,								//	_In_        DWORD        nNumberOfBytesToRead,
			&readedBytesNumber,							//	_Out_opt_   LPDWORD      lpNumberOfBytesRead,
			NULL										//	_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		packageListWrapper[iPackage].size = readedBytesNumber;
		if(readedBytesNumber != PACKAGE_SIZE && iPackage != (totalPackagesNumber-1))
		{
			status = FILE_ERROR;
		}
	}

	*packageList = packageListWrapper;
	*packageListSize = totalPackagesNumber;
Exit:
	return status;
}