#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <Windows.h>
#include "../Client/status.h"
#include "../Client/ClientProtocolSide.h"


typedef struct _CLIENT
{
	STATUS(*RemoveClient)(struct _CLIENT* pclient);
	STATUS(*OpenConnexion)(struct _CLIENT* pclient);
	STATUS(*Run)(struct _CLIENT* pclient, char* inputFileHandle, char* outputFileHandle);
	PCLIENT_PROTOCOL clientProtocol;
	char* pipeName;
}CLIENT, *PCLIENT;

STATUS CreateClient(PCLIENT pclient, char* pipeName);

#endif //! _CLIENT_H_
