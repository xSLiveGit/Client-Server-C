#ifndef _SERVER_PROTOCOL_SIDE_H_
#define _SERVER_PROTOCOL_SIDE_H_
#include "../Server/status.h"
#include <Windows.h>

enum REQUESTS_TYPE {LOGIN,SEND_MESSAGE_FOR_ENCRYPTION};
enum RESPONSE_TYPE {LOGIN_RESPONSE,RECEIVE_MESSAGE_FOR_ENCRYPTION_OK,RECEIVE_MESSAGE_FOR_ENCRYPTION_FAILED};

typedef struct
{
	char username[20];
	char password[20];
} USER,*PUSER;

typedef struct
{
	unsigned int size;
	char content[4097];
} PACKAGE_MESSAGE,*PPACKAGE_MESSAGE;

typedef struct _PROTOCOL
{
	char* pipeName;
	STATUS(*InitializeConnexion) (struct _PROTOCOL*, char* );
	STATUS(*CloseConnexion) (struct _PROTOCOL* protocol);
	STATUS(*ReadNetworkMessage)(struct _PROTOCOL* serverProtocol, int* packetsNumber, PPACKET *packetsList);
	STATUS(*SendNetworkMessage)(struct _PROTOCOL* serverProtocol, int packetsNumber, PPACKET *packetsList, BOOL tryToDezalloc);
	STATUS(*ReadUserInformation)(struct _PROTOCOL* serverProtocol, char** username, char** password);
	HANDLE pipeHandle;
}SERVER_PROTOCOL,*PSERVER_PROTOCOL;

STATUS CreateProtocol(PSERVER_PROTOCOL protocol);

#endif //!_SERVER_PROTOCOL_SIDE_H_