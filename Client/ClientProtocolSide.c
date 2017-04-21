#define _CRT_SECURE_NO_WARNINGS
#include "ClientProtocolSide.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>

STATUS InitializeConnexion(PCLIENT_PROTOCOL protocol, char* fileName);
STATUS CloseConnexion(PCLIENT_PROTOCOL serverProtocol);
STATUS SendNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int packetsNumber, PACKET *packetsList, BOOL tryToDezalloc);
STATUS ReadNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int* packetsNumber, PPACKET *packetsList, BOOL tryToDezalloc);
STATUS Login(PCLIENT_PROTOCOL serverProtocol, char* username, char* password);
// --------------- Helper Function ----------------
STATUS ReadFromPipe(PCLIENT_PROTOCOL serverProtocol, char** buffer, char** result)
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
	strcpy_s(*result, readedBytes, *buffer);

	if (!res) return COMUNICATION_ERROR;
	return SUCCESS;
}

STATUS CreateProtocol(PCLIENT_PROTOCOL protocol)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	protocol->InitializeConnexion = &InitializeConnexion;
	protocol->CloseConnexion = &CloseConnexion;
	protocol->SendNetworkMessage = &SendNetworkMessage;
	protocol->ReadNetworkMessage = &ReadNetworkMessage;
	protocol->Login = &Login;
	// --- Exit/CleanUp --
	return status;
}

STATUS InitializeConnexion(PCLIENT_PROTOCOL protocol, char* fileName)
{
	// --- Declarations ---
	char *tempFileName;
	BOOL res;
	STATUS status;
	DWORD dwMode;
	// --- Initializations ---
	status = 0;
	res = TRUE;
	tempFileName = NULL;
	dwMode = 0;
	// --- Process ---
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto EXIT;
	}

	tempFileName = (char*)malloc(4096 * sizeof(char));
	//StringCchCopyA(tempFileName, strlen(PREFIX_NAMED_PIPE), PREFIX_NAMED_PIPE);
	strcpy_s(tempFileName, 13, PREFIX_NAMED_PIPE);
	// ReSharper disable CppDeprecatedEntity
	strcat(tempFileName, fileName);
	protocol->pipeName = tempFileName;

	// @TODO must create a while(1) statement 
	protocol->pipeHandle = CreateFileA(
		protocol->pipeName,					//	_In_     LPCTSTR               lpFileName,
		GENERIC_READ | GENERIC_WRITE,		//	_In_     DWORD                 dwDesiredAccess,
		0,									//	_In_     DWORD                 dwShareMode,
		NULL,								//	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,						//	_In_     DWORD                 dwCreationDisposition,
		0,									//	_In_     DWORD                 dwFlagsAndAttributes,
		NULL								//	_In_opt_ HANDLE                hTemplateFile
		);

	dwMode = PIPE_READMODE_MESSAGE;
	DWORD nr = 4096;
	SetNamedPipeHandleState(
		protocol->pipeHandle,	    // pipe handle 
		&dwMode,					// new pipe mode 
		&nr,						// don't set maximum bytes 
		NULL);						// don't set maximum time


	/*if (!res)
	{
		status = CONNEXION_ERROR;
		goto EXIT;
	}*/

	// --- Exit/CleanUp --
EXIT:
	return status;
}

STATUS CloseConnexion(PCLIENT_PROTOCOL serverProtocol)
{
	// --- Declarations ---
	STATUS status;
	BOOL res;

	// --- Initializations ---
	status = SUCCESS;
	res = TRUE;

	// --- Process ---
	res = CloseHandle(serverProtocol->pipeHandle);
	if(!res)
	{
		status |= FILE_ERROR;
	}
	serverProtocol->pipeName = "";

	// --- Exit/CleanUp --
	return status;
}

/*
* This function try to dezalloc the packets of the packetsList
*/
STATUS SendNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int packetsNumber, PACKET *packetsList, BOOL tryToDezalloc)
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
			packetsList[indexPacket].buffer,	//_In_        LPCVOID      lpBuffer,
			packetsList[indexPacket].size,		//_In_        DWORD        nNumberOfBytesToWrite,
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
//	if (tryToDezalloc)
//	{
//		for (indexPacket = 0; indexPacket < packetsNumber; ++indexPacket)
//		{
//			free(packetsList[indexPacket]);
//		}
//	}
	return status;
}

STATUS ReadNetworkMessage(PCLIENT_PROTOCOL clientProtocol, int* packetsNumber, PPACKET *packetsList,BOOL tryToDezalloc)
{
	//declaration
	STATUS status;
	BOOL res;
	char* buffer;
	DWORD readedBytes;
	int indexPacket;
//	char tempMessage[4096] = "";

	//initialization
	status = SUCCESS;
	res = TRUE;
	indexPacket = 0;
	buffer = (char*)malloc(5001 * sizeof(char));
	*packetsNumber = 0;

	//process
	res = ReadFile(
		clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		10,								//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	if (!res)
	{
		status |= COMUNICATION_ERROR;
		goto Exit;
	}

//	*packetsNumber = sprintf_s(buffer, 10, "%ul");
	buffer[readedBytes] = '\0';
	*packetsNumber = atoi(buffer);
	*packetsList = (PPACKET)malloc(*packetsNumber * sizeof(PACKET));
	for (indexPacket = 0; indexPacket < *packetsNumber - 1; ++indexPacket)
	{

		res = ReadFile(
			clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
			buffer,							//_Out_       LPVOID       lpBuffer,
			4096,							//_In_        DWORD        nNumberOfBytesToRead,
			&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
			NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);

		if (!res || readedBytes != 4097)
		{
			status |= COMUNICATION_ERROR;
			goto Exit;
		}

		//construct new message. It will be modified for constructing package processed by multiple threads
		packetsList[indexPacket]->size = 4096;
		strcpy_s(packetsList[indexPacket]->buffer, 4096, buffer);
	}

	res = ReadFile(
		clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		4096,							//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	packetsList[indexPacket]->size = readedBytes;
	strcpy_s(packetsList[indexPacket]->buffer, readedBytes, buffer);

Exit:
	free(buffer);
	return status;
}

STATUS Login(PCLIENT_PROTOCOL serverProtocol, char* username, char* password)
{
	//@TODO
	return SUCCESS;
}