#include "status.h"
#include <stdio.h>
#include "server.h"

STATUS CryptMessage(char* originalString, char* encryptionKey, char** encryptedString);
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS Run(PSERVER pserver);

STATUS CreateServer(PSERVER pserver, char* pipeName)
{
	STATUS status = 0;
	if(NULL == pserver)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	pserver->referenceCounter = 0;
	pserver->pipeName = pipeName;
	pserver->serverProtocol = (SERVER_PROTOCOL*)malloc(sizeof(SERVER_PROTOCOL));
	CreateProtocol(pserver->serverProtocol);
	//pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pipeName);
	pserver->OpenConnexion = &OpenConnexion;
	pserver->RemoveServer = &RemoveServer;
	pserver->SetStopFlag = &SetStopFlag;
	pserver->Run = &Run;
Exit:
	return status;
}

STATUS CryptMessage(char* originalString,char* encryptionKey,char** encryptedString)
{
	STATUS status = 0;
	//@TODO
	*encryptedString = originalString;
	return status;
}

STATUS OpenConnexion(PSERVER pserver)
{
	STATUS status;

	status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
	
	return status;
}

STATUS RemoveServer(PSERVER pserver)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	pserver->SetStopFlag(pserver);
	while (pserver->referenceCounter);
	status |= pserver->serverProtocol->CloseConnexion(pserver->serverProtocol);
	free(pserver->serverProtocol);

	// --- Exit/CleanUp ---
	return status;
}

STATUS SetStopFlag(PSERVER pserver)
{
	STATUS status;
	status = SUCCESS;

	if(NULL == pserver)
	{
		status |= NULL_POINTER_ERROR;
		goto Exit;
	}
	
	pserver->flagOptions |= REJECT_CLIENTS_FLAG;
Exit:
	return status;
}

STATUS Run(PSERVER pserver)
{
	STATUS status;
	BOOL res;
	int packetNumbers;
	PPACKET list;

	status = SUCCESS;
	res = TRUE;
	packetNumbers = 0;
	list = NULL;

	status |= pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
	if(SUCCESS != status)
	{
		printf_s("Unsuccesfuly initialize conexion - server");
		goto Exit;
	}

	status = pserver->serverProtocol->ReadNetworkMessage(pserver->serverProtocol, &packetNumbers, &list);
	if (SUCCESS != status)
	{
		printf_s("Unsuccesfuly read string - server");
		goto Exit;
	}
	printf("%s", list[0].buffer);
	status = pserver->serverProtocol->SendNetworkMessage(pserver->serverProtocol, packetNumbers, &list,TRUE);
	if (SUCCESS != status)
	{
		printf_s("Unsuccesfuly send string - server");
		goto Exit;
	}
Exit:
	return status;
}
