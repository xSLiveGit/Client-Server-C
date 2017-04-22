#ifndef _CLIENT_PROTOCOL_SIDE_H_
#define _CLIENT_PROTOCOL_SIDE_H_
#include <Windows.h>
#include "status.h"

typedef struct
{
	char username[20];
	char password[20];
} USER, *PUSER;

typedef struct
{
	unsigned int size;
	char content[4097];
} PACKAGE_MESSAGE, *PPACKAGE_MESSAGE;

typedef struct _PROTOCOL
{
	char* pipeName;
	STATUS(*InitializeConnexion) (struct _PROTOCOL* protocol, char* pipeName);
	STATUS(*CloseConnexion) (struct _PROTOCOL* protocol);
	STATUS(*ReadNetworkMessage)(struct _PROTOCOL* serverProtocol, int* packetsNumber, PPACKAGE *packetsList, BOOL tryToDezalloc);
	STATUS(*SendNetworkMessage)(struct _PROTOCOL* serverProtocol, int packetsNumber, PACKAGE *packetsList, BOOL tryToDezalloc);
	STATUS(*Login)(struct _PROTOCOL* serverProtocol, char* username, char* password);
	HANDLE pipeHandle;
}CLIENT_PROTOCOL, *PCLIENT_PROTOCOL;

STATUS CreateProtocol(PCLIENT_PROTOCOL protocol);


#endif //! _CLIENT_PROTOCOL_SIDE_H_