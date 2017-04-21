#include "status.h"
#include <stdio.h>
#include "server.h"

//	---	Public functions declarations: ---
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS Run(PSERVER pserver);
//	---	End public functions declarations: ---


//	---	Private functions declarations: ---
static char globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKET *list, int size, char* encryptionKey);
//  ---	End private functions declarations: ---

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

STATUS CryptMessage(char* stringToBeProcessed,char* encryptionKey,unsigned int size)
{
	unsigned int index;
	unsigned int keyFroCryptLength;
	STATUS status;
	
	if (NULL == stringToBeProcessed || NULL == encryptionKey)
	{
		return status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = SUCCESS;
	keyFroCryptLength = strlen(encryptionKey);

	for (index = 0; index < size; index++)
	{
		stringToBeProcessed[index] ^= encryptionKey[index % keyFroCryptLength];
	}

Exit:
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
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	if(NULL == pserver)
	{
		status |= NULL_POINTER_ERROR;
		goto Exit;
	}
	
	pserver->flagOptions |= REJECT_CLIENTS_FLAG;

	// --- Exit/CleanUp ---
Exit:
	return status;
}

/**
 *		_IN_			PSERVER			pserver - 
 */
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
//	printf("server string before encryption process: %s", list[0].buffer);
	//ExportFirstNCharacters(list[0].buffer, stdout, list[0].size);
	
	CryptAllMessages(list, packetNumbers,globalEncryptionKey);
//	printf("server string after encryption process: %s", list[0].buffer);


	status = pserver->serverProtocol->SendNetworkMessage(pserver->serverProtocol, packetNumbers, &list,TRUE);
	
	if (SUCCESS != status)
	{
		printf_s("Unsuccesfuly send string - server");
		goto Exit;
	}
Exit:
	return status;
}

STATUS CryptAllMessages(PACKET *list,int size,char* encryptionKey)
{
	int index;
	STATUS status;

	status = SUCCESS;

	for (index = 0; index < size && (SUCCESS == status);index++)
	{
		status = CryptMessage(list[index].buffer, encryptionKey, list[index].size);
	}

	return status;
}