#define _CRT_SECURE_NO_WARNINGS
#include "status.h"
#include <stdio.h>
#include "server.h"
#include <string.h>
#include <strsafe.h>
#include "Globals.h"
#include <psapi.h>

//	---	Public functions declarations: ---
STATUS OpenConnexion(PSERVER pserver);
STATUS RemoveServer(PSERVER pserver);
STATUS SetStopFlag(PSERVER pserver);
STATUS StartServer(PSERVER pserver);
STATUS IsValidUser(CHAR* username, CHAR* password);
STATUS CreateServer(PSERVER pserver, CHAR* pipeName, CHAR* loggerOutputFilePath);
//	---	End public functions declarations: ---



//	---	Private functions declarations: ---
STATUS CreateSpecialPackage(PSPECIAL_PACKAGE *specialPackage, PPACKAGE package,CHAR* encryptionKey);
STATUS DestroySpecialPackage(PSPECIAL_PACKAGE *specialPackage);
static CHAR globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(CHAR* stringToBeProcessed, CHAR* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKAGE *list, int size, CHAR* encryptionKey);
DWORD WINAPI InstanceThread(LPVOID lpvParam);
STATUS ValidUserStatusToResponse(STATUS status, RESPONSE_TYPE* response);
STATUS CreatePackage(PPACKAGE *package);
STATUS DestroyPackage(PPACKAGE *package);
STATUS EncryptionRoutineForSpecialPackage(LPVOID);
//  ---	End private functions declarations: ---

CHAR* users[][2] = { { "Raul","ParolaRaul" } ,{ "Sergiu","ParolaSergiu" } };
DWORD nUsers = 2;
#define BUFSIZE 4096

typedef struct {
	CHAR fileName[100];
	PMY_BLOCKING_QUEUE blockingQueue;
} PARAMS_LOAD;

STATUS CreatePackage(PPACKAGE *package)
{
	STATUS status;
	PPACKAGE _package;

	status = SUCCESS;
	_package = NULL;

	if(NULL == package)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_package = (PPACKAGE)malloc(sizeof(PACKAGE));
	if(NULL == _package)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	*package = _package;

Exit:
	if(SUCCESS != status)
	{
		*package = NULL;
	}
	return status;
}

STATUS DestroyPackage(PPACKAGE *package)
{
	STATUS status;

	status = SUCCESS;

	if(NULL == package)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	free(*package);
	*package = NULL;
Exit:
	return status;
}

STATUS EncryptionRoutineForSpecialPackage(LPVOID parameter)
{
	STATUS status;
	PSPECIAL_PACKAGE specialPackage;

	status = SUCCESS;
	if(NULL == parameter)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	specialPackage = (PSPECIAL_PACKAGE)parameter;
	status = CryptMessage(specialPackage->package->buffer, specialPackage->encryptionKey, specialPackage->package->size);
	specialPackage->isEncrypted = TRUE;
	//@TODO: aici trebuie sa vad ce fac daca esueaza criptarea
Exit:
	return status;
}

STATUS CreateServer(PSERVER pserver, CHAR* pipeName,CHAR* loggerOutputFilePath)
{
	STATUS status = 0;
	if (NULL == pserver)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	pserver->referenceCounter = 0;
	pserver->pipeName = pipeName;
	pserver->serverProtocol = (PROTOCOL*)malloc(sizeof(PROTOCOL));
	if(NULL == pserver->serverProtocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = CreateProtocol(pserver->serverProtocol);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	status = InitializeLogger(&logger, loggerOutputFilePath);
	if (SUCCESS != status)
	{
		goto Exit;
	}

	status = CreateThreadPool(&pserver->threadPool, &EncryptionRoutineForSpecialPackage);
	pserver->OpenConnexion = &OpenConnexion;
	pserver->RemoveServer = &RemoveServer;
	pserver->SetStopFlag = &SetStopFlag;
	pserver->Run = &StartServer;
	pserver->flagOptions = 0;
Exit:
	return status;
}

STATUS CreateSpecialPackage(PSPECIAL_PACKAGE *specialPackage, PPACKAGE package,CHAR* encryptionKey)
{
	STATUS status;
	PSPECIAL_PACKAGE _specialPackage;

	status = SUCCESS;
	_specialPackage = NULL;

	if((NULL == specialPackage) || (NULL == package))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_specialPackage = (PSPECIAL_PACKAGE)GlobalAlloc(0,sizeof(_specialPackage));
	if(NULL == _specialPackage)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	_specialPackage->isEncrypted = FALSE;
	_specialPackage->package = package;
	_specialPackage->encryptionKey = encryptionKey;
Exit:
	if(SUCCESS == status)
	{
		*specialPackage = _specialPackage;
	}
	return status;
}

STATUS DestroySpecialPackage(PSPECIAL_PACKAGE *specialPackage)
{
	STATUS status;

	status = SUCCESS;

	if(NULL == specialPackage)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = DestroyPackage(&((*specialPackage)->package));
//	free(*specialPackage);
	*specialPackage = NULL;
Exit:
	return status;
}

STATUS CryptMessage(CHAR* stringToBeProcessed, CHAR* encryptionKey, unsigned int size)
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
	keyFroCryptLength = (unsigned int)strlen(encryptionKey);

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
	//while (pserver->referenceCounter);
	//status |= pserver->serverProtocol->CloseConnexion(pserver->serverProtocol);
	free(pserver->serverProtocol);
	pserver->serverProtocol = NULL;
	DestroyLogger(&logger);
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
STATUS StartServer(PSERVER pserver)
{
	int iThread;
	STATUS status;
	BOOL res;
	int packetNumbers;
	PACKAGE package;
	CHAR tempBuffer[20];
	PARAMS_LOAD params;
	DWORD dwThreadId;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD readedBytes;
	PMY_BLOCKING_QUEUE blockingQueue;

	blockingQueue = NULL;
	status = SUCCESS;
	HANDLE hThread[10];
	int hSize = 0;
	int clientPipeIndex;
	clientPipeIndex = 0;
	pserver->referenceCounter = 0;
	//int times = 1;
	iThread = 0;


	while (TRUE)
	{
	StartServer:
		printf_s("Server start new sesion\n");
		logger.Info(&logger,"Server start new sesion");
		status = SUCCESS;
		res = TRUE;
		packetNumbers = 0;
		status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);		
		if (SUCCESS != status)
		{
			logger.Warning(&logger,"Initialize connexion has been failed");
			goto Exit;
		}
		logger.Info(&logger, "Successfully initialize connexion");

		dwThreadId = 0;
		
		printf_s("Successfully initialize conexion - server\n");
		pserver->serverProtocol->ReadPackage(pserver->serverProtocol, &request, sizeof(REQUEST_TYPE), &readedBytes);
		if (INITIALIZE_REQUEST == request)
		{
			printf_s("INITIAILIZE_REQUEST\n");
			if (ON_REFUSED_CONNECTION((pserver->flagOptions)))
			{
				response = REJECTED_CONNECTION_RESPONSE;
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto StartServer;
			}
			clientPipeIndex++;
			logger.Info(&logger,"Accepted connection by initialize request");

			status = CreateMyBlockingQueue(&blockingQueue);
			if (SUCCESS != status)
			{
				response = REJECTED_CONNECTION_RESPONSE;
				logger.Warning(&logger, "CreateBlockingQueue failed");
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto StartServer;
			}

			_itoa(clientPipeIndex, tempBuffer, 10);
			//Create in params.
			StringCchCopyA(params.fileName, sizeof(params.fileName), pserver->pipeName);
			StringCchCatA(params.fileName, sizeof(params.fileName), tempBuffer);
			size_t nr;
			StringCchCopyA(package.buffer, sizeof(package.buffer), params.fileName);
			StringCchLengthA(package.buffer, sizeof(package.buffer), &nr);
			package.size = (DWORD)nr;
			package.size++;
			
			params.fileName[package.size] = '\0';
			params.blockingQueue = blockingQueue;
			

			hThread[hSize] = CreateThread(
				logger.lpSecurityAtributes,              // no security attribute 
				5000000,					// stack size 
				InstanceThread,    // thread proc
				(LPVOID)(&params), // thread parameter 
				0,                 // not suspended 
				&dwThreadId		   // returns thread ID
				);       
			hSize++;
			if (hThread == NULL)
			{
				printf_s("CreateThread failed, GLE=%d.\n", GetLastError());
				response = REJECTED_CONNECTION_RESPONSE;
				logger.Warning(&logger, "Create thread opertaion failed");
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto StartServer;
			}
			response = ACCEPTED_CONNECTION_RESPONSE;
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &package, sizeof(package));
		}
		//times--;
		//if (times == 0)
		//	break;
	}
	for (iThread = 0; iThread < hSize;iThread++)
	{
		if(NULL != hThread[iThread])
		{
			WaitForSingleObject(hThread[iThread], INFINITE);
		}
	}
	printf_s("aici\n");
Exit:
	return status;
}

STATUS CryptAllMessages(PACKAGE *list, int size, CHAR* encryptionKey)
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
*		- _IN_		CHAR*		username - NULL terminated CHAR*
*		- _IN_		CHAR*		password - NULL terminated CHAR*

*	Returns:
*		- VALID_USER				-	if credentials are valid
*		- NULL_POINTER_ERROR		-	if message is NULL
*		- WRONG_CREDENTIALS			-	if credentials are not valid
*/

STATUS IsValidUser(CHAR* username, CHAR* password)
{
	STATUS status;
	int i;

	status = VALID_USER;

	if (NULL == username || NULL == password)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	for (i = 0; i < (int)nUsers; i++)
	{
		printf_s("Username: %s Password: %s\n", username, password);
		if (strcmp(username, users[i][0]) == 0 && (strcmp(password, users[i][1]) == 0))
		{
			status = VALID_USER;
			goto Exit;
		}
	}
	status = WRONG_CREDENTIALS;

Exit:
	return status;
}

/**
* Routine for login
*
* _In_		PSERVER		pserver
*
* Returns:
*			- VALID_USER - if given paramaters are valid username and password
*			- NULL_POINTER_ERROR
*			- MALLOC_FAILED_ERROR - if any malloc failed
*			- WRONG_CREDENTIALS - if username or password are wrong
*
*/
STATUS LoginHandler(PPROTOCOL protocol)
{
	STATUS status;
	PACKAGE package;
	CHAR* username;
	CHAR* password;
	DWORD nReadedBytes;

	status = SUCCESS;
	username = NULL;
	password = NULL;
	printf_s("StartServer login handler\n");
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		printf_s("Readed username failed\n");
		goto Exit;
	}
	printf_s("Readed username successfully\n");

	username = (CHAR*)malloc(package.size * sizeof(CHAR) + sizeof(CHAR));//last byte is for '\0'
	if (NULL == username)
	{
		status = MALLOC_FAILED_ERROR;
		printf("Malloc error\n");
		goto Exit;
	}
	memcpy(username, package.buffer, package.size);
	username[package.size] = '\0';

	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		printf_s("Readed password failed\n");

		goto Exit;
	}
	printf_s("Readed password successfully\n");

	password = (CHAR*)malloc(package.size * sizeof(CHAR) + sizeof(CHAR));//last byte is for '\0'
	if (NULL == username)
	{
		status = MALLOC_FAILED_ERROR;
		printf("Malloc error\n");
		goto Exit;
	}
	memcpy(password, package.buffer, package.size);
	password[package.size] = '\0';

	status = IsValidUser(username, password);

Exit:
	free(username);
	free(password);
	return status;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
	STATUS status;
	BOOL res;
	PARAMS_LOAD params;
	PACKAGE package;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD nReadedBytes;
	LONG nPackagesToSendBack;
	//@TODO temp for testing
	PPROTOCOL protocol;
	CHAR encryptionKey[100];
	PMY_BLOCKING_QUEUE blockingQueue;
	PSPECIAL_PACKAGE specialPackgeForThreadPool;
	PPACKAGE packageForEncrypt;
	INT timeToSleep;
	ULONG li, ls;
	timeToSleep = 10;
	packageForEncrypt = NULL;
	protocol = NULL;
	status = SUCCESS;
	res = TRUE;
	StringCchCopyA(package.buffer, sizeof(package.buffer), "");
	StringCchCopyA(package.optBuffer, sizeof(package.optBuffer), "");
	nReadedBytes = 0;
	nPackagesToSendBack = 0;
	specialPackgeForThreadPool = NULL;
	size_t heapMemory;
	PROCESS_MEMORY_COUNTERS pmc;

	params = *((PARAMS_LOAD*)lpvParam);
	blockingQueue = params.blockingQueue;

	protocol = (PPROTOCOL)malloc(sizeof(PROTOCOL));
	if(NULL == protocol)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	CreateProtocol(protocol);
	protocol->InitializeConnexion(protocol, params.fileName);
	logger.Info(&logger, "New connetion has been esteblished in thread");
	if (!res)
	{
		logger.Warning(&logger,"Connection error in server thread");
		printf("Connection error");
		goto Exit;
	}
	
	GetCurrentThreadStackLimits(
		&li,
		&ls
		);
	printf_s("ci = %ul cs = %ul\n", li, ls);

	while (1)//login request
	{
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			printf("\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount);
			printf("\tPeakWorkingSetSize: 0x%08X\n",
				pmc.PeakWorkingSetSize);
			printf("\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize);
			printf("\tQuotaPeakPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPeakPagedPoolUsage);
			printf("\tQuotaPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPagedPoolUsage);
			printf("\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPeakNonPagedPoolUsage);
			printf("\tQuotaNonPagedPoolUsage: 0x%08X\n",
				pmc.QuotaNonPagedPoolUsage);
			printf("\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage);
			printf("\tPeakPagefileUsage: 0x%08X\n\n\n",
				pmc.PeakPagefileUsage);
		}
		status = protocol->ReadPackage(protocol, &request, sizeof(request), &nReadedBytes);
		printf_s("Readed request in login area.");
		printf_s("code: %d\n", GetLastError());
		if (SUCCESS != status)
		{
			goto Exit;
		}
		if (LOGIN_REQUEST == request)
		{
			printf_s("Is login request\n");
			logger.Info(&logger, "The server has received a login request");
			status = LoginHandler(protocol);
			if ((NULL_POINTER_ERROR == status) || (MALLOC_FAILED_ERROR == status))
			{
				response = REJECTED_CONNECTION_RESPONSE;
				logger.Warning(&logger, "Internar error");
				protocol->SendPackage(protocol, &response, sizeof(response));
				goto Exit;
			}
			ValidUserStatusToResponse(status, &response);
			protocol->SendPackage(protocol, &response, sizeof(response));
			if (VALID_USER == status)
			{
				logger.Info(&logger, "Valid user has been accepted");
				break;
			}
			else
			{
				logger.Info(&logger, "Invalid user");
			}
		}
		else if (FINISH_CONNECTION_REQUEST == request)
		{
			printf("Finish connection\n");
			logger.Info(&logger, "Finish connection request");
			goto Exit;
		}
		else
		{
			logger.Warning(&logger, "WRONG PROTOCOL BEHAVIOR");
			response = WRONG_PROTOCOL_BEHAVIOR_RESPONSE;
			protocol->SendPackage(protocol, &response, sizeof(response));
			goto Exit;
		}
	}

	protocol->ReadPackage(protocol, &encryptionKey, sizeof(encryptionKey), &nReadedBytes);
	encryptionKey[nReadedBytes] = '\0';

	while (1)
	{
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			printf("\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount);
			printf("\tPeakWorkingSetSize: 0x%08X\n",
				pmc.PeakWorkingSetSize);
			printf("\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize);
			printf("\tQuotaPeakPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPeakPagedPoolUsage);
			printf("\tQuotaPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPagedPoolUsage);
			printf("\tQuotaPeakNonPagedPoolUsage: 0x%08X\n",
				pmc.QuotaPeakNonPagedPoolUsage);
			printf("\tQuotaNonPagedPoolUsage: 0x%08X\n",
				pmc.QuotaNonPagedPoolUsage);
			printf("\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage);
			printf("\tPeakPagefileUsage: 0x%08X\n\n\n",
				pmc.PeakPagefileUsage);
		}
		status = protocol->ReadPackage(protocol, &request, sizeof(request), &nReadedBytes);
		if (SUCCESS != status)
		{
			logger.Warning(&logger, "Reading operation FAILED");
			goto Exit;
		}
		if ((LOGOUT_REQUEST == request) || (FINISH_CONNECTION_REQUEST == request))
		{
			logger.Info(&logger, "The server has received a logout request");
			goto Exit;
		}
		if (ENCRYPTED_MESSAGE_REQUEST == request)
		{
			//@TODO verify with __try malloc error
			status = CreatePackage(&packageForEncrypt);
			status = CreateSpecialPackage(&specialPackgeForThreadPool, packageForEncrypt,encryptionKey);
			packageForEncrypt->size = 0;

			logger.Info(&logger, "The server has received an encryption message request");
			status = protocol->ReadPackage(protocol, packageForEncrypt, sizeof(PACKAGE), &nReadedBytes);
			if (SUCCESS != status)
			{
				logger.Info(&logger, "The server could not read the encryption package");
				goto Exit;
			}
			logger.Info(&logger, "The server read the encryption package");

			//@TODO Here we will put the package in a threadpool
			packageForEncrypt->buffer[packageForEncrypt->size] = '\0';
			logger.Info(&logger, "Server is trying to encrypt given message.\n");
			
			blockingQueue->Add(blockingQueue, (LPVOID)specialPackgeForThreadPool);
//			CryptMessage(package.buffer, encryptionKey, package.size);

			nPackagesToSendBack = InterlockedIncrement(&nPackagesToSendBack);
		}
		else if (GET_ENCRYPTED_MESSAGE_REQUEST == request)//AICI II DAM SI MESAJUL OK/WRONG_BEHAVIOR_PROTOCOL si apoi mesajul 
		{
			logger.Info(&logger,"The server has received an encryption request");
			//@TODO Here we will get the package form the list filled by threadpool process
			if (nPackagesToSendBack == 0)
			{
				logger.Warning(&logger, "WRONG PROTOCOL BEHAVIOR. The server has received a encription request.");
				response = WRONG_PROTOCOL_BEHAVIOR_RESPONSE;
				protocol->SendPackage(protocol, &response, sizeof(response));
				goto Exit;
			}
			response = OK_RESPONSE;
			status = protocol->SendPackage(protocol, &response, sizeof(response));
			if (SUCCESS != status)
			{
				logger.Warning(&logger, "The server can not send the ok response for encryption request");
				goto Exit;
			}
			status = blockingQueue->Take(blockingQueue,(LPVOID) &specialPackgeForThreadPool);
			//@TODO: thread status!= succes
			timeToSleep = 10;
//			while(specialPackgeForThreadPool->isEncrypted != TRUE)
//			{
//				Sleep(timeToSleep);
//				timeToSleep += 5;
//			}
			status = protocol->SendPackage(protocol, specialPackgeForThreadPool->package, sizeof(PACKAGE));
			if (SUCCESS != status)
			{
				logger.Warning(&logger, "The server can not send the encrypted package back to client");
				DestroySpecialPackage(&specialPackgeForThreadPool);
				goto Exit;
			}
			DestroySpecialPackage(&specialPackgeForThreadPool);
		}
	}

Exit:
	free(protocol);
	printf_s("Exit thread\n");
	FlushFileBuffers(protocol->pipeHandle);
	DisconnectNamedPipe(protocol->pipeHandle);
//	CloseHandle(protocol->pipeHandle);
	printf("InstanceThread exitting.\n");
	//ExitThread(1);
	return 1;
}

STATUS ValidUserStatusToResponse(STATUS loginStatus, RESPONSE_TYPE* response)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == response)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	if (loginStatus == VALID_USER)
	{
		*response = SUCCESS_LOGIN_RESPONSE;
		goto Exit;
	}
	if (WRONG_CREDENTIALS == loginStatus)
	{
		*response = WRONG_CREDENTIALS_RESPONSE;
	}
Exit:
	return status;
}
