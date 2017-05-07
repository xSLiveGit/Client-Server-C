#ifndef _CONSOLE_COMMUNICATION_H_
#define _CONSOLE_COMMUNICATION_H_
#include "server.h"

typedef struct
{
	PSERVER pserver;
} CONSOLE_PARAMS;

STATUS WINAPI ConsoleCommunicationThread(
	_In_ LPVOID parameters);

#endif
