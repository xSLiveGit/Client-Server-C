#define _CRT_SECURE_NO_WARNINGS
#include "status.h"
#include <stdio.h>
#include "server.h"
#include <string.h>
#include <strsafe.h>
#include "../Client/client.h"

//	---	Public functions declarations: ---
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS Run(PSERVER pserver);
STATUS IsValidUser(char* username, char* password);
//	---	End public functions declarations: ---

char* users[][2] = { { "Raul","ParolaRaul" } ,{ "Sergiu","ParolaSergiu" } };
DWORD nUsers = 2;
#define BUFSIZE 4096

typedef struct {
	char fileName[100];
} PARAMS_LOAD;

//	---	Private functions declarations: ---
static char globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKAGE *list, int size, char* encryptionKey);
DWORD WINAPI InstanceThread(LPVOID lpvParam);
STATUS ValidUserStatusToResponse(STATUS status, RESPONSE_TYPE* response);

//  ---	End private functions declarations: ---

STATUS CreateServer(PSERVER pserver, char* pipeName)
{
	STATUS status = 0;
	if (NULL == pserver)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	pserver->referenceCounter = 0;
	pserver->pipeName = pipeName;
	pserver->serverProtocol = (PROTOCOL*)malloc(sizeof(PROTOCOL));
	CreateProtocol(pserver->serverProtocol);
	pserver->OpenConnexion = &OpenConnexion;
	pserver->RemoveServer = &RemoveServer;
	pserver->SetStopFlag = &SetStopFlag;
	pserver->Run = &Run;
	pserver->flagOptions = 0;
Exit:
	return status;
}

STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size)
{
	unsigned int index;
	unsigned int keyFroCryptLength;
	STATUS status;

	if (NULL == stringToBeProcessed || NULL == encryptionKey)
	{
		return status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = SUCCESS;
	keyFroCryptLength = strlen(encryptionKey);

	for (index = 0; index < size; index++)
	{
		stringToBeProcessed[index] ^= encryptionKey[index % keyFroCryptLength];
	}

Exit:
	return status;
}

STATUS OpenConnexion(PSERVER pserver)
{
	STATUS status;

	status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);

	return status;
}

STATUS RemoveServer(PSERVER pserver)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	pserver->SetStopFlag(pserver);
	while (pserver->referenceCounter);
	status |= pserver->serverProtocol->CloseConnexion(pserver->serverProtocol);
	free(pserver->serverProtocol);

	// --- Exit/CleanUp ---
	return status;
}

STATUS SetStopFlag(PSERVER pserver)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	if (NULL == pserver)
	{
		status |= NULL_POINTER_ERROR;
		goto Exit;
	}

	pserver->flagOptions |= REJECT_CLIENTS_FLAG;

	// --- Exit/CleanUp ---
Exit:
	return status;
}

/**
*		_IN_			PSERVER			pserver -
*/
STATUS Run(PSERVER pserver)
{
	STATUS status;
	BOOL res;
	int packetNumbers;
	PACKAGE package;
	char  username[4096];
	char  password[4096];
	CHAR tempBuffer[20];
	PARAMS_LOAD params;
	HANDLE hThread;
	DWORD dwThreadId;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD readedBytes;
	SECURITY_ATTRIBUTES security;
	security.bInheritHandle = TRUE;
	security.lpSecurityDescriptor = NULL;
	security.nLength = sizeof(security);
	unsigned size;

	int clientPipeIndex;
	clientPipeIndex = 0;

	while (TRUE)
	{
	Start:
		status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
//		res = ConnectNamedPipe(pserver->serverProtocol->pipeHandle,NULL);
//		if(!res)
//		{
//			printf_s("Connect named pipe error\n");
//			goto Exit;
//		}
		printf_s("Server start new sesion\n");
		status = SUCCESS;
		res = TRUE;
		packetNumbers = 0;

		hThread = NULL;
		dwThreadId = 0;
		printf_s("Server pipe name: %s. Principal handle: %p.\n", pserver->pipeName, pserver->serverProtocol->pipeHandle);

		if (SUCCESS != status)
		{
			goto Exit;
		}

		printf_s("Successfully initialize conexion - server\n");
		pserver->serverProtocol->ReadPackage(pserver->serverProtocol, &request, sizeof(REQUEST_TYPE), &readedBytes);
		if (INITIALIZE_REQUEST == request)
		{
			printf_s("INITIAILIZE_REQUEST\n");
			if (ON_REFUSED_CONNECTION((pserver->flagOptions)))
			{
				response = REJECTED_CONNECTION_RESPONSE;
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto Start;
			}
			clientPipeIndex++;

			printf_s("Accept connection\n");

			_itoa(clientPipeIndex, tempBuffer, 10);
			//Create in params.
			StringCchCopyA(params.fileName, sizeof(params.fileName), pserver->pipeName);
			StringCchCatA(params.fileName, sizeof(params.fileName), tempBuffer);

			StringCchCopyA(package.buffer, sizeof(package.buffer), params.fileName);
			StringCchLengthA(package.buffer, sizeof(package.buffer), &size);
			package.size = (DWORD)size;
			package.size++;
			params.fileName[package.size] = '\0';

			hThread = CreateThread(
				NULL,              // no security attribute 
				5000 * BUFSIZE,    // stack size 
				InstanceThread,    // thread proc
				(LPVOID)(&params), // thread parameter 
				0,                 // not suspended 
				&dwThreadId		   // returns thread ID
				);       

			if (hThread == NULL)
			{
				printf_s("CreateThread failed, GLE=%d.\n", GetLastError());
				response = REJECTED_CONNECTION_RESPONSE;
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto Start;
				return -1;
			}
			response = ACCEPTED_CONNECTION_RESPONSE;
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &package, sizeof(package));
		}
	}
Exit:
	return status;
}

STATUS CryptAllMessages(PACKAGE *list, int size, char* encryptionKey)
{
	int index;
	STATUS status;

	status = SUCCESS;

	for (index = 0; index < size && (SUCCESS == status); index++)
	{
		status = CryptMessage(list[index].buffer, encryptionKey, list[index].size);
	}

	return status;
}


/**
*
*	Features:
*			- Verify if given credentials are valid credentials
*
*	Parameters:
*		- _IN_		char*		username - NULL terminated char*
*		- _IN_		char*		password - NULL terminated char*

*	Returns:
*		- VALID_USER				-	if credentials are valid
*		- NULL_POINTER_ERROR		-	if message is NULL
*		- WRONG_CREDENTIALS			-	if credentials are not valid
*/

STATUS IsValidUser(char* username, char* password)
{
	STATUS status;
	int i;

	status = VALID_USER;

	if (NULL == username || NULL == password)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	for (i = 0; i < (int)nUsers; i++)
	{
		printf_s("Username: %s Password: %s\n", username, password);
		if (strcmp(username, users[i][0]) == 0 && (strcmp(password, users[i][1]) == 0))
		{
			status = VALID_USER;
			goto Exit;
		}
	}
	status = WRONG_CREDENTIALS;

Exit:
	return status;
}

/**
* Routine for login
*
* _In_		PSERVER		pserver
*
* Returns:
*			- VALID_USER - if given paramaters are valid username and password
*			- NULL_POINTER_ERROR
*			- MALLOC_FAILED_ERROR - if any malloc failed
*			- WRONG_CREDENTIALS - if username or password are wrong
*
*/
STATUS LoginHandler(PPROTOCOL protocol)
{
	STATUS status;
	PACKAGE package;
	CHAR* username;
	CHAR* password;
	DWORD nReadedBytes;

	status = SUCCESS;
	username = NULL;
	password = NULL;
	printf_s("Start login handler\n");
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		printf_s("Readed username failed\n");
		goto Exit;
	}
	printf_s("Readed username successfully\n");

	username = (CHAR*)malloc(package.size * sizeof(CHAR) + sizeof(CHAR));//last byte is for '\0'
	if (NULL == username)
	{
		status = MALLOC_FAILED_ERROR;
		printf("Malloc error\n");
		goto Exit;
	}
	memcpy(username, package.buffer, package.size);
	username[package.size] = '\0';

	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		printf_s("Readed password failed\n");

		goto Exit;
	}
	printf_s("Readed password successfully\n");

	password = (CHAR*)malloc(package.size * sizeof(CHAR) + sizeof(CHAR));//last byte is for '\0'
	if (NULL == username)
	{
		status = MALLOC_FAILED_ERROR;
		printf("Malloc error\n");
		goto Exit;
	}
	memcpy(password, package.buffer, package.size);
	password[package.size] = '\0';

	status = IsValidUser(username, password);

Exit:
	free(username);
	free(password);
	return status;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	STATUS status;
	BOOL res;
	PARAMS_LOAD params;
	PACKAGE package;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nReadedBytes;
	UINT32 nPackagesToSendBack;
	//@TODO temp for testing
	PACKAGE packageList[11];
	DWORD packageListSize;
	unsigned long long size;
	PPROTOCOL protocol;

	protocol = NULL;
	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;
	status = SUCCESS;
	res = TRUE;
	StringCchCopyA(package.buffer, sizeof(package.buffer), "");
	StringCchCopyA(package.optBuffer, sizeof(package.optBuffer), "");
	nReadedBytes = 0;
	nPackagesToSendBack = 0;
	packageListSize = 0;

	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}
	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		return (DWORD)-1;
	}
	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	params = *((PARAMS_LOAD*)lpvParam);
	protocol = (PPROTOCOL)malloc(sizeof(PROTOCOL));
	if(NULL == protocol)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	CreateProtocol(protocol);
	protocol->InitializeConnexion(protocol, params.fileName);
//	ConnectNamedPipe(protocol->pipeHandle,NULL);

	printf_s("In thread, the handle is: %p\n", protocol->pipeHandle);
	if (!res)
	{
		printf("Connection error");
		goto Exit;
	}
	
	while (1)//login request
	{
		status = protocol->ReadPackage(protocol, &request, sizeof(request), &nReadedBytes);
		printf_s("Readed request in login area.");
		printf_s("code: %d\n", GetLastError());
		if (SUCCESS != status)
		{
			goto Exit;
		}
		if (LOGIN_REQUEST == request)
		{
			printf_s("Is login request\n");
			status = LoginHandler(protocol);
			if ((NULL_POINTER_ERROR == status) || (MALLOC_FAILED_ERROR == status))
			{
				response = REJECTED_CONNECTION_RESPONSE;
				protocol->SendPackage(protocol, &response, sizeof(response));
				goto Exit;
			}
			ValidUserStatusToResponse(status, &response);
			protocol->SendPackage(protocol, &response, sizeof(response));
			if (VALID_USER == status)
			{
				break;
			}
		}
		else if (FINISH_CONNECTION_REQUEST == request)
		{
			printf("Finish connection\n");
			goto Exit;
		}
		else
		{
			response = WRONG_PROTOCOL_BEHAVIOR_RESPONSE;
			protocol->SendPackage(protocol, &response, sizeof(response));
			goto Exit;
		}
	}

	while (1)
	{
		status = protocol->ReadPackage(protocol, &request, sizeof(request), &nReadedBytes);
		if (SUCCESS != status)
		{
			goto Exit;
		}
		if ((LOGOUT_REQUEST == request) || (FINISH_CONNECTION_REQUEST == request))
		{
			goto Exit;
		}
		if (ENCRYPTED_MESSAGE_REQUEST == request)
		{
			status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
			if (SUCCESS != status)
			{
				goto Exit;
			}
			//@TODO Here we will put the package in a threadpool
			package.buffer[package.size] = '\0';
			printf("Server is trying to encrypt given message.\n");
			CryptMessage(package.buffer, globalEncryptionKey, package.size);
			packageList[packageListSize] = package;
			packageListSize++;
			nPackagesToSendBack++;
		}
		else if (GET_ENCRYPTED_MESSAGE_REQUEST == request)//AICI II DAM SI MESAJUL OK/WRONG_BEHAVIOR_PROTOCOL si apoi mesajul 
		{
			//@TODO Here we will get the package form the list filled by threadpool process
			if (nPackagesToSendBack == 0)
			{
				response = WRONG_PROTOCOL_BEHAVIOR_RESPONSE;
				protocol->SendPackage(protocol, &response, sizeof(response));
				goto Exit;
			}
			response = OK_RESPONSE;
			status = protocol->SendPackage(protocol, &response, sizeof(response));
			if (SUCCESS != status)
			{
				goto Exit;
			}
			status = protocol->SendPackage(protocol, &packageList[0], sizeof(packageList[packageListSize]));
			if (SUCCESS != status)
			{
				goto Exit;
			}
			for (int i = 0; i < packageListSize - 1; i++)
			{
				packageList[i] = packageList[i + 1];
			}
			packageListSize--;
		}
	}

Exit:
	free(protocol);
	printf_s("Exit thread\n");
	FlushFileBuffers(protocol->pipeHandle);
	DisconnectNamedPipe(protocol->pipeHandle);
	CloseHandle(protocol->pipeHandle);
	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	printf("InstanceThread exitting.\n");
	//	ExitThread(1);
	return 1;
}

STATUS ValidUserStatusToResponse(STATUS loginStatus, RESPONSE_TYPE* response)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == response)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	if (loginStatus == VALID_USER)
	{
		*response = SUCCESS_LOGIN_RESPONSE;
		goto Exit;
	}
	if (WRONG_CREDENTIALS == loginStatus)
	{
		*response = WRONG_CREDENTIALS_RESPONSE;
	}
Exit:
	return status;
}
