#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>
#include <strsafe.h>

#ifndef PACKAGE_SIZE
#define PACKAGE_SIZE MAX_BUFFER_SIZE
#endif //!PACKAGE_SIZE

typedef struct
{
	char filename[100];
	DWORD *nEncryptedPackages;
	HANDLE openedFileHandle;
} PARAMS_LOAD;

STATUS 
OpenConnexion
(
	_In_ PCLIENT pclient
);

STATUS 
RemoveClient(
	_Inout_ PCLIENT pclient
	);

STATUS 
Start(
	_In_ PCLIENT pclient,
	_In_ CHAR* inputFile,
	_In_ CHAR* outputFile,
	_In_ CHAR* encryptionKey,
	_In_ CHAR* username,
	_In_ CHAR* password
	);

STATUS 
LoginHandler(
	_In_ CHAR* username, 
	_In_ CHAR* password, 
	_In_ PPROTOCOL protocol);

STATUS 
CreateClient(
	_Inout_ PCLIENT pclient,
	_In_ CHAR* pipeName
	);

STATUS WINAPI 
ReceiverWorker(
	_In_ LPVOID parameter
	);

STATUS WINAPI 
SenderWorker(
	_In_ LPVOID parameter
	);


STATUS
CreateClient(
	_Inout_ PCLIENT pclient,
	_In_ CHAR* pipeName)
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

STATUS
OpenConnexion
(
	_In_ PCLIENT pclient
)
{
	STATUS status;

	status = pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, pclient->pipeName);

	return status;
}

STATUS
RemoveClient(
	_Inout_ PCLIENT pclient
)
{
	STATUS status;

	status = SUCCESS;

	free(pclient->clientProtocol);
	return status;
}

STATUS Start(
	_In_ PCLIENT pclient, 
	_In_ CHAR* inputFile, 
	_In_ CHAR* outputFile,
	_In_ CHAR* encryptionKey,
	_In_ CHAR* username,
	_In_ CHAR* password)
{
	STATUS status;
	char buffer[MAX_BUFFER_SIZE];
	CHAR newFileName[100];
	HANDLE inputFileHandle;
	HANDLE outputFileHandle;
	DWORD readedBytes;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	PACKAGE package;
	DWORD totalSize;
	DWORD encrysize;
	PARAMS_LOAD *params;
	size_t universalSize;
	HANDLE sentPackageForEncryptHandle;
	HANDLE receivePackageForEncryptHandle;
	STATUS returnedStatusCodeSender;
	DWORD nReceivedPackages;
	DWORD nSentedPackages;
	LPSECURITY_ATTRIBUTES secutiry_attributes;
	HRESULT result;

	result = S_OK;
	inputFileHandle = INVALID_HANDLE_VALUE;
	outputFileHandle = INVALID_HANDLE_VALUE;
	nReceivedPackages = 0;
	returnedStatusCodeSender = SUCCESS;
	status = SUCCESS;
	strcpy(buffer, "");
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	readedBytes = 0;
	package.size = 0;
	totalSize = 0;
	secutiry_attributes = (LPSECURITY_ATTRIBUTES)malloc(sizeof(SECURITY_ATTRIBUTES));
	secutiry_attributes->bInheritHandle = TRUE;
	secutiry_attributes->lpSecurityDescriptor = NULL;
	secutiry_attributes->nLength = sizeof(SECURITY_ATTRIBUTES);

	if(NULL == username || NULL == password || NULL == encryptionKey || NULL == inputFile || NULL == outputFile)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	inputFileHandle = CreateFileA(
		inputFile,				//	_In_     LPCTSTR               lpFileName,
		GENERIC_READ,			//	_In_     DWORD                 dwDesiredAccess,
		FILE_SHARE_READ,		//	_In_     DWORD                 dwShareMode,
		secutiry_attributes,	//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,			//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,	//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL					//_In_opt_ HANDLE                hTemplateFile
		);
	if(INVALID_HANDLE_VALUE == inputFileHandle)
	{
		printf_s("Invalid input file");
		status = FILE_ERROR;
		goto Exit;
	}
	totalSize = GetFileSize(inputFileHandle, &totalSize);

	if(NULL == secutiry_attributes)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	outputFileHandle = CreateFileA(
		outputFile,								//	_In_     LPCTSTR               lpFileName,
		GENERIC_WRITE,							//	_In_     DWORD                 dwDesiredAccess,
		0,										//	_In_     DWORD                 dwShareMode,
		secutiry_attributes,					//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_ALWAYS,							//	_In_     DWORD                 dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,					//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL									//	_In_opt_ HANDLE                hTemplateFile
		);

	if (INVALID_HANDLE_VALUE == outputFileHandle)
	{
		printf_s("Invalid output file\n");
		status = FILE_ERROR;
		goto Exit;
	}

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

	//Verify response
	if (ACCEPTED_CONNECTION_RESPONSE == response)
	{
		pclient->clientProtocol->ReadPackage(pclient->clientProtocol, &package, sizeof(package), &readedBytes);
		package.buffer[package.size] = '\0';
		StringCchCopyA(newFileName, sizeof(newFileName), package.buffer);
		Sleep(1000);
		status = pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, package.buffer);
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
			printf_s("User is allready connected\n");
			goto Exit;
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
	
	//Sender
	params = (PARAMS_LOAD*)malloc(sizeof(PARAMS_LOAD));
	if(NULL == params)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	universalSize = strlen(newFileName) + 3;
	result = StringCchCopyA(params->filename, universalSize, newFileName);
	if(S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	result = StringCchCatA(params->filename, universalSize, "R");
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	params->nEncryptedPackages = &nSentedPackages;
	params->openedFileHandle = inputFileHandle;
	Sleep(500);//Let server to create the news namedpipes
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


	//Receiver
	params = (PARAMS_LOAD*)malloc(sizeof(PARAMS_LOAD));
	if (NULL == params)
	{
		status = MALLOC_FAILED_ERROR;
		TerminateThread(sentPackageForEncryptHandle, status);
		goto Exit;
	}
	universalSize = strlen(newFileName) + 3;
	result = StringCchCopyA(params->filename, universalSize, newFileName);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	result = StringCchCatA(params->filename, universalSize, "W");
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	params->nEncryptedPackages = &nReceivedPackages;
	params->openedFileHandle = outputFileHandle;

	receivePackageForEncryptHandle = CreateThread(
		NULL,
		0,
		ReceiverWorker,
		(LPVOID)params,
		0,
		&returnedStatusCodeSender
		);

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
		TerminateThread(sentPackageForEncryptHandle, THREAD_ERROR);
		TerminateThread(receivePackageForEncryptHandle, THREAD_ERROR);
		printf_s("A problem has been occur during encryption process.");
		goto Exit;
	}
	while(nSentedPackages != nReceivedPackages)//Receiver must must write as much bytes as bytes was sent by sender
	{
		Sleep(25);
	}
	Sleep(100);
	TerminateThread(receivePackageForEncryptHandle, SUCCESS);

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
STATUS
LoginHandler(
	_In_ CHAR* username,
	_In_ CHAR* password,
	_In_ PPROTOCOL protocol
	)
{
	STATUS status;
	PACKAGE package;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nReadedBytes;
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

	package.size = (DWORD)strlen(username);
	memcpy(package.buffer, username, package.size);
	package.buffer[package.size] = '\0';
	status = protocol->SendPackage(protocol, &package, sizeof(package));

	if (SUCCESS != status)
	{
		goto Exit;
	}

	package.size = (DWORD)strlen(password);
	memcpy(package.buffer, password, package.size);
	package.buffer[package.size] = '\0';
	protocol->SendPackage(protocol, &package, sizeof(package));

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
	DWORD *nReadedPackages;
	HRESULT result;

	result = S_OK;
	nReadedBytes = 0;
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
	result = StringCchCopyA(filename, universalSize + 2, params.filename);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}

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

STATUS WINAPI
SenderWorker(
	_In_ LPVOID parameter
	)
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

	for (iPackage = 0; iPackage < nPackages && (SUCCESS == status); iPackage++)
	{
		ReadFile(
			inputFileHandle,							//	_In_        HANDLE       hFile,
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
		status = protocol.SendPackage(&protocol, &request, sizeof(request));

		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send req %d\n", iPackage);
			goto Exit;
		}

		status = protocol.SendPackage(&protocol, &package, sizeof(package));
		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully Send pack %d", iPackage);
			goto Exit;
		}
		*nEncryptedPackages += 1;
	}
Exit:
	request = FINISH_CONNECTION_REQUEST;
	status = protocol.SendPackage(&protocol, &request, sizeof(request));
	ExitThread(status);
}