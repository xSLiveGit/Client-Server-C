#define CRT_SECURE_NO_WARNINGS
#include "Logger.h"
#include <stdio.h>
#include <strsafe.h>

STATUS CreateLogger(PLOGGER *plogger, CHAR* outputFilePath);
STATUS Info (PLOGGER logger, CHAR* message);
STATUS Warning (PLOGGER logger, CHAR* message);
STATUS DestroyLogger(PLOGGER *plogger);


STATUS CreateLogger(PLOGGER *plogger, CHAR* outputFilePath)
{
	STATUS status;
	PLOGGER logger;
	
	status = SUCCESS;
	logger = NULL;
	if(NULL == plogger)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	
	logger = (PLOGGER)malloc(sizeof(logger));
	if(NULL == logger)
	{
		status = MALLOC_FAILED_ERROR;
		*plogger = NULL;
		goto Exit;
	}

	logger->lpSecurityAtributes = (LPSECURITY_ATTRIBUTES)malloc(sizeof(SECURITY_ATTRIBUTES));
	logger->Info = &Info;
	logger->Warning = &Warning;
	logger->lpSecurityAtributes->bInheritHandle = TRUE;
	logger->lpSecurityAtributes->lpSecurityDescriptor = NULL;
	logger->lpSecurityAtributes->nLength = sizeof(*(logger->lpSecurityAtributes));
	InitializeCriticalSection(&(logger->criticalSection));
	if(NULL == outputFilePath)
	{
		logger->outputFileHandle = stdout;
	}
	else
	{
		logger->outputFileHandle = CreateFileA(
			outputFilePath,					//_In_     LPCTSTR               lpFileName,
			GENERIC_WRITE,					//_In_     DWORD                 dwDesiredAccess,
			0,								//_In_     DWORD                 dwShareMode,
			logger->lpSecurityAtributes,	//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			CREATE_ALWAYS,					//_In_     DWORD                 dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,			//_In_     DWORD                 dwFlagsAndAttributes,
			NULL							//_In_opt_ HANDLE                hTemplateFile
			);
	}
	*plogger = logger;
Exit:
	return status;
}


STATUS WriteGenericLoggerMessage(PLOGGER logger, CHAR* messageTypeIdentifier,CHAR* message)
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
	messageToWrite = (CHAR*)malloc(timeBytesSize * sizeof(CHAR) + tempBytes*sizeof(CHAR) + totalBytesMessage*sizeof(CHAR));
	totalBytesMessage = timeBytesSize + tempBytes + totalBytesMessage;
	if(NULL == messageToWrite)
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
	sprintf_s(tempBuffer,sizeof(tempBuffer), " %d/%d/%d %d:%d:%d:%4d ", systemtime.wDay,systemtime.wMonth,systemtime.wYear,systemtime.wHour,systemtime.wMinute,systemtime.wSecond,systemtime.wMilliseconds);
	StringCchCopyA(messageToWrite, totalBytesMessage, tempBuffer);
	StringCchCatA(messageToWrite, totalBytesMessage, messageTypeIdentifier);//Type: day/month/year hour:minute:second:milisecond
	StringCchCatA(messageToWrite, totalBytesMessage, message);
	StringCchLengthA(messageToWrite, totalBytesMessage, &length);
	StringCchCatA(messageToWrite, totalBytesMessage, "\n");
	EnterCriticalSection(&(logger->criticalSection));//I prefer an error margin for time rather than blocking the critical section for a long time
	res = WriteFile(
		logger->outputFileHandle,		//_In_        HANDLE       hFile,
		messageToWrite,					//_In_        LPCVOID      lpBuffer,
		length+1,						//_In_        DWORD        nNumberOfBytesToWrite,
		NULL,							//_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		NULL							//_Inout_opt_ LPOVERLAPPED lpOverlapped
	);
	LeaveCriticalSection(&(logger->criticalSection));
	if(!res)
	{
		status = FILE_ERROR;
	}

Exit:
	free(messageToWrite);
	return status;
}

STATUS Info(PLOGGER logger, CHAR* message)
{
	return WriteGenericLoggerMessage(logger, "INFO: ", message);
}

STATUS Warning(PLOGGER logger,CHAR* message)
{
	return WriteGenericLoggerMessage(logger, "WARNING: ", message);
}

STATUS DestroyLogger(PLOGGER *plogger)
{
	STATUS status = SUCCESS;
	PLOGGER logger;

	status = SUCCESS;

	if(NULL ==  plogger || NULL == *plogger)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}
	logger = *plogger;

	free(logger->lpSecurityAtributes);
	logger->lpSecurityAtributes = NULL;
	FlushFileBuffers(logger->outputFileHandle);
	CloseHandle(logger->outputFileHandle);
	DeleteCriticalSection(&(logger->criticalSection));
	logger = NULL;
	*plogger = NULL;
Exit:
	return status;
}

