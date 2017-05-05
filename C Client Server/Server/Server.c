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
STATUS StartServer(PSERVER pserver,LONG nMaxClients);
STATUS IsValidUser(CHAR* username, CHAR* password);
STATUS CreateServer(PSERVER pserver, CHAR* pipeName, CHAR* loggerOutputFilePath);
//	---	End public functions declarations: ---



//	---	Private functions declarations: ---
DWORD WINAPI ServerReaderWorker(LPVOID parameters);
DWORD WINAPI ServerWriterWorker(LPVOID parameters);
STATUS CreateSpecialPackage(PSPECIAL_PACKAGE *specialPackage, CHAR* encryptionKey);
STATUS DestroySpecialPackage(PSPECIAL_PACKAGE *specialPackage);
static CHAR globalEncryptionKey[] = "encryptionKey";
STATUS CryptMessage(CHAR* stringToBeProcessed, CHAR* encryptionKey, unsigned int size);
STATUS CryptAllMessages(PACKAGE *list, int size, CHAR* encryptionKey);
DWORD WINAPI InstanceThread(LPVOID lpvParam);
STATUS ValidUserStatusToResponse(STATUS status, RESPONSE_TYPE* response);
STATUS CreatePackage(PPACKAGE *package);
STATUS DestroyPackage(PPACKAGE *package);
STATUS EncryptionRoutineForSpecialPackage(LPVOID);
STATUS WINAPI ConsoleCommunicationThread(LPVOID parameters);
//  ---	End private functions declarations: ---

CHAR* users[][2] = { { "Raul","ParolaRaul" } ,{ "Sergiu","ParolaSergiu" } };
DWORD nUsers = 2;
#define BUFSIZE 4096

typedef struct
{
	PSERVER pserver;
} CONSOLE_PARAMS;

typedef struct {
	CHAR fileName[100];
	PMY_BLOCKING_QUEUE blockingQueue;
	PTHREAD_POOL threadPool;
	DWORD *nEncryptedPackage;
	CHAR encryptionKey[100];
	LONG *refCounter;
} PARAMS_LOAD;

STATUS CreatePackage(PPACKAGE *package)
{
	STATUS status;
	PPACKAGE _package;

	status = SUCCESS;
	_package = NULL;

	if (NULL == package)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_package = (PPACKAGE)malloc(sizeof(PACKAGE));
	if (NULL == _package)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	*package = _package;

Exit:
	if (SUCCESS != status)
	{
		*package = NULL;
	}
	return status;
}

STATUS DestroyPackage(PPACKAGE *package)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == package)
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
	if (NULL == parameter)
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

STATUS CreateServer(PSERVER pserver, CHAR* pipeName, CHAR* loggerOutputFilePath)
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
	if (NULL == pserver->serverProtocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = CreateProtocol(pserver->serverProtocol);
	if (SUCCESS != status)
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

STATUS CreateSpecialPackage(PSPECIAL_PACKAGE *specialPackage, CHAR* encryptionKey)
{
	STATUS status;
	PSPECIAL_PACKAGE _specialPackage;

	status = SUCCESS;
	_specialPackage = NULL;

	if ((NULL == specialPackage))
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	_specialPackage = (PSPECIAL_PACKAGE)malloc( sizeof(SPECIAL_PACKAGE));
	if (NULL == _specialPackage)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	_specialPackage->encryptionKey = (CHAR*)malloc((strlen(encryptionKey) + 1) * sizeof(CHAR));
	_specialPackage->isEncrypted = FALSE;
	memcpy(_specialPackage->encryptionKey, encryptionKey, strlen(encryptionKey));
	_specialPackage->encryptionKey[strlen(encryptionKey)] = '\0';
Exit:
	if (SUCCESS == status)
	{
		*specialPackage = _specialPackage;
	}
	return status;
}

STATUS DestroySpecialPackage(PSPECIAL_PACKAGE *specialPackage)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == specialPackage)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = DestroyPackage(&((*specialPackage)->package));
	free((*specialPackage)->encryptionKey);
	free(*specialPackage);
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
	DestroyThreadPool(&(pserver->threadPool));

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
STATUS StartServer(PSERVER pserver,LONG maxClients)
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
	HANDLE readerHandle;
	HANDLE writerHandle;
	HANDLE consoleComunicationThreadHandle;
	CONSOLE_PARAMS consoleParams;
	STATUS consoleCommunicationThread;

	consoleCommunicationThread = SUCCESS;
	consoleComunicationThreadHandle = NULL;
	readerHandle = NULL;
	writerHandle = NULL;
	blockingQueue = NULL;
	status = SUCCESS;
	HANDLE hThread[10];
	int hSize = 0;
	int clientPipeIndex;
	clientPipeIndex = 0;
	pserver->referenceCounter = 0;
	//int times = 1;
	iThread = 0;

	
	pserver->threadPool->Start(pserver->threadPool, 3);
	consoleParams.pserver = pserver;
	consoleComunicationThreadHandle = CreateThread(
		logger.lpSecurityAtributes,              // no security attribute 
		0,							// stack size 
		ConsoleCommunicationThread,			// thread proc
		(&consoleParams),	    // thread parameter 
		0,						// not suspended 
		&consoleCommunicationThread			// returns thread ID
		);
	while (TRUE)
	{
	StartServer:
		printf_s("Server start new sesion\n");
		logger.Info(&logger, "Server start new sesion");
		status = SUCCESS;
		res = TRUE;
		packetNumbers = 0;
		status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
		if(pserver->flagOptions & REJECT_CLIENTS_FLAG == REJECT_CLIENTS_FLAG && SUCCESS != status)
		{
			break;
		}
		if (SUCCESS != status)
		{
			logger.Warning(&logger, "Initialize connexion has been failed");
			goto Exit;
		}
		logger.Info(&logger, "Successfully initialize connexion");

		dwThreadId = 0;

		printf_s("Successfully initialize conexion - server\n");
		pserver->serverProtocol->ReadPackage(pserver->serverProtocol, &request, sizeof(REQUEST_TYPE), &readedBytes);
		if (INITIALIZE_REQUEST == request)
		{
			printf_s("INITIAILIZE_REQUEST\n");
			if (ON_REFUSED_CONNECTION((pserver->flagOptions)) || (maxClients == pserver->referenceCounter))
			{
				response = REJECTED_CONNECTION_RESPONSE;
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto StartServer;
			}
			clientPipeIndex++;
			logger.Info(&logger, "Accepted connection by initialize request");

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
			params.threadPool = pserver->threadPool;
			params.refCounter = &pserver->referenceCounter;

			hThread[hSize] = CreateThread(
				logger.lpSecurityAtributes,              // no security attribute 
				0,					// stack size 
				InstanceThread,    // thread proc
				(&params), // thread parameter 
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
//		times--;
//		if (times == 0)
//			break;
		if (pserver->flagOptions & REJECT_CLIENTS_FLAG == REJECT_CLIENTS_FLAG)
		{
			break;
		}
	}
	for (iThread = 0; iThread < hSize; iThread++)
	{
		if (NULL != hThread[iThread])
		{
			WaitForSingleObject(hThread[iThread], INFINITE);
		}
	}
	TerminateThread(consoleComunicationThreadHandle, SUCCESS);
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
	if (NULL == protocol)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	printf_s("StartServer login handler\n");
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
	PTHREAD_POOL threadPool;
	PARAMS_LOAD serverParamsLoader;
	PARAMS_LOAD clientParamsLoader;
	DWORD nWriterPackageProccessed;
	DWORD nReaderPackageProccessed;
	CHAR* readerFileName;
	CHAR* writerFileName;
	HANDLE readerHandle;
	HANDLE writerHandle;
	DWORD readerExitCode;
	DWORD writerExitCode;
	LONG *refCounter;

	refCounter = NULL;
	readerExitCode = 0;
	writerExitCode = 0;
	readerHandle = NULL;
	writerHandle = NULL;
	threadPool = NULL;
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
	readerFileName = NULL;
	writerFileName = NULL;

	params = *((PARAMS_LOAD*)lpvParam);
	blockingQueue = params.blockingQueue;
	threadPool = params.threadPool;
	refCounter = params.refCounter;
	InterlockedIncrement(refCounter);

	readerFileName = (CHAR*)malloc((strlen(params.fileName) + 3) * sizeof(CHAR));
	if(NULL == readerFileName)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	writerFileName = (CHAR*)malloc((strlen(params.fileName) + 3) * sizeof(CHAR));
	if (NULL == writerFileName)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	protocol = (PPROTOCOL)malloc(sizeof(PROTOCOL));
	if (NULL == protocol)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}
	CreateProtocol(protocol);
	protocol->InitializeConnexion(protocol, params.fileName);
	//logger.Info(&logger, "New connetion has been esteblished in thread");
	if (!res)
	{
		logger.Warning(&logger, "Connection error in server thread");
		printf("Connection error");
		goto Exit;
	}
	
	

	while (1)//login request
	{
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

	//Here i will create the threads for reader and writer
	clientParamsLoader.threadPool = threadPool;
	clientParamsLoader.nEncryptedPackage = &nReaderPackageProccessed;
	clientParamsLoader.blockingQueue = blockingQueue;
	StringCchCopyA(clientParamsLoader.fileName, strlen(params.fileName)+3, params.fileName);
	StringCchCatA(clientParamsLoader.fileName, strlen(params.fileName) + 3, "R");
	StringCchCopyA(clientParamsLoader.encryptionKey, sizeof(params.encryptionKey), encryptionKey);
	readerHandle = CreateThread(
		NULL,              // no security attribute 
		0,						// stack size 
		ServerReaderWorker,			// thread proc
		(&clientParamsLoader),	    // thread parameter 
		0,						// not suspended 
		&readerExitCode			// returns thread ID
		);
	if(NULL == readerHandle || INVALID_HANDLE_VALUE == readerHandle)
	{
		status = THREAD_ERROR;
		goto Exit;
	}

	StringCchCopyA(serverParamsLoader.encryptionKey, sizeof(params.encryptionKey), encryptionKey);
	serverParamsLoader.threadPool = threadPool;
	serverParamsLoader.nEncryptedPackage = &nWriterPackageProccessed;
	serverParamsLoader.blockingQueue = blockingQueue;
	StringCchCopyA(serverParamsLoader.fileName, strlen(params.fileName) + 3, params.fileName);
	StringCchCatA(serverParamsLoader.fileName, strlen(params.fileName) + 3, "W");
	writerHandle = CreateThread(
		NULL,		// no security attribute 
		0,								// stack size 
		ServerWriterWorker,					// thread proc
		(&serverParamsLoader),	// thread parameter 
		0,								// not suspended 
		&writerExitCode					// returns thread ID
		);
	if (NULL == writerHandle || INVALID_HANDLE_VALUE == writerHandle)
	{
		status = THREAD_ERROR;
		response = FAILED_RESPONSE;
		TerminateThread(readerHandle, THREAD_ERROR);
		protocol->SendPackage(protocol, &response, sizeof(response));
		goto Exit;
	}

	WaitForSingleObject(readerHandle, INFINITE);
//	if(readerExitCode != SUCCESS)
//	{
//		status = THREAD_ERROR;
//		response = FAILED_RESPONSE;
//		TerminateThread(readerHandle, THREAD_ERROR);
//		protocol->SendPackage(protocol, &response, sizeof(response));
//		goto Exit;
//	}
	while(nReaderPackageProccessed != nWriterPackageProccessed)//wait for writer worker to resend encripted message
	{
		Sleep(25);
	}
	TerminateThread(writerHandle, SUCCESS);
	
//	if (writerExitCode != SUCCESS)
//	{
//		status = THREAD_ERROR;
//		response = FAILED_RESPONSE;
//		TerminateThread(readerHandle, THREAD_ERROR);
//		protocol->SendPackage(protocol, &response, sizeof(response));
//		goto Exit;
//	}

	response = OK_RESPONSE;
	protocol->SendPackage(protocol, &response, sizeof(response));

Exit:
	InterlockedDecrement(refCounter);
	free(readerFileName);
	free(writerFileName);
	free(protocol);
	printf_s("Exit thread\n");
	FlushFileBuffers(protocol->pipeHandle);
	DisconnectNamedPipe(protocol->pipeHandle);
	DestroyBlockingQueue(&blockingQueue);
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

DWORD WINAPI ServerReaderWorker(LPVOID parameters)
{
	STATUS status;
	PARAMS_LOAD params;
	PTHREAD_POOL threadPool;
	PMY_BLOCKING_QUEUE blockingQueue;
	PROTOCOL protocol;
	REQUEST_TYPE request;
	DWORD nReadedBytes;
	BOOL res;
	PACKAGE package;
	//@TODO temp for testing
	CHAR encryptionKey[100];
	PSPECIAL_PACKAGE specialPackgeForThreadPool;
	PPACKAGE packageForEncrypt;

	nReadedBytes = 0;
	blockingQueue = NULL;
	threadPool = NULL;
	status = SUCCESS;
	params.threadPool = NULL;
	params.blockingQueue = NULL;
	params.fileName[0] = '\0';
	packageForEncrypt = NULL;
	res = TRUE;
	StringCchCopyA(package.buffer, sizeof(package.buffer), "");
	StringCchCopyA(package.optBuffer, sizeof(package.optBuffer), "");
	nReadedBytes = 0;
	specialPackgeForThreadPool = NULL;


	if(NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	logger.Info(&logger,"Server reader was started");
	params = *((PARAMS_LOAD*)parameters);
	blockingQueue = params.blockingQueue;
	threadPool = params.threadPool;
	
	StringCchCopyA(encryptionKey, sizeof(encryptionKey), params.encryptionKey);

	CreateProtocol(&protocol);
	logger.Info(&logger, "Server reader will initialize the connection with");
	logger.Info(&logger, params.fileName);
	protocol.InitializeConnexion(&protocol, params.fileName);
	logger.Info(&logger, "Server initialized the conection");
	(*(params.nEncryptedPackage)) = 0;
	while(TRUE)
	{
		logger.Info(&logger,"Server will read a request");
		status = protocol.ReadPackage(&protocol, &request, sizeof(request), &nReadedBytes);
		logger.Info(&logger, "Server read a request");
		if (ENCRYPTED_MESSAGE_REQUEST == request)
		{
			logger.Info(&logger, "The readed request is ENCRYPTED_MESSAGE_REQUEST");
			//@TODO verify with __try malloc error
			status = CreatePackage(&packageForEncrypt);
			status = CreateSpecialPackage(&specialPackgeForThreadPool, encryptionKey);
			specialPackgeForThreadPool->package = packageForEncrypt;
			packageForEncrypt->size = 0;

			logger.Info(&logger, "The server has received an encryption message request");
			status = protocol.ReadPackage(&protocol, packageForEncrypt, sizeof(PACKAGE), &nReadedBytes);
			if (SUCCESS != status)
			{
				logger.Info(&logger, "The server could not read the encryption package");
				goto Exit;
			}
			logger.Info(&logger, "The server read the encryption package");

			packageForEncrypt->buffer[packageForEncrypt->size] = '\0';
			logger.Info(&logger, "Server is trying to encrypt given message.\n");
			blockingQueue->Add(blockingQueue, specialPackgeForThreadPool);
			//			CryptMessage(package.buffer, encryptionKey, package.size);
			threadPool->Add(threadPool, specialPackgeForThreadPool);
			(*(params.nEncryptedPackage))++;
		}
		else if ((LOGOUT_REQUEST == request) || (FINISH_CONNECTION_REQUEST == request))
		{
			logger.Info(&logger, "The server has received a logout request");
			goto Exit;
		}
		else
		{
			logger.Warning(&logger, "Wrong behavior at reader level");
			status = WRONG_BEHAVIOR;
			goto Exit;
		}
	}

Exit:
	ExitThread(status);
}


DWORD WINAPI ServerWriterWorker(LPVOID parameters)
{
	STATUS status;
	PARAMS_LOAD params;
	PTHREAD_POOL threadPool;
	PMY_BLOCKING_QUEUE blockingQueue;
	PROTOCOL protocol;
	REQUEST_TYPE request;
	DWORD nReadedBytes;
	BOOL res;
	PACKAGE package;
	RESPONSE_TYPE response;
	LONG nPackagesToSendBack;
	//@TODO temp for testing
	PSPECIAL_PACKAGE specialPackgeForThreadPool;
	PPACKAGE packageForEncrypt;
	INT timeToSleep;

	nReadedBytes = 0;
	blockingQueue = NULL;
	threadPool = NULL;
	status = SUCCESS;
	params.threadPool = NULL;
	params.blockingQueue = NULL;
	params.fileName[0] = '\0';
	timeToSleep = 10;
	packageForEncrypt = NULL;
	res = TRUE;
	StringCchCopyA(package.buffer, sizeof(package.buffer), "");
	StringCchCopyA(package.optBuffer, sizeof(package.optBuffer), "");
	nReadedBytes = 0;
	nPackagesToSendBack = 0;
	specialPackgeForThreadPool = NULL;


	if (NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	logger.Info(&logger, "Writer has started");
	params = *((PARAMS_LOAD*)parameters);
	blockingQueue = params.blockingQueue;
	threadPool = params.threadPool;

	logger.Info(&logger, "Writer will create the protocol");
	CreateProtocol(&protocol);
	logger.Info(&logger, "Writer will initialize the connection");

	protocol.InitializeConnexion(&protocol, params.fileName);
	logger.Info(&logger, "The connection has benn initialized");
	(*(params.nEncryptedPackage)) = 0;
	while (1)
	{
		logger.Info(&logger, "The writer will read a package");
		status = protocol.ReadPackage(&protocol, &request, sizeof(request), &nReadedBytes);
		if (SUCCESS != status)
		{
			logger.Warning(&logger, "Reading operation FAILED");
			goto Exit;
		}
		logger.Info(&logger,"A package was readed by writer");
		if (GET_ENCRYPTED_MESSAGE_REQUEST == request)//AICI II DAM SI MESAJUL OK/WRONG_BEHAVIOR_PROTOCOL si apoi mesajul 
		{
			logger.Info(&logger, "The server has received GET_ENCRYPTED_MESSAGE_REQUEST");
			//@TODO Here we will get the package form the list filled by threadpool process
			status = blockingQueue->Take(blockingQueue, &specialPackgeForThreadPool);
			timeToSleep = 10;
			logger.Info(&logger, "Will wait fait package encription");
			while (specialPackgeForThreadPool->isEncrypted != TRUE)
			{
				Sleep(timeToSleep);
				timeToSleep += 5;
			}
			logger.Info(&logger, "Taked a package for ennript");
			//@TODO: thread status!= succes
			response = OK_RESPONSE;
			status = protocol.SendPackage(&protocol, &response, sizeof(response));
			if (SUCCESS != status)
			{
				logger.Warning(&logger, "The server can not send the ok response for encryption request");
				goto Exit;
			}
			
			status = protocol.SendPackage(&protocol, specialPackgeForThreadPool->package, sizeof(PACKAGE));
			(*(params.nEncryptedPackage))++;
			if (SUCCESS != status)
			{
				logger.Warning(&logger, "The server can not send the encrypted package back to client");
				DestroySpecialPackage(&specialPackgeForThreadPool);
				goto Exit;
			}
			DestroySpecialPackage(&specialPackgeForThreadPool);
		}
		else
		{
			status = WRONG_BEHAVIOR;
			goto Exit;
		}
	}

Exit:
	return status;
}

STATUS WINAPI ConsoleCommunicationThread(LPVOID parameters)
{
	STATUS status;
	CONSOLE_PARAMS params;
	PSERVER pserver;
	CHAR infoString[] = { "You can chose 1 of the next options:\n\t\t1 - Show server info\n\t\t2 - Exit\n" };
	pserver = NULL;
	status = SUCCESS;
	BOOL res;
	char c;
	char next;

	next = '\n';
	res = TRUE;
	if(NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	params = *((CONSOLE_PARAMS*)parameters);
	pserver = params.pserver;
	while(TRUE)
	{
		printf_s("%s", infoString);
		scanf_s("%c", &c);
		if(next != '\n')
		{
			do
			{
				scanf_s("%c", &next);			
			} while (next == '\0');
			printf_s("Invalid option. Try again\n");
		}
		else if(c == '1')
		{
			printf_s("Urmeaza sa afisez informatii\n");
		}
		else if(c == '2')
		{
			pserver->SetStopFlag(pserver);
			Sleep(100);//sleep to avoid cancel de the pipe handle while a client request a connection;
			res = CancelIoEx(pserver->serverProtocol->pipeHandle,NULL);
			if(!res)
			{
				printf_s("Operation failed. You should try again.\n");
			}
			//res = CloseHandle(pserver->serverProtocol->pipeHandle);
			goto Exit;
		}
	}

Exit:
	ExitThread(status);
}