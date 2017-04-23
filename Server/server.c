#include "status.h"
#include <stdio.h>
#include "server.h"

//	---	Public functions declarations: ---
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS Run(PSERVER pserver);
STATUS IsValidUser(char* username, char* password);
//	---	End public functions declarations: ---

char* users[][2] = { {"Raul","ParolaRaul"} , {"Sergiu","ParolaSergiu"} };
DWORD nUsers = 2;
#define BUFSIZE 4096

typedef struct {
	PSERVER pserver;
	HANDLE pipe;
} PARAMS_LOAD;

//	---	Private functions declarations: ---
static char globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKAGE *list, int size, char* encryptionKey);
DWORD WINAPI InstanceThread(LPVOID lpvParam);
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
	pserver->serverProtocol = (SERVER_PROTOCOL*)malloc(sizeof(SERVER_PROTOCOL));
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
	PPACKAGE list;
	char  username[4096];
	char  password[4096];
	PARAMS_LOAD params;
	HANDLE hThread;
	DWORD dwThreadId;
	while(TRUE)
	{
		Start:
		printf_s("Server start new sesion\n");
		status = SUCCESS;
		res = TRUE;
		packetNumbers = 0;
		list = NULL;
		hThread = NULL;
		dwThreadId = 0;
		status |= pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
		if (SUCCESS != status)
		{
			printf_s("Unsuccessfully initialize conexion - server\n");
			goto Exit;
		}
		else
		{
			printf_s("Successfully initialize conexion - server\n");
			params.pserver = pserver;
			params.pipe = pserver->serverProtocol->pipeHandle;
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				InstanceThread,    // thread proc
				(LPVOID)(&params),    // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				printf_s("CreateThread failed, GLE=%d.\n", GetLastError());
				return -1;
			}
			//else CloseHandle(hThread);
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
STATUS IsValidUser(char* username,char* password)
{
	STATUS status;
	int i;

	status = VALID_USER;

	if(NULL == username || NULL == password)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	for (i = 0; i < nUsers;i++)
	{
		if(strcmp(username,users[i][0]) == 0 && (strcmp(password,users[i][1]) == 0))
		{
			status = VALID_USER;
			goto Exit;
		}
	}
	status = WRONG_CREDENTIALS;

Exit:
	return status;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
	STATUS status;
	BOOL res;
	int packetNumbers;
	PPACKAGE list;
	char  username[4096];
	char  password[4096];
	PSERVER pserver;
	PARAMS_LOAD params;

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;
	status = SUCCESS;
	res = TRUE;
	packetNumbers = 0;
	list = NULL;
	pserver = NULL;
	// Do some extra error checking since the app will keep running even if this
	// thread fails.

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

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");

	// The thread's parameter is a handle to a pipe object instance. 
	params = *((PARAMS_LOAD*)lpvParam);
	pserver = params.pserver;
	pserver->serverProtocol->pipeHandle = params.pipe;
	
	//	--------------------------------------------------			START		--------------------------------------------------
	// Loop until done reading
	status = pserver->serverProtocol->ReadUserInformation(pserver->serverProtocol, username, password, 30);
	if (SUCCESS != status)
	{
		printf("Unsuccessfully login.");
		pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		goto Exit;
	}
	else if ((ON_REJECT_CLIENT_FLAG((pserver->flagOptions))))
	{
		pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		goto Exit;
	}
	status = IsValidUser(username, password);
	if (VALID_USER != status)
	{
		if (WRONG_CREDENTIALS == status)
		{
			pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_WRONG_CREDENTIALS_MESSAGE);
		}
		else
		{
			pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		}
		goto Exit;
	}
	pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, PERMISED_LOGIN_MESSAGE);

	status = pserver->serverProtocol->ReadNetworkMessage(pserver->serverProtocol, &packetNumbers, &list);
	if (SUCCESS != status)
	{
		printf_s("Unsuccessfully read string - server\n");
		goto Exit;
	}
	else
	{
		printf_s("Successfully read string - server\n");
	}

	printf("Server is trying to encrypt given message.\n");
	CryptAllMessages(list, packetNumbers, globalEncryptionKey);

	status = pserver->serverProtocol->SendNetworkMessage(pserver->serverProtocol, packetNumbers, &list, TRUE);
	printf("Server sent encrypted packages");

	if (SUCCESS != status)
	{
		printf_s("Unsuccessfully send string - server");
		goto Exit;
	}
	//	--------------------------------------------------		END		--------------------------------------------------
	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 
Exit:
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	printf("InstanceThread exitting.\n");
	ExitThread(1);
	return 1;
}

