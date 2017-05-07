#ifndef _SERVER_H_
#define _SERVER_H_

#include <Windows.h>
#include "../Server/status.h"
#include "../Server/Protocol.h"
#include "Logger.h"
#include "Globals.h"
#include "ThreadPool.h"
#include "DynamicVector.h"
#define REJECT_CLIENTS_FLAG			1
#define LIMITED_WORKERS_FLAG		(1<<1)

#define ON_LIMITED_WORKERS_FLAG(flg) ( ((flg) & LIMITED_WORKERS_FLAG) == LIMITED_WORKERS_FLAG) 
#define ON_REJECT_CLIENT_FLAG(flg) ( ((flg) & REJECT_CLIENTS_FLAG) == REJECT_CLIENTS_FLAG)

typedef struct _SPECIAL_PACKAGE {
	BOOL isEncrypted;
	PPACKAGE package;
	CHAR* encryptionKey;
} SPECIAL_PACKAGE, *PSPECIAL_PACKAGE;

typedef enum {ONLINE,OFFLINE} USER_TYPE;

typedef struct
{
	CHAR username[100];
	USER_TYPE type;
	long nEncryptedBytes;
} USER_STATE,*PUSER_STATE;

typedef struct {
	CHAR fileName[100];
	PMY_BLOCKING_QUEUE blockingQueue;
	PTHREAD_POOL threadPool;
	DWORD *nEncryptedPackage;
	CHAR encryptionKey[100];
	LONG *refCounter;
	PDYNAMIC_VECTOR pDynamicVector;
	long *nEncryptedBytes;
} PARAMS_LOAD;
typedef struct _SERVER
{
	STATUS(*RemoveServer)(struct _SERVER* pserver);
	STATUS(*OpenConnexion)(struct _SERVER* pserver);
	STATUS(*SetStopFlag)(struct _SERVER* pserver);
	STATUS(*Run)(struct _SERVER* pserver,LONG nMaxClients,INT nWorkers);
	PPROTOCOL serverProtocol;
	CHAR* pipeName;
	DWORD flagOptions;
	LONG referenceCounter;
	PTHREAD_POOL threadPool;
	PDYNAMIC_VECTOR pdynamicVector;
}SERVER, *PSERVER;

STATUS CreateServer(PSERVER pserver, CHAR* pipeName, CHAR* loggerOutputFilePath);
STATUS InitializeUserState(PUSER_STATE *userState,CHAR* username);

#endif //_SERVER_H_
