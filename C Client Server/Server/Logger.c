#define CRT_SECURE_NO_WARNINGS
#include "Logger.h"
#include <stdio.h>
#include <strsafe.h>
#include "status.h"



STATUS InitializeLogger(
	_Inout_ PLOGGER plogger, 
	_In_ CHAR* outputFilePath);

STATUS Info(
	_In_ PLOGGER logger, 
	_In_ CHAR* message);

STATUS Warning(
	_In_ PLOGGER logger,
	_In_ CHAR* message);

STATUS DestroyLogger(
	_Inout_ PLOGGER plogger);


STATUS InitializeLogger(
	_Inout_ PLOGGER plogger,
	_In_ CHAR* outputFilePath)
{
	STATUS status;

	status = SUCCESS;
	if (NULL == plogger)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	plogger->lpSecurityAtributes = (LPSECURITY_ATTRIBUTES)malloc(sizeof(SECURITY_ATTRIBUTES));
	plogger->Info = &Info;
	plogger->Warning = &Warning;
	plogger->lpSecurityAtributes->bInheritHandle = TRUE;
	plogger->lpSecurityAtributes->lpSecurityDescriptor = NULL;
	plogger->lpSecurityAtributes->nLength = sizeof(*(plogger->lpSecurityAtributes));
	InitializeCriticalSection(&(plogger->criticalSection));
	if (NULL == outputFilePath)
	{
		plogger->outputFileHandle = stdout;
	}
	else
	{
		plogger->outputFileHandle = CreateFileA(
			outputFilePath,					//_In_     LPCTSTR               lpFileName,
			GENERIC_WRITE,					//_In_     DWORD                 dwDesiredAccess,
			0,								//_In_     DWORD                 dwShareMode,
			plogger->lpSecurityAtributes,	//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			CREATE_ALWAYS,					//_In_     DWORD                 dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,			//_In_     DWORD                 dwFlagsAndAttributes,
			NULL							//_In_opt_ HANDLE                hTemplateFile
			);
	}
Exit:
	return status;
}


STATUS WriteGenericLoggerMessage(PLOGGER logger, CHAR* messageTypeIdentifier, CHAR* message)
{
	STATUS status;
	SYSTEMTIME systemtime;
	CHAR* mallocErrorMsg = "FATAL ERROR: Malloc error!!!!\n";
	CHAR* messageToWrite;
	DWORD timeBytesSize = 50;
	CHAR tempBuffer[50]; // for conversion
	size_t length;
	unsigned tempBytes;
	unsigned totalBytesMessage;
	BOOL res;

	res = TRUE;
	length = 0;
	tempBytes = 0;
	status = SUCCESS;
	messageToWrite = NULL;

	if (NULL == logger || NULL == message)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	//StringCchLengthA(message, (1<<31), &totalBytesMessage);
	//StringCchLengthA(messageTypeIdentifier, (1 << 31), &tempBytes);
	totalBytesMessage = strlen(message);
	tempBytes = strlen(messageTypeIdentifier);
	messageToWrite = (CHAR*)GlobalAlloc(0, 100 + timeBytesSize * sizeof(CHAR) + tempBytes*sizeof(CHAR) + totalBytesMessage*sizeof(CHAR));
	totalBytesMessage = timeBytesSize + tempBytes + totalBytesMessage;
	if (NULL == messageToWrite)
	{
		EnterCriticalSection(&(logger->criticalSection));
		WriteFile(
			logger->outputFileHandle,		//_In_        HANDLE       hFile,
			mallocErrorMsg,					//_In_        LPCVOID      lpBuffer,
			sizeof(mallocErrorMsg),			//_In_        DWORD        nNumberOfBytesToWrite,
			NULL,							//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
			NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
			);
		LeaveCriticalSection(&(logger->criticalSection));
		status = MALLOC_FAILED_ERROR;
		goto Exit;
	}

	GetSystemTime(&systemtime);
	sprintf_s(tempBuffer, sizeof(tempBuffer), " %d/%d/%d %d:%d:%d:%4d ", systemtime.wDay, systemtime.wMonth, systemtime.wYear, systemtime.wHour, systemtime.wMinute, systemtime.wSecond, systemtime.wMilliseconds);
	res = StringCchCopyA(messageToWrite, totalBytesMessage, tempBuffer);
	if (!res)
	{
		status = FACILITY_AUDCLNT;//O EROARE BUNA PANA VAD DE CE CRAPA
	}
	res = StringCchCatA(messageToWrite, totalBytesMessage, messageTypeIdentifier);//Type: day/month/year hour:minute:second:milisecond
	if (!res)
	{
		status = FACILITY_AUDCLNT;//O EROARE BUNA PANA VAD DE CE CRAPA
	}
	res = StringCchCatA(messageToWrite, totalBytesMessage, message);
	if (!res)
	{
		status = FACILITY_AUDCLNT;//O EROARE BUNA PANA VAD DE CE CRAPA
	}
	res = StringCchLengthA(messageToWrite, totalBytesMessage, &length);
	if (!res)
	{
		status = FACILITY_AUDCLNT;//O EROARE BUNA PANA VAD DE CE CRAPA
	}
	res = StringCchCatA(messageToWrite, totalBytesMessage, "\n");
	if (!res)
	{
		status = FACILITY_AUDCLNT;//O EROARE BUNA PANA VAD DE CE CRAPA
	}
	EnterCriticalSection(&(logger->criticalSection));//I prefer an error margin for time rather than blocking the critical section for a long time
	res = WriteFile(
		logger->outputFileHandle,		//_In_        HANDLE       hFile,
		messageToWrite,					//_In_        LPCVOID      lpBuffer,
		length + 1,						//_In_        DWORD        nNumberOfBytesToWrite,
		NULL,							//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
		);
	LeaveCriticalSection(&(logger->criticalSection));
	if (!res)
	{
		status = FILE_ERROR;
	}

Exit:
	GlobalFree(messageToWrite);
	return status;
}

STATUS Info(
	_In_ PLOGGER logger,
	_In_ CHAR* message)
{
	return WriteGenericLoggerMessage(logger, "INFO: ", message);
}

STATUS Warning(
	_In_ PLOGGER logger, 
	_In_ CHAR* message)
{
	return WriteGenericLoggerMessage(logger, "WARNING: ", message);
}

STATUS DestroyLogger(
	_Inout_ PLOGGER plogger)
{
	STATUS status = SUCCESS;

	status = SUCCESS;

	if (NULL == plogger)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	free(plogger->lpSecurityAtributes);
	plogger->lpSecurityAtributes = NULL;
	FlushFileBuffers(plogger->outputFileHandle);
	CloseHandle(plogger->outputFileHandle);
	DeleteCriticalSection(&(plogger->criticalSection));
Exit:
	return status;
}
