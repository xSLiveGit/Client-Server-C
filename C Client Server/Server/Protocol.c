#define _CRT_SECURE_NO_WARNINGS 0

#include "Protocol.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>		

STATUS InitializeConnexion(PPROTOCOL protocol, CHAR* fileName);
STATUS ReadPackage(PPROTOCOL protocol, LPVOID buffer, DWORD nNumberOfBytesToRead, DWORD *nNumberOfBytesReaded);
void SetPipeHandle(PPROTOCOL protocol, HANDLE pipeHandle);
HANDLE GetPipeHandle(PPROTOCOL protocol);
STATUS OpenAndConnectNamedPipe(CHAR* fileName, HANDLE* pipeHandle);
STATUS CloseConnexion(PPROTOCOL server);
STATUS  OpenNamedPipe(CHAR* fileName, HANDLE* pipeHandle);
STATUS SendPackage(PPROTOCOL protocol, LPVOID message, DWORD nBytesToSend);
STATUS CreateProtocol(PPROTOCOL protocol)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	(*protocol).InitializeConnexion = &InitializeConnexion;
	protocol->CloseConnexion = &CloseConnexion;
	protocol->SendPackage = &SendPackage;
	protocol->ReadPackage = &ReadPackage;
	protocol->GetPipeHandle = &GetPipeHandle;
	protocol->SetPipeHandle = &SetPipeHandle;
	protocol->OpenNamedPipe = &OpenNamedPipe;
	// --- Exit/CleanUp --
	return status;
}

HANDLE GetPipeHandle(PPROTOCOL protocol)
{
	return protocol->pipeHandle;
}

void SetPipeHandle(PPROTOCOL protocol, HANDLE pipeHandle)
{
	protocol->pipeHandle = pipeHandle;
}

STATUS ReadPackage(PPROTOCOL protocol, LPVOID buffer, DWORD nNumberOfBytesToRead, DWORD *nNumberOfBytesReaded)
{
	BOOL res;
	STATUS status;

	status = SUCCESS;
	res = TRUE;

	if (NULL == nNumberOfBytesReaded || NULL == buffer)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	res = ReadFile(
		protocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		nNumberOfBytesToRead,			//_In_        DWORD        nNumberOfBytesToRead,
		nNumberOfBytesReaded,			//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	if (!res)
	{
		status = COMUNICATION_ERROR;
	}
Exit:
	return status;
}

STATUS InitializeConnexion(PPROTOCOL protocol, CHAR* fileName)
{
	// --- Declarations ---
	char *tempFileName;
	BOOL res;
	STATUS status;
	SECURITY_ATTRIBUTES security;

	// --- Initializations ---
	status = 0;
	res = TRUE;
	tempFileName = (char*)malloc(MAX_BUFFER_SIZE * sizeof(char));
	tempFileName[0] = '\0';
	security.bInheritHandle = TRUE;
	security.nLength = sizeof(security);
	security.lpSecurityDescriptor = NULL;
	// --- Process ---
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto EXIT;
	}

	protocol->pipeName = fileName;
	strcpy_s(tempFileName, 13, PREFIX_NAMED_PIPE);
	strcat(tempFileName, fileName);
	printf_s("Pipe namefile: %s\n", tempFileName);
	protocol->pipeHandle = CreateNamedPipeA
		(
			tempFileName,				//_In_     LPCTSTR               lpName,
			PIPE_ACCESS_DUPLEX,			//_In_     DWORD                 dwOpenMode,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,			//_In_     DWORD                 dwPipeMode,
			PIPE_UNLIMITED_INSTANCES,	//_In_     DWORD                 nMaxInstances,
			MAX_MESSAGE_BYTES,						//_In_     DWORD                 nOutBufferSize,
			MAX_MESSAGE_BYTES,						//_In_     DWORD                 nInBufferSize,
			0,							//_In_     DWORD                 nDefaultTimeOut,
			NULL						//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
			);

	res = ConnectNamedPipe
		(
			protocol->pipeHandle,
			NULL
			);

	if (!res)
	{
		status = CONNECTION_ERROR;
		goto EXIT;
	}
	// --- Exit/CleanUp --
EXIT:
	free(tempFileName);
	return status;
}



STATUS  OpenNamedPipe(CHAR* fileName, HANDLE* pipeHandle)
{
	STATUS status;
	HANDLE handle;
	BOOL res;
	CHAR* tempString;
	SECURITY_ATTRIBUTES security;
	status = SUCCESS;
	handle = NULL;
	res = TRUE;
	tempString = NULL;

	if (NULL == pipeHandle)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	tempString = (CHAR*)malloc(4096 * sizeof(CHAR));
	if (NULL == tempString)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	StringCchCopyA(tempString, sizeof(tempString), PREFIX_NAMED_PIPE);
	StringCchCatA(tempString, sizeof(tempString), fileName);
	printf("Format string for pipe construction: %s\n", tempString);
	handle = CreateNamedPipeA
		(
			tempString,														//_In_     LPCTSTR               lpName,
			PIPE_ACCESS_DUPLEX,												//_In_     DWORD                 dwOpenMode,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,			//_In_     DWORD                 dwPipeMode,
			PIPE_UNLIMITED_INSTANCES,										//_In_     DWORD                 nMaxInstances,
			PIPE_READMODE_MESSAGE,											//_In_     DWORD                 nOutBufferSize,
			PIPE_READMODE_MESSAGE,											//_In_     DWORD                 nInBufferSize,
			0,																//_In_     DWORD                 nDefaultTimeOut,
			&security														//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
			);
	printf_s("Returned code after the named pipe was created: %d", GetLastError());
	*pipeHandle = handle;
Exit:
	free(tempString);
	return status;
}

STATUS OpenAndConnectNamedPipe(CHAR* fileName, HANDLE* pipeHandle)
{
	STATUS status;
	HANDLE handle;
	BOOL res;
	CHAR* tempString;
	SECURITY_ATTRIBUTES security;

	status = SUCCESS;
	handle = NULL;
	res = TRUE;
	tempString = NULL;
	security.bInheritHandle = TRUE;
	security.nLength = 0;
	security.lpSecurityDescriptor = NULL;
	if (NULL == pipeHandle)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	tempString = (CHAR*)malloc(4096 * sizeof(CHAR));
	if (NULL == tempString)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	StringCchCopyA(tempString, sizeof(tempString), PREFIX_NAMED_PIPE);
	StringCchCatA(tempString, sizeof(tempString), fileName);
	printf("Format string for pipe construction: %s\n", tempString);
	handle = CreateNamedPipeA
		(
			tempString,														//_In_     LPCTSTR               lpName,
			PIPE_ACCESS_DUPLEX,												//_In_     DWORD                 dwOpenMode,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,			//_In_     DWORD                 dwPipeMode,
			PIPE_UNLIMITED_INSTANCES,										//_In_     DWORD                 nMaxInstances,
			PIPE_READMODE_MESSAGE,											//_In_     DWORD                 nOutBufferSize,
			PIPE_READMODE_MESSAGE,											//_In_     DWORD                 nInBufferSize,
			0,																//_In_     DWORD                 nDefaultTimeOut,
			&security															//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
			);
	printf_s("Error connect pipe before connect: %d", GetLastError());
	res = ConnectNamedPipe
		(
			handle,
			NULL
			);
	printf_s("Error connect pipe: %d", GetLastError());
	if (!res)
	{
		status = CONNECTION_ERROR;
		goto Exit;
	}
	*pipeHandle = handle;
Exit:
	free(tempString);
	return status;
}

STATUS CloseConnexion(PPROTOCOL server)
{
	// --- Declarations ---
	STATUS status;
	BOOL res;

	// --- Initializations ---
	status = SUCCESS;
	res = TRUE;

	// --- Process ---
	res = DisconnectNamedPipe(server->pipeHandle);
	if (!res)
	{
		status |= CONNECTION_ERROR;
	}

	res = CloseHandle(server->pipeHandle);
	if (!res)
	{
		status |= FILE_ERROR;
	}

	server->pipeName = "";

	// --- Exit/CleanUp --
	return status;
}


/**
*	Features:
*		- Send a simple message through pipe
*	Parameters:
*		- _IN_		PPROTOCOL		protocol
*		- _IN_		CHAR*					message - NULL terminated CHAR*
*	Returns:
*		- SUCCESS
*		- NULL_POINTER_ERROR if message is NULL
*		- CONNECTION_ERROR if there occurs pipe connexion error
*/

STATUS SendPackage(PPROTOCOL protocol, LPVOID message, DWORD nBytesToSend)
{
	STATUS status;
	BOOL res;
	DWORD readedBytes;

	status = SUCCESS;
	res = TRUE;
	readedBytes = 0;

	if (NULL == message)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	res = WriteFile(
		protocol->pipeHandle,			//	_In_        HANDLE       hFile,
		message,							//	_In_        LPCVOID      lpBuffer,
		nBytesToSend,								//	_In_        DWORD        nNumberOfBytesToWrite,
		&readedBytes,						//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//	_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	if (!res)
	{
		status = CONNECTION_ERROR;
	}

Exit:
	return status;
}

/**
* Parameters:
*		_IN_		PPROTOCOL		protocol
* Returns:
*		SUCCESS
*
*/




