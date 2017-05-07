#define _CRT_SECURE_NO_WARNINGS 0

#include "Protocol.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>		

STATUS 
InitializeConnexion(
	_Inout_ PPROTOCOL protocol,
	_In_ CHAR* fileName);

STATUS 
ReadPackage(
	_In_ PPROTOCOL protocol, 
	_Out_ LPVOID buffer, 
	_In_ DWORD nNumberOfBytesToRead, 
	_Out_ DWORD *nNumberOfBytesReaded);

void 
SetPipeHandle(
	_Inout_ PPROTOCOL protocol,
	_In_ HANDLE pipeHandle);

HANDLE 
GetPipeHandle(
	_In_ PPROTOCOL protocol);

STATUS 
CloseConnexion(
	_Inout_ PPROTOCOL server
	);

STATUS  
OpenNamedPipe(
	_In_ CHAR* fileName, 
	_Out_ HANDLE* pipeHandle);

STATUS 
SendPackage(
	_In_ PPROTOCOL protocol, 
	_In_ LPVOID message, 
	_In_ DWORD nBytesToSend);



STATUS CreateProtocol(
	_Inout_ PPROTOCOL protocol)
{
	STATUS status;

	status = SUCCESS;

	protocol->InitializeConnexion = &InitializeConnexion;
	protocol->CloseConnexion = &CloseConnexion;
	protocol->SendPackage = &SendPackage;
	protocol->ReadPackage = &ReadPackage;
	protocol->GetPipeHandle = &GetPipeHandle;
	protocol->SetPipeHandle = &SetPipeHandle;
	protocol->OpenNamedPipe = &OpenNamedPipe;
	
	return status;
}

HANDLE GetPipeHandle(
	_In_ PPROTOCOL protocol)
{
	return protocol->pipeHandle;
}

void SetPipeHandle(
	_Inout_ PPROTOCOL protocol,
	_In_ HANDLE pipeHandle
	)
{
	protocol->pipeHandle = pipeHandle;
}

STATUS 
ReadPackage(
	_In_ PPROTOCOL protocol,
	_Out_ LPVOID buffer,
	_In_ DWORD nNumberOfBytesToRead,
	_Out_ DWORD *nNumberOfBytesReaded
	)
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

STATUS InitializeConnexion(
	_Inout_ PPROTOCOL protocol, 
	_In_ CHAR* fileName
	)
{
	char *tempFileName;
	BOOL res;
	STATUS status;
	DWORD dwMode;
	HRESULT result;

	result = S_OK;
	status = 0;
	res = TRUE;
	tempFileName = NULL;
	dwMode = 0;

	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto EXIT;
	}

	tempFileName = (char*)malloc(MAX_BUFFER_SIZE * sizeof(char));
	result =  StringCchCopyA(tempFileName, strlen(PREFIX_NAMED_PIPE)+1, PREFIX_NAMED_PIPE);
	if(S_OK != result)
	{
		status = STRING_ERROR;
		goto EXIT;
	}
	StringCchCatA(tempFileName, MAX_BUFFER_SIZE, fileName);
	protocol->pipeName = tempFileName;

	protocol->pipeHandle = CreateFileA(
		protocol->pipeName, // pipe name 
		GENERIC_READ | GENERIC_WRITE,
		0, // no sharing 
		NULL, // default security attributes
		OPEN_EXISTING, // opens existing pipe 
		0, // default attributes 
		NULL);

	dwMode = PIPE_READMODE_MESSAGE;
	res = SetNamedPipeHandleState(
		protocol->pipeHandle, // pipe handle 
		&dwMode, // new pipe mode 
		NULL, // don't set maximum bytes 
		NULL); // don't set maximum time 
	if (!res)
	{
		status = CONNECTION_ERROR;
		goto EXIT;
	}

EXIT:
	free(tempFileName);
	return status;
}



STATUS  OpenNamedPipe(
	_In_ CHAR* fileName, 
	_Out_ HANDLE* pipeHandle
	)
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
	StringCchCatA(tempString,sizeof(tempString), fileName);
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

STATUS 
CloseConnexion(
	_In_ PPROTOCOL server
	)
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

STATUS SendPackage(
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




