#define _CRT_SECURE_NO_WARNINGS

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "client.h"
#include <stdio.h>
#include <strsafe.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

int main(int argc, char** argv)
{
	CLIENT client;

	CreateClient(&client, "nume");
	client.OpenConnexion(&client);
	client.Run(&client, argv[1], argv[2],"mykey");

	client.RemoveClient(&client);

	getchar();


	_CrtDumpMemoryLeaks();
	return 0;
}