#define _CRT_SECURE_NO_WARNINGS 0

#include "client.h"
#include <stdio.h>
#include <string.h>
STATUS OpenConnexion(PCLIENT pclient);
STATUS RemoveClient(PCLIENT pclient);
STATUS Run(PCLIENT pclient,char*,char*);
STATUS PrintAllMessages(PPACKET list, unsigned long size, FILE* outputHandle);
 
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

STATUS Run(PCLIENT pclient,char* inputFile,char* outputFile)
{
	// --- Declarations ---
	STATUS status;
	int packetsNumber;
	PACKET list[1];
	PPACKET readedList ;
	char buffer[4096];
	PACKET pachet;
	size_t readedCharacters;
	FILE *inputFileHandle;
	FILE *outputFileHandle;
	// --- End declarations ---

	// --- Initializations ---
	status = SUCCESS;
	packetsNumber = 0;
	readedList = NULL;
	strcpy(buffer, "");
	inputFileHandle = NULL;
	outputFileHandle = NULL;
	readedCharacters = 0;
	inputFileHandle = fopen(inputFile, "r");
	outputFileHandle = fopen(outputFile, "w");
	// --- End initializations ---

	// --- Process --
	status |= pclient->OpenConnexion(pclient);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	
	//strcpy(pachet.buffer, buffer);
	readedCharacters = fread(buffer, 1, 4096, inputFileHandle);
	memcpy(pachet.buffer, buffer, readedCharacters);
	pachet.size = readedCharacters;
	list[0] = pachet;
	status |= pclient->clientProtocol->SendNetworkMessage(pclient->clientProtocol, 1, list,FALSE);
	if(SUCCESS != status)
	{
		printf_s("Can't sent.\n");
		goto Exit;
	}
//	printf("client string before encryption process: %s", list[0].buffer);
	pclient->clientProtocol->ReadNetworkMessage(pclient->clientProtocol, &packetsNumber, &readedList, FALSE);
//	printf("client string after encryption process: %s", list[0].buffer);

	PrintAllMessages(readedList, packetsNumber, outputFileHandle);

	if (SUCCESS != status)
	{
		printf_s("Can't receive.\n");
		goto Exit;
	}
	//printf_s("%s", readedList[0].buffer);
	// --- Exit/CleanUp ---
Exit:
	return status;
}

STATUS PrintAllMessages(PPACKET list, unsigned long size, FILE* outputFileHandle)
{
	STATUS status;
	unsigned int i;
	int writedCharacters;

	status = SUCCESS;
	writedCharacters = 0;

	for (i = 0; i < size && (SUCCESS == status); i++)
	{
		fwrite(list[i].buffer, 1, list[i].size, outputFileHandle);
		if(writedCharacters != list[i].size)
		{
			status = FILE_ERROR;
		}
	}

	return status;
}
