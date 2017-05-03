#ifndef _SERVER_H_
#define _SERVER_H_

#include <Windows.h>
#include "../Server/status.h"
#include "../Server/Protocol.h"
#include "Logger.h"
#include "Globals.h"
#define REJECT_CLIENTS_FLAG			1
#define LIMITED_WORKERS_FLAG		(1<<1)

#define ON_LIMITED_WORKERS_FLAG(flg) ( ((flg) & LIMITED_WORKERS_FLAG) == LIMITED_WORKERS_FLAG) 
#define ON_REJECT_CLIENT_FLAG(flg) ( ((flg) & REJECT_CLIENTS_FLAG) == REJECT_CLIENTS_FLAG)


typedef struct _SERVER
{
	STATUS(*RemoveServer)(struct _SERVER* pserver);
	STATUS(*OpenConnexion)(struct _SERVER* pserver);
	STATUS(*SetStopFlag)(struct _SERVER* pserver);
	STATUS(*Run)(struct _SERVER* pserver);
	PPROTOCOL serverProtocol;
	CHAR* pipeName;
	DWORD flagOptions;
	DWORD referenceCounter;
}SERVER, *PSERVER;

STATUS CreateServer(PSERVER pserver, CHAR* pipeName, CHAR* loggerOutputFilePath);



#endif //_SERVER_H_
