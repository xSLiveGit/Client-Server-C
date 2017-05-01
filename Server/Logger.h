#ifndef _LOGGER_H_
#define _LOGGER_H_
#include <Windows.h>
#include "../Client/Protocol.h"

typedef struct _LOGGER{
	HANDLE outputFileHandle;
	STATUS(*Info)(struct _LOGGER* logger, CHAR* message);
	STATUS(*Warning)(struct _LOGGER* logger, CHAR* message);
	CRITICAL_SECTION criticalSection;
	LPSECURITY_ATTRIBUTES lpSecurityAtributes;
} LOGGER, *PLOGGER;

STATUS CreateLogger(PLOGGER *plogger,CHAR* outputFilePath);
STATUS DestroyLogger(PLOGGER *plogger);
#endif//!_LOGGER_H_