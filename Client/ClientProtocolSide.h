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

	/**
	*
	*	Parameters:
	*			-	_IN_	PCLIENT_PROTOCOL	serverProtocol
	*			-	_IN_	char*				username - null terminated char*
	*			-	_IN_	char*				password - null terminated char*
	*
	*  Returned:
	*			-	COMUNICATION_ERROR						- if any communication error occurs
	*			-	SUCCESS_LOGIN							- Success logged in
	*			-	FAILED_LOGIN_WRONG_CREDENTIALS			- username or password are wrong
	*			-	FAILED_LOGIN_SERVER_REFUSED_CONNECTION	- server refusez to accept client connexion
	*/
	STATUS(*Login)(struct _PROTOCOL* serverProtocol, char* username, char* password);
	HANDLE pipeHandle;
}CLIENT_PROTOCOL, *PCLIENT_PROTOCOL;

STATUS CreateProtocol(PCLIENT_PROTOCOL protocol);


#endif //! _CLIENT_PROTOCOL_SIDE_H_