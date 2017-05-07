#define _CRT_SECURE_NO_WARNINGS
#include "status.h"
#include <stdio.h>
#include "server.h"
#include <string.h>
#include <strsafe.h>
#include "Globals.h"
#include <psapi.h>
#include "DynamicVector.h"

//	---	Public functions declarations: ---
STATUS OpenConnexion(
	_Inout_ PSERVER pserver);

STATUS RemoveServer(
	_Inout_ PSERVER pserver);

STATUS SetStopFlag(
	_Inout_ PSERVER pserver);

STATUS StartServer(
	_In_ PSERVER pserver, 
	_In_ LONG nMaxClients, 
	_In_ INT nWorkers);

STATUS IsValidUser(
	_In_ CHAR* username, 
	_In_ CHAR* password);

STATUS CreateServer(
	_In_ PSERVER pserver, 
	_In_ CHAR* pipeName, 
	_In_ CHAR* loggerOutputFilePath);
//	---	End public functions declarations: ---



//	---	Private functions declarations: ---
BOOL FindElement(
	_In_ LPVOID item1, 
	_In_ LPVOID item2);

STATUS InitializeUserState(
	_Inout_ PUSER_STATE *userState,
	_In_ CHAR* username);

STATUS DestroyUserState(
	_Inout_ PUSER_STATE* userState);

DWORD WINAPI ServerReaderWorker(
	_In_ LPVOID parameters);

DWORD WINAPI ServerWriterWorker(
	_In_ LPVOID parameters);

STATUS CreateSpecialPackage(
	_Inout_ PSPECIAL_PACKAGE *specialPackage,
	_In_ CHAR* encryptionKey);

STATUS DestroySpecialPackage(
	_Inout_ PSPECIAL_PACKAGE *specialPackage);

static CHAR globalEncryptionKey[] = "encryptionKey";

STATUS CryptMessage(
	_Inout_ CHAR* stringToBeProcessed,
	_In_ CHAR* encryptionKey, 
	_In_ unsigned int size);

STATUS CryptAllMessages(
	_In_ PACKAGE *list, 
	_In_ int size, 
	_In_ CHAR* encryptionKey);

DWORD WINAPI InstanceThread(
	_In_ LPVOID lpvParam);

STATUS ValidUserStatusToResponse(
	_In_ STATUS status,
	_Out_ RESPONSE_TYPE* response);

STATUS CreatePackage(
	_Inout_ PPACKAGE *package);

STATUS DestroyPackage(
	_Inout_ PPACKAGE *package);

STATUS EncryptionRoutineForSpecialPackage(
	_In_ LPVOID);

STATUS WINAPI ConsoleCommunicationThread(
	_In_ LPVOID parameters);
//  ---	End private functions declarations: ---

CHAR* users[][4] = { { "Raul","ParolaRaul" } ,{ "Sergiu","ParolaSergiu" },{ "Ana","Ana" },{ "Dorel","Dorel" } };
DWORD nUsers = 4;
#define BUFSIZE MAX_BUFFER_SIZE



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
	if (SUCCESS != status)
	{
		DestroyLogger(&logger);
		goto Exit;
	}

	status = VectorCreate(&pserver->pdynamicVector);
	if (SUCCESS != status)
	{
		DestroyLogger(&logger);
		DestroyThreadPool(&pserver->threadPool);
		goto Exit;
	}
	pserver->OpenConnexion = &OpenConnexion;
	pserver->RemoveServer = &RemoveServer;
	pserver->SetStopFlag = &SetStopFlag;
	pserver->Run = &StartServer;
	pserver->flagOptions = 0;
Exit:
	return status;
}

STATUS CreateSpecialPackage(
	_Inout_ PSPECIAL_PACKAGE *specialPackage,
	_In_ CHAR* encryptionKey)
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

	_specialPackage = (PSPECIAL_PACKAGE)malloc(sizeof(SPECIAL_PACKAGE));
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

STATUS DestroySpecialPackage(
	_Inout_ PSPECIAL_PACKAGE *specialPackage)
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

STATUS OpenConnexion(
	_Inout_ PSERVER pserver)
{
	STATUS status;

	status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);

	return status;
}

STATUS DestroyElement(LPVOID el)
{
	STATUS status;
	PUSER_STATE userState;

	status = SUCCESS;

	if (NULL == el)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	userState = (PUSER_STATE)el;
	DestroyUserState(&userState);

Exit:
	return status;
}

STATUS RemoveServer(
	_Inout_ PSERVER pserver)
{
	// --- Declarations ---
	STATUS status;

	// --- Initializations ---
	status = SUCCESS;

	// --- Process ---
	pserver->SetStopFlag(pserver);
	DestroyThreadPool(&(pserver->threadPool));

	free(pserver->serverProtocol);
	pserver->serverProtocol = NULL;
	DestroyLogger(&logger);
	VectorDestroy(&pserver->pdynamicVector, &DestroyElement);
	// --- Exit/CleanUp ---
	return status;
}

STATUS SetStopFlag(
	_Inout_ PSERVER pserver)
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
STATUS StartServer(
	_In_ PSERVER pserver,
	_In_ LONG nMaxClients,
	_In_ INT nWorkers)
{
	int iThread;
	STATUS status;
	PACKAGE package;
	CHAR tempBuffer[20];
	PARAMS_LOAD params;
	DWORD dwThreadId;
	REQUEST_TYPE request;
	RESPONSE_TYPE response;
	DWORD readedBytes;
	PMY_BLOCKING_QUEUE blockingQueue;
	BOOL flag;

	blockingQueue = NULL;
	status = SUCCESS;
	HANDLE hThread[10];
	int hSize = 0;
	int clientPipeIndex;
	clientPipeIndex = 0;
	pserver->referenceCounter = 0;
	//int times = 1;
	iThread = 0;


	pserver->threadPool->Start(pserver->threadPool, nWorkers);

	while (TRUE)
	{
	StartServer:
		flag = FALSE;
		printf_s("Server start new sesion\n");
		logger.Info(&logger, "Server start new sesion");
		status = pserver->serverProtocol->InitializeConnexion(pserver->serverProtocol, pserver->pipeName);
		if ((((pserver->flagOptions) & (REJECT_CLIENTS_FLAG)) == (REJECT_CLIENTS_FLAG)) && (SUCCESS != status))
		{
			status = SUCCESS;
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
			params.refCounter = &(pserver->referenceCounter);
			params.pDynamicVector = pserver->pdynamicVector;
			params.flag = &flag;
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
				response = REJECTED_CONNECTION_RESPONSE;
				logger.Warning(&logger, "Create thread opertaion failed");
				pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
				goto StartServer;
			}
			Sleep(50);
			while(TRUE != flag)
			{
				Sleep(10);
			}
			response = ACCEPTED_CONNECTION_RESPONSE;
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &response, sizeof(response));
			pserver->serverProtocol->SendPackage(pserver->serverProtocol, &package, sizeof(package));

		}
		if (((pserver->flagOptions) & (REJECT_CLIENTS_FLAG)) == (REJECT_CLIENTS_FLAG))
		{
			break;
		}
	}
	printf_s("URMEAZA SA ASTEPT THREADURI-LE\n");
	for (iThread = 0; iThread < hSize; iThread++)
	{
		printf_s("Threadul %d\n", iThread);
		if (NULL != hThread[iThread])
		{
			WaitForSingleObject(hThread[iThread], INFINITE);
			hThread[iThread] = NULL;
		}
	}
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

STATUS IsValidUser(
	_In_ CHAR* username,
	_In_ CHAR* password)
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
STATUS LoginHandler(PPROTOCOL protocol, CHAR* usernameP)
{
	STATUS status;
	PACKAGE package;
	CHAR* username;
	CHAR* password;
	DWORD nReadedBytes;

	status = SUCCESS;
	username = NULL;
	password = NULL;
	if (NULL == protocol || NULL == usernameP)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		logger.Warning(&logger, "Failed to read username");
		goto Exit;
	}

	username = (CHAR*)malloc(package.size * sizeof(CHAR) + sizeof(CHAR));//last byte is for '\0'
	if (NULL == username)
	{
		status = MALLOC_FAILED_ERROR;
		printf("Malloc error\n");
		logger.Warning(&logger, "Malloc error");
		goto Exit;
	}
	memcpy(username, package.buffer, package.size);
	username[package.size] = '\0';

	status = protocol->ReadPackage(protocol, &package, sizeof(package), &nReadedBytes);
	if (SUCCESS != status)
	{
		logger.Warning(&logger, "Read password failed\n");
		printf_s("Read password failed\n");
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
	StringCchCopyA(usernameP, strlen(username), username);
	status = IsValidUser(username, password);

Exit:
	free(username);
	free(password);
	return status;
}

DWORD WINAPI InstanceThread(
	_In_ LPVOID lpvParam)
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
	PDYNAMIC_VECTOR pDynamicVector;
	CHAR username[100];
	INT position;
	PUSER_STATE userState;
	PUSER_STATE userStateAux;
	HRESULT result;

	result = S_OK;
	userStateAux = NULL;
	userState = NULL;
	position = -1;
	pDynamicVector = NULL;
	refCounter = NULL;
	readerExitCode = 0;
	writerExitCode = 0;
	readerHandle = NULL;
	writerHandle = NULL;
	threadPool = NULL;
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
	pDynamicVector = params.pDynamicVector;
	InterlockedIncrement(refCounter);

	readerFileName = (CHAR*)malloc((strlen(params.fileName) + 3) * sizeof(CHAR));
	if (NULL == readerFileName)
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
	*(params.flag) = TRUE;
	protocol->InitializeConnexion(protocol, params.fileName);
	logger.Info(&logger, "A new connetion has been esteblished in thread");
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
			logger.Info(&logger, "The server has received a login request");
			status = LoginHandler(protocol, username);
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
				status = InitializeUserState(&userState, username);
				if (SUCCESS != status)
				{
					goto Exit;
				}
				status = VectorSearch(pDynamicVector, userState, &position, &FindElement);
				if (SUCCESS == status)
				{
					DestroyUserState(&userState);
					userState = NULL;
					status = VectorGet(*pDynamicVector, position, &userState);
					if (userState->type == OFFLINE)
					{
						userState->type = ONLINE;
						break;
					}
					else
					{
						printf_s("Already connected\n");
						response = ALREADY_CONNECTED_RESPONSE;
						protocol->SendPackage(protocol, &response, sizeof(response));
						//						goto Exit;
					}
				}
				else if (ELEMENT_NOT_FOUND == status)
				{
					VectorAdd(pDynamicVector, userState);
				}
				else
				{
					goto Exit;
				}
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
	result = StringCchCopyA(clientParamsLoader.fileName, strlen(params.fileName) + 3, params.fileName);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	result = StringCchCatA(clientParamsLoader.fileName, strlen(params.fileName) + 3, "R");
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	result = StringCchCopyA(clientParamsLoader.encryptionKey, sizeof(params.encryptionKey), encryptionKey);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}

	readerHandle = CreateThread(
		NULL,              // no security attribute 
		0,						// stack size 
		ServerReaderWorker,			// thread proc
		(&clientParamsLoader),	    // thread parameter 
		0,						// not suspended 
		&readerExitCode			// returns thread ID
		);
	if (NULL == readerHandle || INVALID_HANDLE_VALUE == readerHandle)
	{
		status = THREAD_ERROR;
		goto Exit;
	}

	result = StringCchCopyA(serverParamsLoader.encryptionKey, sizeof(params.encryptionKey), encryptionKey);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	serverParamsLoader.threadPool = threadPool;
	serverParamsLoader.nEncryptedPackage = &nWriterPackageProccessed;
	serverParamsLoader.blockingQueue = blockingQueue;
	serverParamsLoader.nEncryptedBytes = &userState->nEncryptedBytes;
	result = StringCchCopyA(serverParamsLoader.fileName, strlen(params.fileName) + 3, params.fileName);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	result = StringCchCatA(serverParamsLoader.fileName, strlen(params.fileName) + 3, "W");
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	writerHandle = CreateThread(
		NULL,							// no security attribute 
		0,								// stack size 
		ServerWriterWorker,				// thread proc
		(&serverParamsLoader),			// thread parameter 
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

	readerExitCode = WaitForSingleObject(readerHandle, INFINITE);
	if (readerExitCode != SUCCESS)
	{
		status = THREAD_ERROR;
		response = FAILED_RESPONSE;
		TerminateThread(readerHandle, THREAD_ERROR);
		protocol->SendPackage(protocol, &response, sizeof(response));
		goto Exit;
	}
	while (nReaderPackageProccessed != nWriterPackageProccessed)//wait for writer worker to resend encrypted message
	{
		Sleep(25);
	}
	TerminateThread(writerHandle, SUCCESS);

	response = OK_RESPONSE;
	protocol->SendPackage(protocol, &response, sizeof(response));
	userState->type = OFFLINE;
Exit:
	InterlockedDecrement(refCounter);
	free(readerFileName);
	free(writerFileName);
	FlushFileBuffers(protocol->GetPipeHandle(protocol));
	DisconnectNamedPipe(protocol->GetPipeHandle(protocol));
	free(protocol);
	DestroyBlockingQueue(&blockingQueue);
	printf("InstanceThread exitting.\n");
	ExitThread(status);
}

STATUS ValidUserStatusToResponse(
	_In_ STATUS loginStatus,
	_Out_ RESPONSE_TYPE* response)
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

STATUS WINAPI ServerReaderWorker(
	_In_ LPVOID parameters)
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


	if (NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	logger.Info(&logger, "Server reader was started");
	params = *((PARAMS_LOAD*)parameters);
	blockingQueue = params.blockingQueue;
	threadPool = params.threadPool;

	StringCchCopyA(encryptionKey, sizeof(encryptionKey), params.encryptionKey);

	CreateProtocol(&protocol);
	protocol.InitializeConnexion(&protocol, params.fileName);
	logger.Info(&logger, "Server initialized the conection");
	(*(params.nEncryptedPackage)) = 0;
	while (TRUE)
	{
		status = protocol.ReadPackage(&protocol, &request, sizeof(request), &nReadedBytes);
		if (ENCRYPTED_MESSAGE_REQUEST == request)
		{
			logger.Info(&logger, "The server read ENCRYPTED_MESSAGE_REQUEST");
			status = CreatePackage(&packageForEncrypt);
			if (SUCCESS != status)
			{
				goto Exit;
			}
			status = CreateSpecialPackage(&specialPackgeForThreadPool, encryptionKey);
			if (SUCCESS != status)
			{
				goto Exit;
			}
			specialPackgeForThreadPool->package = packageForEncrypt;
			packageForEncrypt->size = 0;

			status = protocol.ReadPackage(&protocol, packageForEncrypt, sizeof(PACKAGE), &nReadedBytes);
			if (SUCCESS != status)
			{
				logger.Info(&logger, "The server could not read the encryption package");
				goto Exit;
			}
			packageForEncrypt->buffer[packageForEncrypt->size] = '\0';
			status = blockingQueue->Add(blockingQueue, specialPackgeForThreadPool);
			if (SUCCESS != status)
			{
				goto Exit;
			}
			status = threadPool->Add(threadPool, specialPackgeForThreadPool);
			if (SUCCESS != status)
			{
				goto Exit;
			}
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


STATUS WINAPI ServerWriterWorker(
	_In_ LPVOID parameters)
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
	long *nEncryptedBytes;

	nEncryptedBytes = NULL;
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
	nEncryptedBytes = params.nEncryptedBytes;

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
		logger.Info(&logger, "A package was readed by writer");
		if (GET_ENCRYPTED_MESSAGE_REQUEST == request)
		{
			logger.Info(&logger, "The server has received GET_ENCRYPTED_MESSAGE_REQUEST");
			status = blockingQueue->Take(blockingQueue, &specialPackgeForThreadPool);
			timeToSleep = 10;
			logger.Info(&logger, "Will wait fait package encription");
			while (specialPackgeForThreadPool->isEncrypted != TRUE)
			{
				Sleep(timeToSleep);
				timeToSleep += 5;
			}
			logger.Info(&logger, "Taked a package for ennript");
			response = OK_RESPONSE;
			status = protocol.SendPackage(&protocol, &response, sizeof(response));
			if (SUCCESS != status)
			{
				logger.Warning(&logger, "The server can not send the ok response for encryption request");
				goto Exit;
			}

			status = protocol.SendPackage(&protocol, specialPackgeForThreadPool->package, sizeof(PACKAGE));
			InterlockedAdd(nEncryptedBytes, specialPackgeForThreadPool->package->size);
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

STATUS InitializeUserState(
	_Inout_ PUSER_STATE *userState,
	_In_ CHAR* username)
{
	HRESULT result;
	STATUS status;
	size_t length;
	PUSER_STATE _userState;

	_userState = NULL;
	length = 0;
	result = S_OK;
	status = SUCCESS;

	if (NULL == userState || NULL == username)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	result = StringCchLengthA(username, 100, &length);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	_userState = (PUSER_STATE)malloc(sizeof(USER_STATE));
	if (NULL == _userState)
	{
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	result = StringCchCopyA(_userState->username, sizeof(_userState->username) + 1, username);
	if (S_OK != result)
	{
		status = STRING_ERROR;
		goto Exit;
	}
	_userState->nEncryptedBytes = 0;
	_userState->type = ONLINE;
Exit:
	if (SUCCESS != status)
	{
		free(userState);
		userState = NULL;
	}
	*userState = _userState;
	return status;
}

STATUS DestroyUserState(
	_Inout_ PUSER_STATE* userState)
{
	STATUS status;

	status = SUCCESS;

	if (NULL == userState)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	free(*userState);
	*userState = NULL;
Exit:
	return status;
}

BOOL FindElement(
	_In_ LPVOID item1,
	_In_ LPVOID item2)
{
	BOOL res;
	PUSER_STATE userState1;
	PUSER_STATE userState2;

	userState1 = (PUSER_STATE)item1;
	userState2 = (PUSER_STATE)item2;
	res = TRUE;

	if (NULL == item1 || NULL == item2)
	{
		res = FALSE;
		goto Exit;
	}

	if (0 == strcmp(userState1->username, userState2->username))
	{
		res = TRUE;
		goto Exit;
	}
	res = FALSE;
Exit:
	return res;
}