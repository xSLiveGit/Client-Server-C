#define _CRT_SECURE_NO_WARNINGS 0

#include "ServerProtocolSide.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>
//STATUS InitializeConnexion(PSERVER_PROTOCOL protocol, char* fileName);
//STATUS CloseConnexion(PSERVER_PROTOCOL serverProtocol);
//STATUS SendNetworkMessage(PSERVER_PROTOCOL serverProtocol, int packetsNumber, PPACKET *packetsList, BOOL tryToDezalloc);
//STATUS ReadNetworkMessage(PSERVER_PROTOCOL serverProtocol,int* packetsNumber, PPACKET *packetsList);
//STATUS ReadUserInformation (PSERVER_PROTOCOL serverProtocol, char** username, char** password);
// #pragma comment (lib, "DllUtil.lib")

// --------------- Helper Function ----------------
STATUS ReadFromPipe(PSERVER_PROTOCOL serverProtocol, char** buffer, char** result)
{
	BOOL res;
	DWORD readedBytes;

	res = ReadFile(
		serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		10,								//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
//	strcpy_s(*result, readedBytes, *buffer);
	memcpy(*result, *buffer, readedBytes);
	if (!res) return COMUNICATION_ERROR;
	return SUCCESS;
}

STATUS InitializeConnexion(PSERVER_PROTOCOL protocol, char* fileName)
{
	// --- Declarations ---
	char *tempFileName;
	BOOL res;
	STATUS status;

	// --- Initializations ---
	status = 0;
	res = TRUE;
	tempFileName = (char*)malloc(4096 * sizeof(char));
	tempFileName[0] = '\0';
	// --- Process ---
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto EXIT;
	}

	protocol->pipeName = fileName;
	//StringCchCopyA(tempFileName, strlen(PREFIX_NAMED_PIPE), PREFIX_NAMED_PIPE);
	strcpy_s(tempFileName,13, PREFIX_NAMED_PIPE);
	// ReSharper disable CppDeprecatedEntity
	strcat(tempFileName, fileName);
	// ReSharper restore CppDeprecatedEntity
	//StringCchCatA(tempFileName, strlen(fileName), fileName);
	protocol->pipeHandle = CreateNamedPipeA
	(
		tempFileName,				//_In_     LPCTSTR               lpName,
		PIPE_ACCESS_DUPLEX,			//_In_     DWORD                 dwOpenMode,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,			//_In_     DWORD                 dwPipeMode,
		PIPE_UNLIMITED_INSTANCES,	//_In_     DWORD                 nMaxInstances,
		4096,						//_In_     DWORD                 nOutBufferSize,
		4096,						//_In_     DWORD                 nInBufferSize,
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
		status = CONNEXION_ERROR;
		goto EXIT;
	}
	// --- Exit/CleanUp --
EXIT:
	free(tempFileName);
	return status;
}



STATUS CloseConnexion(PSERVER_PROTOCOL server)
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
		status |= CONNEXION_ERROR;
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

/*
* This function try to dezalloc the packets of the packetsList
*/
STATUS SendNetworkMessage(PSERVER_PROTOCOL serverProtocol, int packetsNumber, PPACKET *packetsList, BOOL tryToDezalloc)
{
	// --- declaration ---
	STATUS status = SUCCESS;

	char* buffer;
	int indexPacket;
	BOOL res;
	DWORD writedBytes;
	// -- initialization ---
	status = SUCCESS;
	buffer = (char*)malloc(10 * sizeof(char));
	indexPacket = 0;
	res = TRUE;
	writedBytes = 0;

	//send nr of packets
	_itoa_s(packetsNumber, buffer, 10, 10);

	res = WriteFile(
		serverProtocol->pipeHandle,			//_In_        HANDLE       hFile,
		buffer,								//_In_        LPCVOID      lpBuffer,
		strlen(buffer),						//_In_        DWORD        nNumberOfBytesToWrite,
		&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	for (indexPacket = 0; indexPacket < packetsNumber; ++indexPacket)
	{
		res = WriteFile(
			serverProtocol->pipeHandle,			//_In_        HANDLE       hFile,
			packetsList[indexPacket]->buffer,	//_In_        LPCVOID      lpBuffer,
			packetsList[indexPacket]->size,		//_In_        DWORD        nNumberOfBytesToWrite,
			&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		if (!res)
		{
			status = COMUNICATION_ERROR;
			goto Exit;
		}
	}

Exit:
	free(buffer);
	if (tryToDezalloc)
	{
		for (indexPacket = 0; indexPacket < packetsNumber; ++indexPacket)
		{
			free(packetsList[indexPacket]);
		}
	}
	return status;
}



/*
* !!! All elements of packetsList must be dezalocated
*/
STATUS ReadNetworkMessage(PSERVER_PROTOCOL serverProtocol, int* packetsNumber, PPACKET *packetsList)
{
	//declaration
	STATUS status;
	BOOL res;
	char* buffer;
	DWORD readedBytes;
	DWORD writedBytes;
	int indexPacket;
	char tempMessage[4096] = "";

	//initialization
	status = SUCCESS;
	res = TRUE;
	indexPacket = 0;
	buffer = (char*)malloc(5001 * sizeof(char));
	*packetsNumber = 0;
	writedBytes = 0;
	readedBytes = 0;

	//process
	res = ReadFile(
		serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		10,								//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	if (!res)
	{
		status = COMUNICATION_ERROR;
		goto Exit;
	}

	//*packetsNumber = sprintf_s(buffer, 10, "%ul");
	buffer[readedBytes] = '\0';
	*packetsNumber = atoi(buffer);

//	// ------------------ SEND OK MESSAGGE
//	strcpy(buffer, "OK");
//	res = WriteFile(
//		serverProtocol->pipeHandle,			//_In_        HANDLE       hFile,
//		buffer,								//_In_        LPCVOID      lpBuffer,
//		2,									//_In_        DWORD        nNumberOfBytesToWrite,
//		&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
//		NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
//		);
//	if (!res)
//	{
//		status = COMUNICATION_ERROR;
//		goto Exit;
//	}
//	res = FlushFileBuffers(serverProtocol->pipeHandle);
//	if (!res)
//	{
//		status = COMUNICATION_ERROR;
//		goto Exit;
//	}
//	// ------------------ END SEND OK MESSAGGE

	for (indexPacket = 0; indexPacket < *packetsNumber - 1; ++indexPacket)
	{
		res = ReadFile(
			serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
			buffer,							//_Out_       LPVOID       lpBuffer,
			4096,							//_In_        DWORD        nNumberOfBytesToRead,
			&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
			NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);

		if (!res || readedBytes != 4096)
		{
			status |= COMUNICATION_ERROR;
			goto Exit;
		}

		//construct new message. It will be modified for constructing package processed by multiple threads
		packetsList[indexPacket] = (PPACKET)malloc(sizeof(PACKET));
		packetsList[indexPacket]->size = readedBytes;
//		strcpy_s(packetsList[indexPacket]->buffer, readedBytes, buffer);
		memcpy(packetsList[indexPacket]->buffer, buffer, readedBytes);
	}
	res = ReadFile(
		serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		4096,							//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	if (!res)
	{
		status |= COMUNICATION_ERROR;
		goto Exit;
	}
	//buffer[readedBytes] = '\0';
	//construct new message. It will be modified for constructing package processed by multiple threads
	packetsList[indexPacket] = (PPACKET)malloc(sizeof(PACKET));
	packetsList[indexPacket]->size = readedBytes;
//	strcpy(packetsList[indexPacket]->buffer, buffer);
	memcpy(packetsList[indexPacket]->buffer, buffer, readedBytes);

Exit:
	free(buffer);
	return status;
}



STATUS ReadUserInformation(PSERVER_PROTOCOL serverProtocol, char** username, char** password)
{
	STATUS status;
	char* buffer;
	BOOL res;
	DWORD readedBytes;

	buffer = (char*)malloc(10 * sizeof(char));
	status = SUCCESS;
	res = TRUE;
	readedBytes = 0;

	status |= ReadFromPipe(serverProtocol, &buffer, username);
	if (SUCCESS != status)
	{
		goto Exit;
	}

	status |= ReadFromPipe(serverProtocol, &buffer, password);
Exit:
	free(buffer);
	return status;
}





STATUS CreateProtocol(PSERVER_PROTOCOL protocol)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	(*protocol).InitializeConnexion = &InitializeConnexion;
	protocol->CloseConnexion = &CloseConnexion;
	protocol->SendNetworkMessage = &SendNetworkMessage;
	protocol->ReadNetworkMessage = &ReadNetworkMessage;
	protocol->ReadUserInformation = &ReadUserInformation;
	// --- Exit/CleanUp --
	return status;
}



