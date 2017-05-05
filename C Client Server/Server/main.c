#define _CRT_SECURE_NO_WARNINGS
//#include <MyBlockingQueue.h>
#include "server.h"
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

char a[][30] = { {"and"} };

int main()
{
	SERVER server;
	STATUS status = SUCCESS;
	//PMY_BLOCKING_QUEUE queue;

	//CreateMyBlockingQueue(&queue);

	status = CreateServer(&server, "nume", "logger.txt");
	if (!SUCCESS == status)
	{
		printf_s("FAILED TO INITIALIZE SERVER!!!\n");
		goto Exit;
	}
	server.Run(&server,1);
	printf_s("gata\n");
Exit:
	server.RemoveServer(&server);

	getchar();
	_CrtDumpMemoryLeaks();

	return 0;
}