#ifndef _PROTOCOL_SIDE_H_
#define _PROTOCOL_SIDE_H_
#include "../Client/status.h"
#include <Windows.h>

typedef struct
{
	char username[20];
	char password[20];
} USER, *PUSER;

typedef struct
{
	unsigned int size;
	char content[MAX_BUFFER_SIZE];
} PACKAGE_MESSAGE, *PPACKAGE_MESSAGE;

typedef struct _PROTOCOL
{
	CHAR* pipeName;
	STATUS(*InitializeConnexion) (
		_In_ struct _PROTOCOL*, 
		_In_ CHAR* filename
		);
	
	STATUS(*CloseConnexion) (
		_Inout_ struct _PROTOCOL* protocol
		);

	STATUS(*ReadPackage)(
		_In_ struct _PROTOCOL* serverProtocol,
		_Out_ LPVOID buffer,
		_In_ DWORD nNumberOfBytesToRead,
		_Out_ DWORD *nNumberOfBytesReaded
		);

	STATUS(*SendPackage)(
		_In_ struct _PROTOCOL* protocol,
		_In_ LPVOID message,
		_In_ DWORD nBytesToSend
		);

	STATUS(*OpenNamedPipe)(
		_In_ CHAR* fileName,
		_Out_ HANDLE* pipeHandle);

	void(*SetPipeHandle)(
		_Inout_ struct _PROTOCOL* serverProtocol,
		_Out_ HANDLE pipeHandle);

	HANDLE(*GetPipeHandle)(
		_In_ struct _PROTOCOL* serverProtocol);

	HANDLE pipeHandle;
}PROTOCOL, *PPROTOCOL;
STATUS CreateProtocol(PPROTOCOL protocol);

#endif //!_PROTOCOL_SIDE_H_