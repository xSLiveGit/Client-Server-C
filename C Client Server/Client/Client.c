#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#ifndef PACKAGE_SIZE
#define PACKAGE_SIZE 4096
#endif //!PACKAGE_SIZE

typedef struct
{
	char filename[100];
	DWORD *nEncryptedPackages;
	HANDLE openedFileHandle;
} PARAMS_LOAD;

STATUS OpenConnexion(PCLIENT pclient);
STATUS RemoveClient(PCLIENT pclient);
STATUS Start(PCLIENT pclient, CHAR*, CHAR*,CHAR* encryptionKey,CHAR* username,CHAR* password);
STATUS ExportAllMessages(PPACKAGE list, unsigned long size, HANDLE outputFileHandle);
STATUS ReadAllEncryptedPackagesAndWriteInTheOutputFile(HANDLE openedOutputFileHandle, DWORD nPackages, PPROTOCOL protocol);
STATUS ReadAndSendPackages(HANDLE openedInputFileHandle, DWORD *sendedPackages, DWORD inputFileSize, PCLIENT pclient);
STATUS LoginHandler(CHAR* username, CHAR* password, PPROTOCOL protocol);
STATUS CreateClient(PCLIENT pclient, CHAR* pipeName);
STATUS WINAPI ReceiverWorker(LPVOID parameter);
STATUS WINAPI SenderWorker(LPVOID parameter);


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
	pclient->Run = &Start;
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

STATUS Start(PCLIENT pclient, CHAR* inputFile, CHAR* outputFile,CHAR* encryptionKey,CHAR* username,CHAR* password)
{
	// --- Declarations ---
	STATUS status;
	char buffer[4096];
	CHAR newFileName[100];
	HANDLE inputFileHandle;
	HANDLE outputFileHandle;
	DWORD readedBytes;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nSenededPackages;
	PACKAGE package;
	DWORD totalSize;
	DWORD encrysize;
	PARAMS_LOAD *params;
	size_t universalSize;
	HANDLE sentPackageForEncryptHandle;
	HANDLE receivePackageForEncryptHandle;
	STATUS returenedStatusCodeReader;
	STATUS returnedStatusCodeSender;
	DWORD nReceivedPackages;
	DWORD nSentedPackages;
	LPSECURITY_ATTRIBUTES secutiry_attributes;
	// --- End declarations ---

	// --- Initializations ---
	secutiry_attributes = NULL;
	nReceivedPackages = 0;
	nSenededPackages = 0;
	sentPackageForEncryptHandle = INVALID_HANDLE_VALUE;
	receivePackageForEncryptHandle = INVALID_HANDLE_VALUE;
	returnedStatusCodeSender = SUCCESS;
	returenedStatusCodeReader = SUCCESS;
	universalSize = 0;
	params = NULL;
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
	encrysize = 0;
	// --- End initializations ---
	secutiry_attributes = (LPSECURITY_ATTRIBUTES)malloc(sizeof(SECURITY_ATTRIBUTES));
	secutiry_attributes->bInheritHandle = TRUE;
	secutiry_attributes->lpSecurityDescriptor = NULL;
	secutiry_attributes->nLength = sizeof(SECURITY_ATTRIBUTES);

	if(NULL == username || NULL == password || NULL == encryptionKey || NULL == inputFile || NULL == outputFile)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}


	//Open file for processing them 
	inputFileHandle = CreateFileA(
		inputFile,				//	_In_     LPCTSTR               lpFileName,
		GENERIC_READ,			//	_In_     DWORD                 dwDesiredAccess,
		FILE_SHARE_READ,		//	_In_     DWORD                 dwShareMode,
		secutiry_attributes,					//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,			//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,	//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL					//_In_opt_ HANDLE                hTemplateFile
		);
	if(NULL == inputFileHandle || INVALID_HANDLE_VALUE == inputFileHandle)
	{
		status = FILE_ERROR;
		printf_s("inputFileHandle is invalid");
		goto Exit;
	}

	printf_s("inputFileHandle in maine thread is: %p\n", inputFileHandle);
	printf_s("In client initial thread is: %p\n", pclient->clientProtocol->pipeHandle);
	totalSize = GetFileSize(inputFileHandle, &totalSize);
	printf_s("Input file hadnle from main thread has size: %d", totalSize);

	if(NULL == secutiry_attributes)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	outputFileHandle = CreateFileA(
		outputFile,								//	_In_     LPCTSTR               lpFileName,
		GENERIC_WRITE,							//	_In_     DWORD                 dwDesiredAccess,
		0,										//	_In_     DWORD                 dwShareMode,
		secutiry_attributes,									//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_ALWAYS,							//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,					//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL									//	_In_opt_ HANDLE                hTemplateFile
		);
	if (NULL == outputFileHandle || INVALID_HANDLE_VALUE == outputFileHandle)
	{

		status = FILE_ERROR;
		goto Exit;
	}
	printf_s("outputFileHandle in maine thread is: %p\n", outputFileHandle);

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
		StringCchCopyA(newFileName, sizeof(newFileName), package.buffer);
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
	status = LoginHandler(username, password, pclient->clientProtocol);
	if (status != SUCCESS_LOGIN)
	{
		if(USER_ALREADY_CONNECTED == status)
		{
			
		}
		else
		{
			request = FINISH_CONNECTION_REQUEST;
			pclient->clientProtocol->SendPackage(pclient->clientProtocol, &request, sizeof(request));
			goto Exit;
		}
	
	}
	
	else
	{
		printf_s("Successfully login.\n");
	}
	encrysize = (DWORD)strlen(encryptionKey);
	status = pclient->clientProtocol->SendPackage(pclient->clientProtocol, encryptionKey, encrysize);
	//Encryption Area
	
	params = (PARAMS_LOAD*)malloc(sizeof(PARAMS_LOAD));
	if(NULL == params)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	universalSize = strlen(newFileName) + 3;
	StringCchCopyA(params->filename, universalSize, newFileName);
	StringCchCatA(params->filename, universalSize, "R");
	params->nEncryptedPackages = &nSentedPackages;
	params->openedFileHandle = inputFileHandle;
	printf_s("Am deschis sender-ul\n");
	Sleep(1000);
	sentPackageForEncryptHandle = CreateThread(
		NULL,
		0,
		SenderWorker,
		(LPVOID)params,
		0,
		&returnedStatusCodeSender
		);
	if (NULL == sentPackageForEncryptHandle || INVALID_HANDLE_VALUE == sentPackageForEncryptHandle)
	{
		status = THREAD_ERROR;
		goto Exit;
	}
	printf_s("Am trecut de sender-ul\n");


	//Receiver
	params = (PARAMS_LOAD*)malloc(sizeof(PARAMS_LOAD));
	if (NULL == params)
	{
		status = MALLOC_FAILED_ERROR;
		TerminateThread(sentPackageForEncryptHandle, status);
		goto Exit;
	}
	universalSize = strlen(newFileName) + 3;
	StringCchCopyA(params->filename, universalSize, newFileName);
	StringCchCatA(params->filename, universalSize, "W");
	params->nEncryptedPackages = &nReceivedPackages;
	params->openedFileHandle = outputFileHandle;

	printf_s("Deschide receiver-ul\n");
	receivePackageForEncryptHandle = CreateThread(
		NULL,
		0,
		ReceiverWorker,
		(LPVOID)params,
		0,
		&returnedStatusCodeSender
		);
	printf_s("A trecut de receiver \n");

	if (NULL == sentPackageForEncryptHandle || INVALID_HANDLE_VALUE == sentPackageForEncryptHandle)
	{
		status = THREAD_ERROR;
		TerminateThread(sentPackageForEncryptHandle, status);
		goto Exit;
	}

	
	WaitForSingleObject(sentPackageForEncryptHandle, INFINITE);
	status = pclient->clientProtocol->ReadPackage(pclient->clientProtocol, &response, sizeof(response), &readedBytes);
	if (SUCCESS != status)
	{
		goto Exit;
	}

	if (OK_RESPONSE != response)
	{
		printf_s("NU e un raspus ok\n");
		TerminateThread(sentPackageForEncryptHandle, THREAD_ERROR);
		TerminateThread(receivePackageForEncryptHandle, THREAD_ERROR);
		printf_s("A problem has been occur during encryption process");
		goto Exit;
	}
	printf_s("Raspuns ok\n");
	printf_s("Am terminat de astepte dupa sender");
	while(nSentedPackages != nReceivedPackages)
	{
		Sleep(25);
	}
	Sleep(100);
	TerminateThread(receivePackageForEncryptHandle, SUCCESS);
	//AICI E GATA


Exit:
	free(secutiry_attributes);
	if (INVALID_HANDLE_VALUE != inputFileHandle && NULL != inputFileHandle)
	{
		CloseHandle(inputFileHandle);
		inputFileHandle = NULL;
	}
	if (INVALID_HANDLE_VALUE != outputFileHandle && NULL != outputFileHandle)
	{
		CloseHandle(outputFileHandle);
		outputFileHandle = NULL;
	}
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
	if(ALREADY_CONNECTED_RESPONSE == response)
	{
		status = USER_ALREADY_CONNECTED;
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




STATUS WINAPI ReceiverWorker(LPVOID parameter)
{
	STATUS status;
	PARAMS_LOAD params;
	PROTOCOL protocol;
	CHAR *filename;
	size_t universalSize;
	PACKAGE package;
	DWORD nReadedBytes;
	BOOL res;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	HANDLE outputFileHadnle;
	DWORD nPackages;
	DWORD *nReadedPackages;

	nReadedBytes = 0;
	nPackages = 0;
	outputFileHadnle = NULL;
	filename = NULL;
	universalSize = 0;
	status = SUCCESS;
	params.filename[0] = '\0';
	params.nEncryptedPackages = 0;
	request = GET_ENCRYPTED_MESSAGE_REQUEST;
	status = SUCCESS;
	package.size = 0;
	res = TRUE;


	params = *((PARAMS_LOAD*)parameter);
	outputFileHadnle = params.openedFileHandle;
	printf_s("outputfileHandle in receiver worker is: %p", outputFileHadnle);
	nReadedPackages = params.nEncryptedPackages;
	*nReadedPackages = 0;
	free((PARAMS_LOAD*)parameter);

	status = CreateProtocol(&protocol);
	if(SUCCESS != status)
	{
		goto Exit;
	}

	universalSize = strlen(params.filename);
	filename = (CHAR*)malloc((universalSize + 2)*sizeof(CHAR));
	if (NULL == filename)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	StringCchCopyA(filename, universalSize + 2, params.filename);


	status = protocol.InitializeConnexion(&protocol, filename);
	free(filename);
	filename = NULL;
	if(SUCCESS != status)
	{
		goto Exit;
	}
	while(TRUE)
	{
		status = protocol.SendPackage(&protocol, &request, sizeof(request));
		if (SUCCESS != status)
		{
			goto Exit;
		}
		status = protocol.ReadPackage(&protocol, &response, sizeof(response), &nReadedBytes);
		if (SUCCESS != status || response != OK_RESPONSE)
		{
			status = COMUNICATION_ERROR;
			goto Exit;
		}

		status = protocol.ReadPackage(&protocol, &package, sizeof(package), &nReadedBytes);
		if (SUCCESS != status)
		{
			goto Exit;
		}

		res = WriteFile(
			outputFileHadnle,					//	_In_        HANDLE       hFile,
			package.buffer,							//	_In_        LPCVOID      lpBuffer,
			package.size,							//	_In_        DWORD        nNumberOfBytesToWrite,
			&nReadedBytes,							//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL									//	_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		*nReadedPackages += 1;
		if (!res)
		{
			status = FILE_ERROR;
			goto Exit;
		}

		if ((package.size != MAX_BUFFER_SIZE))
		{
			goto Exit;
		}
	}

Exit:
	ExitThread(status);
}

STATUS WINAPI SenderWorker(LPVOID parameter)
{
	STATUS status;
	PARAMS_LOAD params;
	PROTOCOL protocol;
	CHAR *filename;
	size_t universalSize;
	unsigned long fileSize;
	DWORD nPackages;
	DWORD iPackage;
	HANDLE inputFileHandle;
	REQUEST_TYPE request;
	DWORD nReadedBytes;
	PACKAGE package;
	DWORD *nEncryptedPackages;

	nEncryptedPackages = NULL;
	nReadedBytes = 0;
	request = ENCRYPTED_MESSAGE_REQUEST;
	inputFileHandle = NULL;
	nPackages = 0;
	fileSize = 0;
	filename = NULL;
	universalSize = 0;
	status = SUCCESS;
	params.filename[0] = '\0';
	params.nEncryptedPackages = 0;

	
	params = *((PARAMS_LOAD*)parameter);
	inputFileHandle = params.openedFileHandle;
	printf_s("inputFileHadnle in sender worker is: %p", inputFileHandle);

	nEncryptedPackages = params.nEncryptedPackages;
	status = CreateProtocol(&protocol);

	universalSize = strlen(params.filename);
	filename = (CHAR*)malloc((universalSize + 2)*sizeof(CHAR));
	if(NULL == filename)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	*nEncryptedPackages = 0;
	StringCchCopyA(filename, universalSize + 2, params.filename);
	free((PARAMS_LOAD*)parameter);

	status = protocol.InitializeConnexion(&protocol, filename);
	free(filename);
	if(SUCCESS != status)
	{
		goto Exit;
	}

	fileSize = GetFileSize(inputFileHandle,NULL);
	
	nPackages = fileSize / PACKAGE_SIZE;
	if(fileSize % PACKAGE_SIZE)
	{
		nPackages++;
	}
	printf_s("Size-ul este: %d\n", fileSize);
	for (iPackage = 0; iPackage < nPackages && (SUCCESS == status); iPackage++)
	{
		ReadFile(
			inputFileHandle,						//	_In_        HANDLE       hFile,
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
		printf_s("Send req %d.\n", iPackage);
		status = protocol.SendPackage(&protocol, &request, sizeof(request));

		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send req %d\n", iPackage);
			goto Exit;
		}
		printf_s("Successfully Send req %d\n", iPackage);

		printf_s("Send pack %d", iPackage);
		status = protocol.SendPackage(&protocol, &package, sizeof(package));
		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send pack %d", iPackage);
			goto Exit;
		}
		printf_s("Successfully Send pack %d", iPackage);
		*nEncryptedPackages += 1;
	}
Exit:
	request = FINISH_CONNECTION_REQUEST;
	status = protocol.SendPackage(&protocol, &request, sizeof(request));
	ExitThread(status);
}