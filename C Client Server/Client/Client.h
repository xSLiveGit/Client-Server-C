
#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <Windows.h>
#include "../Client/status.h"
#include "../Client/Protocol.h"


typedef struct _CLIENT
{
	STATUS(*RemoveClient)(struct _CLIENT* pclient);
	STATUS(*OpenConnexion)(struct _CLIENT* pclient);
	STATUS(*Run)(struct _CLIENT* pclient, CHAR* inputFileHandle, CHAR* outputFileHandle,CHAR* encryptionKey,CHAR* username,CHAR* password);
	PPROTOCOL clientProtocol;
	CHAR* pipeName;
}CLIENT, *PCLIENT;

STATUS CreateClient(PCLIENT pclient, CHAR* pipeName);

#endif //! _CLIENT_H_