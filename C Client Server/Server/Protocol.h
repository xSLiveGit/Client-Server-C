#ifndef _PROTOCOL_SIDE_H_
#define _PROTOCOL_SIDE_H_
#include <Windows.h>
#include "Globals.h"

typedef struct
{
	char username[20];
	char password[20];
} USER, *PUSER;

typedef struct
{
	unsigned int size;
	char content[4096];
} PACKAGE_MESSAGE, *PPACKAGE_MESSAGE;

typedef struct _PROTOCOL
{
	CHAR* pipeName;
	STATUS(*InitializeConnexion) (_In_ struct _PROTOCOL*, CHAR*);
	STATUS(*CloseConnexion) (struct _PROTOCOL* protocol);
	STATUS(*ReadPackage)(struct _PROTOCOL* serverProtocol, LPVOID buffer, DWORD nNumberOfBytesToRead, DWORD *nNumberOfBytesReaded);
	STATUS(*SendPackage)(_In_ struct _PROTOCOL* protocol, LPVOID message, DWORD nBytesToSend);
	HANDLE pipeHandle;
	STATUS(*ReadUserInformation)(struct _PROTOCOL* serverProtocol, CHAR* username, CHAR* password, DWORD bufferSize);
	STATUS(*OpenNamedPipe)(CHAR* fileName, HANDLE* pipeHandle);
	void(*SetPipeHandle)(struct _PROTOCOL* serverProtocol, HANDLE pipeHandle);
	HANDLE(*GetPipeHandle)(struct _PROTOCOL* serverProtocol);
}PROTOCOL, *PPROTOCOL;
STATUS OpenAndConnectNamedPipe(CHAR* fileName, HANDLE* pipeHandle);
STATUS CreateProtocol(PPROTOCOL protocol);

#endif //!_PROTOCOL_SIDE_H_