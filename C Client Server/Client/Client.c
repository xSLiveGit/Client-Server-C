#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#ifndef PACKAGE_SIZE
#define PACKAGE_SIZE 4096
#endif //!PACKAGE_SIZE

STATUS OpenConnexion(PCLIENT pclient);
STATUS RemoveClient(PCLIENT pclient);
STATUS Run(PCLIENT pclient, CHAR*, CHAR*);
STATUS ExportAllMessages(PPACKAGE list, unsigned long size, HANDLE outputFileHandle);
STATUS ConstructPackage(PPACKAGE *packageList, DWORD *packageListSize, HANDLE openedInputFileHandle, DWORD totalBytesSize);
STATUS ReadAllEncryptedPackagesAndWriteInTheOutputFile(HANDLE openedOutputFileHandle, DWORD nPackages, PPROTOCOL protocol);
STATUS ReadAndSendPackages(HANDLE openedInputFileHandle, DWORD *sendedPackages, DWORD inputFileSize, PCLIENT pclient);
STATUS LoginHandler(CHAR* username, CHAR* password, PPROTOCOL protocol);
STATUS CreateClient(PCLIENT pclient, CHAR* pipeName);

STATUS CreateClient(PCLIENT pclient, CHAR* pipeName)
{
	STATUS status = 0;
	if (NULL == pclient)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	if (NULL == pipeName)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	pclient->pipeName = pipeName;
	pclient->clientProtocol = (PPROTOCOL)malloc(sizeof(PROTOCOL));
	CreateProtocol(pclient->clientProtocol);
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
	STATUS status;

	status = SUCCESS;

	//status = pclient->clientProtocol->CloseConnexion(pclient->clientProtocol);
	free(pclient->clientProtocol);
	return status;
}

STATUS Run(PCLIENT pclient, CHAR* inputFile, CHAR* outputFile)
{
	// --- Declarations ---
	STATUS status;
	char buffer[4096];
	HANDLE inputFileHandle;
	HANDLE outputFileHandle;
	DWORD readedBytes;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nSenededPackages;
	PACKAGE package;
	DWORD totalSize;
	// --- End declarations ---

	// --- Initializations ---
	status = SUCCESS;
	strcpy(buffer, "");
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	readedBytes = 0;
	package.size = 0;
	nSenededPackages = 0;
	totalSize = 0;
	// --- End initializations ---

	//Open file for processing them 
	inputFileHandle = CreateFileA(
		inputFile,				//	_In_     LPCTSTR               lpFileName,
		GENERIC_READ,			//	_In_     DWORD                 dwDesiredAccess,
		FILE_SHARE_READ,		//	_In_     DWORD                 dwShareMode,
		NULL,					//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,			//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,	//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL					//_In_opt_ HANDLE                hTemplateFile
		);
	if (NULL == inputFileHandle)
	{
		status = FILE_ERROR;
		goto Exit;
	}
	printf_s("In client initial thread is: %p\n", pclient->clientProtocol->pipeHandle);
	totalSize = GetFileSize(inputFileHandle, &totalSize);

	outputFileHandle = CreateFileA(
		outputFile,								//	_In_     LPCTSTR               lpFileName,
		GENERIC_WRITE,							//	_In_     DWORD                 dwDesiredAccess,
		0,										//	_In_     DWORD                 dwShareMode,
		NULL,									//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_ALWAYS,							//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,					//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL									//	_In_opt_ HANDLE                hTemplateFile
		);
	if (NULL == outputFileHandle)
	{

		status = FILE_ERROR;
		goto Exit;
	}

	// --- Process --
	//status |= pclient->OpenConnexion(pclient);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	//Initi requet
	request = INITIALIZE_REQUEST;
	status = pclient->clientProtocol->SendPackage(pclient->clientProtocol, &request, sizeof(request));
	if(SUCCESS != status)
	{
		goto Exit;
	}
	status = pclient->clientProtocol->ReadPackage(pclient->clientProtocol, &response, sizeof(response), &readedBytes);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	printf_s("Client readed response:\n");

	//Verify response
	if (ACCEPTED_CONNECTION_RESPONSE == response)
	{
		printf_s("Client accepted response:\n");
		pclient->clientProtocol->ReadPackage(pclient->clientProtocol, &package, sizeof(package), &readedBytes);
		package.buffer[package.size] = '\0';
		//pclient->clientProtocol->CloseConnexion(pclient->clientProtocol);
		Sleep(2000);
		status = pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, package.buffer);
		printf_s("New name is: %s New handle is: %p\n", package.buffer, pclient->clientProtocol->pipeHandle);
	}
	else if (REJECTED_CONNECTION_RESPONSE == response)
	{
		printf_s("Client rejected response : \n");
		goto Exit;
	}
	else
	{
		printf_s("Wrong behavior \n");
		goto Exit;
	}

	//Login area
	status = LoginHandler("Raul", "ParolaRaul", pclient->clientProtocol);
	if (status != SUCCESS_LOGIN)
	{
		request = FINISH_CONNECTION_REQUEST;
		pclient->clientProtocol->SendPackage(pclient->clientProtocol, &request, sizeof(request));
		goto Exit;
	}
	else
	{
		printf_s("Successfully login.\n");
	}

	//Encryption Area
	printf_s("Try to send package for encrypt them\n");
	status = ReadAndSendPackages(inputFileHandle, &nSenededPackages, totalSize, pclient);
	if (SUCCESS != status)
	{
		printf_s("Can't sent.\n");
		goto Exit;
	}
	printf_s("Successfully send packages for encrypt them.\n.Try to receive them and to write in the otput file.\n");
	
	status = ReadAllEncryptedPackagesAndWriteInTheOutputFile(outputFileHandle, nSenededPackages, pclient->clientProtocol);
	if(SUCCESS == status)
	{
		printf_s("Received all encrypted packages and writed them in the outputfile\n");
	}
	else
	{
		printf_s("Rewrite encrypted message error\n");
	}

Exit:
	request = FINISH_CONNECTION_REQUEST;
	pclient->clientProtocol->SendPackage(pclient->clientProtocol, &request, sizeof(request));
	if (INVALID_HANDLE_VALUE != inputFileHandle && NULL != inputFileHandle)
//	{
//		CloseHandle(inputFileHandle);
//		inputFileHandle = NULL;
//	}
//	if (INVALID_HANDLE_VALUE != outputFileHandle && NULL != outputFileHandle)
//	{
//		CloseHandle(outputFileHandle);
//		outputFileHandle = NULL;
//	}
//	pclient->clientProtocol->CloseConnexion(pclient->clientProtocol);
	return status;
}

/***
*
*
*
*
* Returns:
*		-	SUCCESS_LOGIN - if the login operation has been end successfully
*		-	WRONG_CREDENTIALS - if the credentials aren't valid
*		-	NULL_POINTER_ERROR - if the pparameters are NULL
*/
STATUS LoginHandler(CHAR* username, CHAR* password, PPROTOCOL protocol)
{
	STATUS status;
	PACKAGE package;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nReadedBytes;
	printf_s("Start login\n");
	status = SUCCESS;
	package.size = 0;
	request = LOGIN_REQUEST;
	nReadedBytes = 0;

	if (NULL == username || NULL == password || NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status = protocol->SendPackage(protocol, &request, sizeof(request));
	if (SUCCESS != status)
	{
		goto Exit;
	}
	printf_s("The login request has been sent.\n");
	package.size = (DWORD)strlen(username);
	memcpy(package.buffer, username, package.size);
	package.buffer[package.size] = '\0';
	status = protocol->SendPackage(protocol, &package, sizeof(package));
	printf_s("The username has been sent.\n");

	if (SUCCESS != status)
	{
		goto Exit;
	}

	package.size = (DWORD)strlen(password);
	memcpy(package.buffer, password, package.size);
	package.buffer[package.size] = '\0';
	protocol->SendPackage(protocol, &package, sizeof(package));
	printf_s("The password has been sent.\n");

	if (SUCCESS != status)
	{
		goto Exit;
	}

	status = protocol->ReadPackage(protocol, &response, sizeof(response), &nReadedBytes);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	if (SUCCESS_LOGIN_RESPONSE == response)
	{
		status = SUCCESS_LOGIN;
		goto Exit;//SUCCESS status
	}
	if (WRONG_CREDENTIALS_RESPONSE == response)
	{
		printf_s("Wrong credentials");
		status = WRONG_CREDENTIALS;
		goto Exit;
	}
Exit:
	printf_s("End login\n");
	return status;
}

STATUS ExportAllMessages(PPACKAGE list, unsigned long size, HANDLE outputFileHandle)
{
	STATUS status;
	unsigned int i;
	DWORD writedCharacters;
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
			&writedCharacters,					//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
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

STATUS ReadAllEncryptedPackagesAndWriteInTheOutputFile(HANDLE openedOutputFileHandle, DWORD nPackages, PPROTOCOL protocol)
{
	STATUS status;
	PACKAGE package;
	DWORD iPackage;
	DWORD nReadedBytes;
	BOOL res;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;

	request = GET_ENCRYPTED_MESSAGE_REQUEST;
	status = SUCCESS;
	package.size = 0;
	nReadedBytes = 0;
	res = TRUE;

	if (NULL == openedOutputFileHandle || NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	for (iPackage = 0; iPackage < nPackages; iPackage++)
	{
		status = protocol->SendPackage(protocol, &request, sizeof(request));
		if (SUCCESS != status)
		{
			goto Exit;
		}
		status = protocol->ReadPackage(protocol, &response, sizeof(response), &nReadedBytes);
		if (SUCCESS != status || response != OK_RESPONSE)
		{
			status = COMUNICATION_ERROR;
			goto Exit;
		}

		status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
		if (SUCCESS != status)
		{
			goto Exit;
		}

		if ((iPackage < nPackages - 1) && (package.size != MAX_BUFFER_SIZE))
		{
			status = COMUNICATION_ERROR;
			goto Exit;
		}
		res = WriteFile(
			openedOutputFileHandle,					//	_In_        HANDLE       hFile,
			package.buffer,							//	_In_        LPCVOID      lpBuffer,
			package.size,							//	_In_        DWORD        nNumberOfBytesToWrite,
			&nReadedBytes,							//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL									//	_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		if (!res || nReadedBytes != package.size)
		{
			status = FILE_ERROR;
			goto Exit;
		}
	}
Exit:
	return status;
}



STATUS ReadAndSendPackages(HANDLE openedInputFileHandle, DWORD *sendedPackages, DWORD inputFileSize, PCLIENT pclient)
{
	STATUS status;
	DWORD nPackages;
	DWORD iPackage;
	DWORD nReadedBytes;
	PACKAGE package;
	REQUEST_TYPE request;

	request = ENCRYPTED_MESSAGE_REQUEST;
	status = SUCCESS;
	nPackages = inputFileSize / MAX_BUFFER_SIZE;
	if (inputFileSize % MAX_BUFFER_SIZE)
	{
		nPackages++;
	}
	printf_s("Total packages to send : %d", nPackages);
	for (iPackage = 0; iPackage < nPackages && (SUCCESS == status); iPackage++)
	{
		//readedBytesNumber = fread(packageListWrapper[iPackage].buffer, sizeof(char), PACKAGE_SIZE, openedInputFileHandle);
		ReadFile(
			openedInputFileHandle,						//	_In_        HANDLE       hFile,
			package.buffer,								//	_Out_       LPVOID       lpBuffer,
			PACKAGE_SIZE,								//	_In_        DWORD        nNumberOfBytesToRead,
			&nReadedBytes,								//	_Out_opt_   LPDWORD      lpNumberOfBytesRead,
			NULL										//	_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		package.size = nReadedBytes;
		if (nReadedBytes != MAX_BUFFER_SIZE && iPackage < nPackages - 1)//Except for the last package, all had to be the same dimension
		{
			status = FILE_ERROR;
			goto Exit;
		}
		printf_s("Send req %d", iPackage);
		status = pclient->clientProtocol->SendPackage(pclient->clientProtocol, &request, sizeof(request));

		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send req %d", iPackage);

			goto Exit;
		}
		printf_s("Successfully Send req %d", iPackage);

		printf_s("Send pack %d", iPackage);
		status = pclient->clientProtocol->SendPackage(pclient->clientProtocol, &package, sizeof(package));
		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send pack %d", iPackage);
			goto Exit;
		}
		printf_s("Successfully Send pack %d", iPackage);

	}
	*sendedPackages = nPackages;
Exit:
	return status;
}

STATUS ConstructPackage(PPACKAGE *packageList, DWORD *packageListSize, HANDLE openedInputFileHandle, DWORD totalBytesSize)
{
	STATUS status;
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
	if (totalBytesSize % 4096 > 0)
	{
		totalPackagesNumber++;
	}
	packageListWrapper = (PACKAGE*)malloc(totalPackagesNumber * sizeof(PACKAGE));

	for (iPackage = 0; iPackage < totalPackagesNumber && (SUCCESS == status); iPackage++)
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
		if (readedBytesNumber != PACKAGE_SIZE && iPackage != (totalPackagesNumber - 1))
		{
			status = FILE_ERROR;
		}

	}

	*packageList = packageListWrapper;
	*packageListSize = totalPackagesNumber;
Exit:
	return status;
}