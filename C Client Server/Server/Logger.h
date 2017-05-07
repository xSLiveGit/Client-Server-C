#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <Windows.h>
#include "MyBlockingQueue.h"

typedef struct _LOGGER {
	HANDLE outputFileHandle;
	CRITICAL_SECTION criticalSection;
	LPSECURITY_ATTRIBUTES lpSecurityAtributes;

	STATUS(*Info)(
		_In_ struct _LOGGER* logger,
		_In_ CHAR* message);
	
	STATUS(*Warning)(
		_In_ struct _LOGGER* logger,
		_In_ CHAR* message);
} LOGGER, *PLOGGER;

STATUS InitializeLogger(PLOGGER plogger, CHAR* outputFilePath);
STATUS DestroyLogger(PLOGGER plogger);
#endif//!_LOGGER_H_