#define _CRT_SECURE_NO_WARNINGS 0

#include "Protocol.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>		
#include "Status.h"

typedef struct
{
	HANDLE pipeHandle;
	BOOL closeDueTimeout;
}TIME_OUT_PARAMS;
STATUS WINAPI DisconnectNamedPipeDueTimeout(LPVOID parameters);
STATUS InitializeConnexion(PPROTOCOL protocol, CHAR* fileName);
STATUS ReadPackage(PPROTOCOL protocol, LPVOID buffer, DWORD nNumberOfBytesToRead, DWORD *nNumberOfBytesReaded);
void SetPipeHandle(PPROTOCOL protocol, HANDLE pipeHandle);
HANDLE GetPipeHandle(PPROTOCOL protocol);
STATUS CloseConnexion(PPROTOCOL server);
STATUS OpenNamedPipe(CHAR* fileName, HANDLE* pipeHandle);
STATUS SendPackage(PPROTOCOL protocol, LPVOID message, DWORD nBytesToSend);

STATUS 
CreateProtocol(_In_ PPROTOCOL protocol)
{
	STATUS status;

	status = SUCCESS;

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

HANDLE 
GetPipeHandle(_In_ PPROTOCOL protocol)
{
	return protocol->pipeHandle;
}

void SetPipeHandle(_In_ PPROTOCOL protocol,_In_ HANDLE pipeHandle)
{
	protocol->pipeHandle = pipeHandle;
}

STATUS ReadPackage(PPROTOCOL _In_ protocol, LPVOID _Out_ buffer, DWORD _Out_ nNumberOfBytesToRead,_Out_  DWORD *nNumberOfBytesReaded)
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
		protocol->pipeHandle,			//_In_        HANDLE       hFile,
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

STATUS 
InitializeConnexion(_In_ PPROTOCOL protocol,_In_ CHAR* fileName)
{
	// --- Declarations ---
	char *tempFileName;
	BOOL res;
	STATUS status;
	SECURITY_ATTRIBUTES security;
	HRESULT result;
	// --- Initializations ---
	result = S_OK;
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
	result = StringCchCopyA(tempFileName, MAX_BUFFER_SIZE, PREFIX_NAMED_PIPE);
	if(S_OK != result)
	{
		status = STRING_ERROR;
		goto EXIT;
	}
	result = StringCchCatA(tempFileName, MAX_BUFFER_SIZE, fileName);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto EXIT;
	}
	protocol->pipeHandle = CreateNamedPipeA
		(
			tempFileName,												//_In_     LPCTSTR               lpName,
			PIPE_ACCESS_DUPLEX,											//_In_     DWORD                 dwOpenMode,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,		//_In_     DWORD                 dwPipeMode,
			PIPE_UNLIMITED_INSTANCES,									//_In_     DWORD                 nMaxInstances,
			MAX_MESSAGE_BYTES,											//_In_     DWORD                 nOutBufferSize,
			MAX_MESSAGE_BYTES,											//_In_     DWORD                 nInBufferSize,
			0,															//_In_     DWORD                 nDefaultTimeOut,
			logger.lpSecurityAtributes									//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
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



STATUS  OpenNamedPipe(_In_ CHAR* fileName,_Out_ HANDLE* pipeHandle)
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

	tempString = (CHAR*)malloc(MAX_BUFFER_SIZE * sizeof(CHAR));
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
	*pipeHandle = handle;
Exit:
	free(tempString);
	return status;
}


STATUS CloseConnexion(_Inout_ PPROTOCOL server)
{
	STATUS status;
	BOOL res;

	status = SUCCESS;
	res = TRUE;

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

STATUS
SendPackage(
	_In_ PPROTOCOL protocol,
	_In_ LPVOID message,
	_In_ DWORD nBytesToSend
	)
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
		protocol->pipeHandle,				//	_In_        HANDLE       hFile,
		message,							//	_In_        LPCVOID      lpBuffer,
		nBytesToSend,						//	_In_        DWORD        nNumberOfBytesToWrite,
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

STATUS WINAPI DisconnectNamedPipeDueTimeout(_In_ LPVOID parameters)
{
	STATUS status;
	TIME_OUT_PARAMS* params;

	params = NULL;
	status = SUCCESS;
	if(NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	params = ((TIME_OUT_PARAMS*)parameters);
	Sleep(5000);

	params->closeDueTimeout = TRUE;
	CancelIoEx(params->pipeHandle,NULL);
Exit:
	return status;
}





