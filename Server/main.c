#include "server.h"
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#include <stdio.h>

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

	CreateServer(&server, "nume\0");
	//server.OpenConnexion(&server);
	server.Run(&server);
	server.RemoveServer(&server);
	getchar();
	_CrtDumpMemoryLeaks();

	return 0;
}