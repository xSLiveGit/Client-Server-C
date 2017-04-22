#define _CRT_SECURE_NO_WARNINGS
#include "ClientProtocolSide.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>

STATUS InitializeConnexion(PCLIENT_PROTOCOL protocol, char* fileName);
STATUS CloseConnexion(PCLIENT_PROTOCOL serverProtocol);
STATUS SendNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int packetsNumber, PACKAGE *packetsList, BOOL tryToDezalloc);
STATUS ReadNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int* packetsNumber, PPACKAGE *packetsList, BOOL tryToDezalloc);
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
	//strcpy_s(*result, readedBytes, *buffer);
	memcpy(*result, *buffer, readedBytes);
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
	free(tempFileName);
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
	if (!res)
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
STATUS SendNetworkMessage(PCLIENT_PROTOCOL serverProtocol, int packetsNumber, PACKAGE *packetsList, BOOL tryToDezalloc)
{
	// --- declaration ---
	STATUS status = SUCCESS;

	char* buffer;
	int indexPacket;
	BOOL res;
	DWORD writedBytes;
	DWORD readedBytes;
	// -- initialization ---
	status = SUCCESS;
	buffer = (char*)malloc(10 * sizeof(char));
	indexPacket = 0;
	res = TRUE;
	writedBytes = 0;
	readedBytes = 0;

	//send nr of packets
	_itoa_s(packetsNumber, buffer, 10, 10);

	// --------------------------- SEND NUMBERS OF PACKETS
	res = WriteFile(
		serverProtocol->pipeHandle,			//_In_        HANDLE       hFile,
		buffer,								//_In_        LPCVOID      lpBuffer,
		strlen(buffer),						//_In_        DWORD        nNumberOfBytesToWrite,
		&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	if (!res)
	{
		status |= COMUNICATION_ERROR;
		goto Exit;
	}
	res = FlushFileBuffers(serverProtocol->pipeHandle);
	if (!res)
	{
		status = COMUNICATION_ERROR;
		goto Exit;
	}
	// --------------------------- END SEND NUMBERS OF PACKETS

	// --------------------------- RECEIVE OK MESSAGE
	//res = ReadFile(
	//	serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
	//	buffer,							//_Out_       LPVOID       lpBuffer,
	//	10,								//_In_        DWORD        nNumberOfBytesToRead,
	//	&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
	//	NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
	//	);
	//buffer[2] = '\0';
	//if(!res || strcmp(buffer,"ok") != 0)
	//{
	//	status = COMUNICATION_ERROR;
	//	goto Exit;
	//}
	//// --------------------------- END RECEIVE OK MESSAGE


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

STATUS ReadNetworkMessage(PCLIENT_PROTOCOL clientProtocol, int* packetsNumber, PPACKAGE *packetsList, BOOL tryToDezalloc)
{
	//declaration
	STATUS status;
	BOOL res;
	char* buffer;
	DWORD readedBytes;
	int indexPacket;
	PPACKAGE packageListWrapper;
	//	char tempMessage[4096] = "";

	if (NULL == packetsList)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	//initialization
	status = SUCCESS;
	res = TRUE;
	indexPacket = 0;
	buffer = (char*)malloc(40967 * sizeof(char));
	*packetsNumber = 0;
	packageListWrapper = *packetsList;
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
		goto CleanUp;
	}

	buffer[readedBytes] = '\0';
	*packetsNumber = atoi(buffer);
	packageListWrapper = (PPACKAGE)malloc(*packetsNumber * sizeof(PACKAGE));
	for (indexPacket = 0; indexPacket < *packetsNumber - 1; ++indexPacket)
	{

		res = ReadFile(
			clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
			buffer,							//_Out_       LPVOID       lpBuffer,
			4096,							//_In_        DWORD        nNumberOfBytesToRead,
			&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
			NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);

		if (!res || readedBytes != 4096)
		{
			status |= COMUNICATION_ERROR;
			goto CleanUp;
		}

		//construct new message. It will be modified for constructing package processed by multiple threads
		packageListWrapper[indexPacket].size = 4096;
		//strcpy_s(packetsList[indexPacket]->buffer, 4096, buffer);
		memcpy(packageListWrapper[indexPacket].buffer, buffer, 4096);
	}

	res = ReadFile(
		clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		4096,							//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	buffer[readedBytes] = '\0';
	packageListWrapper[indexPacket].size = readedBytes;
	memcpy(packageListWrapper[indexPacket].buffer, buffer, readedBytes);
CleanUp:
	*packetsList = packageListWrapper;
	free(buffer);

Exit:
	return status;
}

/**
 *
 *	Parameters:
 *			-	_IN_	PCLIENT_PROTOCOL	serverProtocol
 *			-	_IN_	char*				username - null terminated char*
 *			-	_IN_	char*				password - null terminated char*
 *  Returned:	
 *			-	COMUNICATION_ERROR						- if any communication error occurs
 *			-	SUCCESS_LOGIN							- Success logged in
 *			-	FAILED_LOGIN_WRONG_CREDENTIALS			- username or password are wrong
 *			-	FAILED_LOGIN_SERVER_REFUSED_CONNECTION	- server refusez to accept client connexion
 *
 */
STATUS Login(PCLIENT_PROTOCOL clientProtocol, char* username, char* password)
{
	BOOL res;
	STATUS status;
	DWORD writedBytes;
	char buffer[10];
	DWORD readedBytes;
	DWORD size;

	res = TRUE;
	status = SUCCESS;
	writedBytes = 0;
	readedBytes = 0;
	size = (DWORD)strlen(username);

	res = WriteFile(
			clientProtocol->pipeHandle,			//_In_        HANDLE       hFile,
			username,							//_In_        LPCVOID      lpBuffer,
			size,					//_In_        DWORD        nNumberOfBytesToWrite,
			&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
	if (!res || writedBytes != strlen(username))
	{
		status = COMUNICATION_ERROR;
		goto Exit;
	}

	size = (DWORD)strlen(password);
	res = WriteFile(
		clientProtocol->pipeHandle,			//_In_        HANDLE       hFile,
		password,							//_In_        LPCVOID      lpBuffer,
		size,								//_In_        DWORD        nNumberOfBytesToWrite,
		&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	if (!res || writedBytes != strlen(password))
	{
		status = COMUNICATION_ERROR;
		goto Exit;
	}

	res = ReadFile(
		clientProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		buffer,							//_Out_       LPVOID       lpBuffer,
		9,								//_In_        DWORD        nNumberOfBytesToRead,
		&readedBytes,					//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	buffer[readedBytes] = '\0';
	if(strcmp(buffer, PERMISED_LOGIN_MESSAGE) == 0)
	{
		status = SUCCESS_LOGIN;
		printf("Succesfully login\n");
	}
	else if(strcmp(buffer, REFUSED_BY_WRONG_CREDENTIALS_MESSAGE) == 0)
	{
		status = FAILED_LOGIN_WRONG_CREDENTIALS;
		printf("Loggin failed. Wrong credentials.\n");
	}
	else if(strcmp(buffer,REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE) == 0)
	{
		status = FAILED_LOGIN_SERVER_REFUSED_CONNECTION;
		printf("Loggin failed caused by server refusing.\n");
	}
	else
	{
		status = COMUNICATION_ERROR;
		printf("Loggin failed caused by comunication error.\n");
	}
Exit:
		return status;
}