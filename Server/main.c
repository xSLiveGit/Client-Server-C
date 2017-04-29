#define _CRT_SECURE_NO_WARNINGS

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

int main()
{
	SERVER server;

	CreateServer(&server, "nume");
	//server.OpenConnexion(&server);
	server.Run(&server);
	printf_s("gata\n");
	server.RemoveServer(&server);

	getchar();
	_CrtDumpMemoryLeaks();

	return 0;
}