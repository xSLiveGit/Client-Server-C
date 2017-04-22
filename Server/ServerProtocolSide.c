#define _CRT_SECURE_NO_WARNINGS 0

#include "ServerProtocolSide.h"
#define PREFIX_NAMED_PIPE "\\\\.\\pipe\\\0"
#include <string.h>
#include <Windows.h>
#include <stdio.h>
#include <strsafe.h>
//STATUS InitializeConnexion(PSERVER_PROTOCOL protocol, char* fileName);
//STATUS CloseConnexion(PSERVER_PROTOCOL serverProtocol);
//STATUS SendNetworkMessage(PSERVER_PROTOCOL serverProtocol, int packetsNumber, PPACKAGE *packetsList, BOOL tryToDezalloc);
//STATUS ReadNetworkMessage(PSERVER_PROTOCOL serverProtocol,int* packetsNumber, PPACKAGE *packetsList);
//STATUS ReadUserInformation (PSERVER_PROTOCOL serverProtocol, char** username, char** password);
// #pragma comment (lib, "DllUtil.lib")

// --------------- Helper Function ----------------
/**
 * 
 * Parameters:
 *		_IN_		PSERVER_PROTOCOL		serverProtocol
 *		_OUT_		char**					buffer - must dezalloc !!! Is not null termination string
 *		_IN_		DWORD					nNumberOfBytesToRead
 *		_OUT_		DWORD*					nNumberOfBytesReaded 
 * 
 * Returns:
 *		- SUCCESS					- Successfully readed from pipe
 *		- NULL_POINTER_ERROR		- 1 or many out params are NULL
 *		- COMUNICATION_ERROR		- Problems of pipe communication have appear
 *
 * Note: 
 *		- If there occurs any error , the memory for buffer is cleaned and the value from buffer adress will be NULL
 */		

STATUS ReadFromPipe(PSERVER_PROTOCOL serverProtocol, char** buffer,DWORD nNumberOfBytesToRead,DWORD *nNumberOfBytesReaded)
{
	BOOL res;
	STATUS status;
	char* localBuffer;

	localBuffer = (char*)malloc(nNumberOfBytesToRead *sizeof(char) + sizeof(char));
	status = SUCCESS;
	res = TRUE;

	if(NULL == nNumberOfBytesReaded || NULL == buffer)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	res = ReadFile(
		serverProtocol->pipeHandle,		//_In_        HANDLE       hFile,
		localBuffer,					//_Out_       LPVOID       lpBuffer,
		nNumberOfBytesToRead,			//_In_        DWORD        nNumberOfBytesToRead,
		nNumberOfBytesReaded,			//_Out_opt_   LPDWORD      lpNumberOfBytesRead,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	if (!res)
	{
		status = COMUNICATION_ERROR;
	}
Exit:
	if(SUCCESS != status)
	{
		*buffer = NULL;
		free(localBuffer);
	}
	else
	{
		*buffer = localBuffer;
	}
	return status;
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
	strcpy_s(tempFileName, 13, PREFIX_NAMED_PIPE);
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
STATUS SendNetworkMessage(PSERVER_PROTOCOL serverProtocol, int packetsNumber, PPACKAGE *packetsList, BOOL tryToDezalloc)
{
	// --- declaration ---
	STATUS status = SUCCESS;

	char* buffer;
	int indexPacket;
	BOOL res;
	DWORD writedBytes;
	DWORD size;
	PPACKAGE packageListWrapper;

	// -- initialization ---
	status = SUCCESS;
	buffer = (char*)malloc(10 * sizeof(char));
	indexPacket = 0;
	res = TRUE;
	writedBytes = 0;
	packageListWrapper = *packetsList;
	size = 0;
	//send nr of packets
	
	_itoa_s(packetsNumber, buffer, 10, 10);
	size = (DWORD)strlen(buffer);

	res = WriteFile(
		serverProtocol->pipeHandle,			//_In_        HANDLE       hFile,
		buffer,								//_In_        LPCVOID      lpBuffer,
		size,								//_In_        DWORD        nNumberOfBytesToWrite,
		&writedBytes,						//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	for (indexPacket = 0; indexPacket < packetsNumber; ++indexPacket)
	{
		res = WriteFile(
			serverProtocol->pipeHandle,					//_In_        HANDLE       hFile,
			packageListWrapper[indexPacket].buffer,		//_In_        LPCVOID      lpBuffer,
			packageListWrapper[indexPacket].size,		//_In_        DWORD        nNumberOfBytesToWrite,
			&writedBytes,								//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL										//_Inout_opt_ LPOVERLAPPED lpOverlapped
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
		free(*packetsList);
		*packetsList = NULL;
	}
	return status;
}



/*
* !!! All elements of packetsList must be dezalocated
*/
STATUS ReadNetworkMessage(PSERVER_PROTOCOL serverProtocol, int* packetsNumber, PPACKAGE *packetsList)
{
	// --------- declaration ---------
	STATUS status;
	BOOL res;
	char* buffer;
	DWORD readedBytes;
	DWORD writedBytes;
	int indexPacket;
	char tempMessage[4096] = "";
	PPACKAGE packageListWrapper;

	// --------- initialization ---------
	status = SUCCESS;
	res = TRUE;
	indexPacket = 0;
	buffer = (char*)malloc(5001 * sizeof(char));
	*packetsNumber = 0;
	writedBytes = 0;
	readedBytes = 0;
	packageListWrapper = NULL;
	// --------- process --------- 
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

	buffer[readedBytes] = '\0';
	*packetsNumber = atoi(buffer);
	packageListWrapper = (PPACKAGE)malloc(*packetsNumber * sizeof(PACKAGE));

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
		//packetsList[indexPacket] = (PPACKAGE)malloc(sizeof(PACKAGE));
		packageListWrapper[indexPacket].size = readedBytes;
		memcpy(packageListWrapper[indexPacket].buffer, buffer, readedBytes);
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
	//construct new message. It will be modified for constructing package processed by multiple threads
	//packetsList[indexPacket] = (PPACKAGE)malloc(sizeof(PACKAGE));
	packageListWrapper[indexPacket].size = readedBytes;
	memcpy(packageListWrapper[indexPacket].buffer, buffer, readedBytes);

	// ------------- Exit ------------
Exit:
	*packetsList = packageListWrapper;
	free(buffer);
	return status;
}

/**
* Features:
*		- Read username and password from pipe
*
* Parameters:
*		- _IN_		PSERVER_PROTOCOL		serverProtocol
*		- _OUT_		char*					username - Is null termination string
*		- _OUT_		char*					password - Is null termination string
*
* Returns:
*		- SUCCESS					- Successfully readed from pipe
*		- NULL_POINTER_ERROR		- 1 or many out params are NULL
*		- COMUNICATION_ERROR		- Problems of pipe communication have appear
*
* Note:
*		- If there occurs any error , the memory for buffer is cleaned and the value from buffer adress will be NULL
*		- username and password will have max 4096 bytes
*/
STATUS ReadUserInformation(PSERVER_PROTOCOL serverProtocol, char* username, char* password,DWORD bufferSize)
{
	STATUS status;
	BOOL res;
	DWORD readedBytes;
	char* buffer;

	status = SUCCESS;
	res = TRUE;
	readedBytes = 0;
	buffer = NULL;

	if(NULL == username || NULL == password)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status |= ReadFromPipe(serverProtocol, &buffer, bufferSize, &readedBytes);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	memcpy(username, buffer, readedBytes);
	username[readedBytes] = '\0';
	free(buffer);	

	status |= ReadFromPipe(serverProtocol, &buffer, bufferSize, & readedBytes);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	memcpy(password, buffer, readedBytes);
	password[readedBytes] = '\0';
	free(buffer);	

Exit:
	return status;
}

/**
 *	Features:
 *		- Send a simple message through pipe
 *	Parameters:
 *		- _IN_		PSERVER_PROTOCOL		serverProtocol
 *		- _IN_		char*					message - NULL terminated char*
 *	Returns:
 *		- SUCCESS
 *		- NULL_POINTER_ERROR if message is NULL
 *		- CONNEXION_ERROR if there occurs pipe connexion error
 */
STATUS SendSimpleMessage (PSERVER_PROTOCOL serverProtocol, char* message)
{
	STATUS status;
	BOOL res;
	DWORD readedBytes;
	DWORD size;

	status = SUCCESS;
	res = TRUE;
	readedBytes = 0;
	size = 0;

	if(NULL == message)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	size = (DWORD)strlen(message);
	res = WriteFile(
		serverProtocol->pipeHandle,			//	_In_        HANDLE       hFile,
		message,							//	_In_        LPCVOID      lpBuffer,
		size,					//	_In_        DWORD        nNumberOfBytesToWrite,
		&readedBytes,						//	_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL								//	_Inout_opt_ LPOVERLAPPED lpOverlapped
	);

	if(!res || readedBytes != strlen(message))
	{
		status = CONNEXION_ERROR;
	}

Exit:
	return status;
}

/**
 * Parameters:
 *		_IN_		PSERVER_PROTOCOL		serverProtocol
 * Returns:
 *		SUCCESS
 *
 */
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
	protocol->SendSimpleMessage = &SendSimpleMessage;
	// --- Exit/CleanUp --
	return status;
}



