#define _CRT_SECURE_NO_WARNINGS
//#include <MyBlockingQueue.h>
#include "server.h"
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>
#include "ParametersLoader.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

void ThreatError(STATUS status)
{
	if (MALLOC_FAILED_ERROR == status)
	{
		printf_s("Malloc error\n");
		return;
	}
	if (STRING_ERROR == status)
	{
		printf_s("String error\n");
		return;
	}
	if (WRONG_ARGUMENTS_STRUCTURE == status)
	{
		printf_s("Wrong arguments. The arguments are:\n"
			"\tMandatory arguments:\n"
			"\t\t-w=\"val\" where val is the number of workers\n"
			"\t\t-c=\"val\" where val is the max clients served at same time\n"
			"\tOptional argument :\n"
			"\t\t-l=\"val\" where val is logger path\n"
			"\t\t-n=\"val\" where val is pipe name\n"
			"Obs: arguments must be by next form: -k=val\n"
			);
	}
}

char a[][30] = { {"and"} };

int main(int argc,char** argv)
{
	SERVER server;
	STATUS status = SUCCESS;
	//PMY_BLOCKING_QUEUE queue;

	//CreateMyBlockingQueue(&queue);
	CHAR* nWorkers;
	CHAR* nMaxClient;
	CHAR* logger;
	CHAR* pipeName;
	LONG nMaxClientsNumber;
	LONG nWorkersNumber;
	status = SUCCESS;
	nWorkers = NULL;
	nMaxClient = NULL;
	pipeName = NULL;
	logger = NULL;

	status = LoadParameters(argv, argc, &nWorkers,&nMaxClient,&pipeName,&logger);
	if (SUCCESS != status)
	{
		ThreatError(status);
		goto Exit;
	}
	printf_s("nWorkers: %s\nMaxClient:%s\npipeName: %s\nlogger: %s\n", nWorkers, nMaxClient, pipeName, logger);

	status = CreateServer(&server, pipeName, logger);
	if (!SUCCESS == status)
	{
		printf_s("FAILED TO INITIALIZE SERVER!!!\n");
		goto Exit;
	}
	nMaxClientsNumber = atoi(nMaxClient);
	nWorkersNumber = atoi(nWorkers);
	server.Run(&server, nMaxClientsNumber, nWorkersNumber);
	printf_s("gata\n");
Exit:
	server.RemoveServer(&server);
	free(nWorkers);
	free(nMaxClient);
	free(pipeName);
	free(logger);
	getchar();
	_CrtDumpMemoryLeaks();

	return 0;
}