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
	CreateProtocol(pserver->serverProtocol);
	pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pipeName);
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

	pserver->serverProtocol->ReadNetworkMessage(pserver->serverProtocol, &packetNumbers, &list);

Exit:
	return status;
}
