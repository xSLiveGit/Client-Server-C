#include "status.h"
#include <stdio.h>
#include "server.h"

//	---	Public functions declarations: ---
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS Run(PSERVER pserver);
STATUS IsValidUser(char* username, char* password);
//	---	End public functions declarations: ---

char* users[][2] = { {"Raul","ParolaRaul"} , {"Sergiu","ParolaSergiu"} };
DWORD nUsers = 2;

//	---	Private functions declarations: ---
static char globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKAGE *list, int size, char* encryptionKey);
//  ---	End private functions declarations: ---

STATUS CreateServer(PSERVER pserver, char* pipeName)
{
	STATUS status = 0;
	if (NULL == pserver)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	pserver->referenceCounter = 0;
	pserver->pipeName = pipeName;
	pserver->serverProtocol = (SERVER_PROTOCOL*)malloc(sizeof(SERVER_PROTOCOL));
	CreateProtocol(pserver->serverProtocol);
	pserver->OpenConnexion = &OpenConnexion;
	pserver->RemoveServer = &RemoveServer;
	pserver->SetStopFlag = &SetStopFlag;
	pserver->Run = &Run;
	pserver->flagOptions = 0;
Exit:
	return status;
}

STATUS CryptMessage(char* stringToBeProcessed, char* encryptionKey, unsigned int size)
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
	if (NULL == pserver)
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
	PPACKAGE list;
	char  username[4096];
	char  password[4096];

	status = SUCCESS;
	res = TRUE;
	packetNumbers = 0;
	list = NULL;


	status |= pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
	if (SUCCESS != status)
	{
		printf_s("Unsuccessfully initialize conexion - server\n");
		goto Exit;
	}
	else
	{
		printf_s("Successfully initialize conexion - server\n");
	}

	status = pserver->serverProtocol->ReadUserInformation(pserver->serverProtocol, username, password,30); 
	if(SUCCESS != status)
	{
		printf("Unsuccessfully login.");
		pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		goto Exit;
	}
	else if((ON_REJECT_CLIENT_FLAG((pserver->flagOptions))))
	{
		pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		goto Exit;
	}
	status = IsValidUser(username, password);
	if(VALID_USER != status)
	{
		if(WRONG_CREDENTIALS == status)
		{
			pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_WRONG_CREDENTIALS_MESSAGE);
		}
		else
		{
			pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE);
		}
		goto Exit;
	}
	pserver->serverProtocol->SendSimpleMessage(pserver->serverProtocol, PERMISED_LOGIN_MESSAGE);

	status = pserver->serverProtocol->ReadNetworkMessage(pserver->serverProtocol, &packetNumbers, &list);
	if (SUCCESS != status)
	{
		printf_s("Unsuccessfully read string - server\n");
		goto Exit;
	}
	else
	{
		printf_s("Successfully read string - server\n");
	}

	printf("Server is trying to encrypt given message.\n");
	CryptAllMessages(list, packetNumbers, globalEncryptionKey);

	status = pserver->serverProtocol->SendNetworkMessage(pserver->serverProtocol, packetNumbers, &list, TRUE);
	printf("Server sent encrypted packages");

	if (SUCCESS != status)
	{
		printf_s("Unsuccessfully send string - server");
		goto Exit;
	}
	
Exit:
	return status;
}

STATUS CryptAllMessages(PACKAGE *list, int size, char* encryptionKey)
{
	int index;
	STATUS status;

	status = SUCCESS;

	for (index = 0; index < size && (SUCCESS == status); index++)
	{
		status = CryptMessage(list[index].buffer, encryptionKey, list[index].size);
	}

	return status;
}


/**
 *
 *	Features:
 *			- Verify if given credentials are valid credentials
 *	
 *	Parameters:
 *		- _IN_		char*		username - NULL terminated char*
 *		- _IN_		char*		password - NULL terminated char*

 *	Returns:
 *		- VALID_USER				-	if credentials are valid
 *		- NULL_POINTER_ERROR		-	if message is NULL
 *		- WRONG_CREDENTIALS			-	if credentials are not valid
 */
STATUS IsValidUser(char* username,char* password)
{
	STATUS status;
	int i;

	status = VALID_USER;

	if(NULL == username || NULL == password)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	for (i = 0; i < nUsers;i++)
	{
		if(strcmp(username,users[i][0]) == 0 && (strcmp(password,users[i][1]) == 0))
		{
			status = VALID_USER;
			goto Exit;
		}
	}
	status = WRONG_CREDENTIALS;

Exit:
	return status;
}