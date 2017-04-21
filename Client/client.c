#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>
STATUS OpenConnexion(PCLIENT pclient);
STATUS RemoveClient(PCLIENT pclient);
STATUS Run(PCLIENT pclient);

STATUS CreateClient(PCLIENT pclient, char* pipeName)
{
	STATUS status = 0;
	if (NULL == pclient)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	if(NULL == pipeName)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	pclient->pipeName = pipeName;
	pclient->clientProtocol = (PCLIENT_PROTOCOL)malloc(sizeof(CLIENT_PROTOCOL));
	CreateProtocol(pclient->clientProtocol);
	//pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, pipeName);
	pclient->OpenConnexion = &OpenConnexion;
	pclient->RemoveClient = &RemoveClient;
	pclient->Run = &Run;
Exit:
	return status;
}

STATUS OpenConnexion(PCLIENT pclient)
{
	STATUS status;

	status = pclient->clientProtocol->InitializeConnexion(pclient->clientProtocol, pclient->pipeName);

	return status;
}

STATUS RemoveClient(PCLIENT pclient)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---

	status |= pclient->clientProtocol->CloseConnexion(pclient->clientProtocol);
	free(pclient->clientProtocol);
	// --- Exit/CleanUp ---
	return status;
}

STATUS Run(PCLIENT pclient)
{
	// --- Declarations ---
	STATUS status;
	int packetsNumber;
	// --- Initializations ---
	status = SUCCESS;
	packetsNumber = 0;
	// --- Process ---
	
	status |= pclient->OpenConnexion(pclient);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	PACKET list[1];
	PPACKET readedList = NULL;
	char buffer[4096] = "ana\0";
	PACKET pachet;
	pachet.size = 3;

	// ReSharper disable CppDeprecatedEntity
	strcpy(pachet.buffer, buffer);
	// ReSharper restore CppDeprecatedEntity
	list[0] = pachet;
	status |= pclient->clientProtocol->SendNetworkMessage(pclient->clientProtocol, 1, list,FALSE);
	if(SUCCESS != status)
	{
		printf_s("Can't sent.\n");
		goto Exit;
	}
	pclient->clientProtocol->ReadNetworkMessage(pclient->clientProtocol, &packetsNumber, &readedList, FALSE);
	if (SUCCESS != status)
	{
		printf_s("Can't receive.\n");
		goto Exit;
	}
	printf_s("%s", readedList[0].buffer);
	// --- Exit/CleanUp ---
Exit:
	return status;
}