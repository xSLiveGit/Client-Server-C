#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "client.h"
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
	CLIENT client;

	CreateClient(&client, "nume");
	client.OpenConnexion(&client);
	client.Run(&client, "input.txt", "output.txt");

	client.RemoveClient(&client);
	getchar();

	_CrtDumpMemoryLeaks();
	return 0;
}