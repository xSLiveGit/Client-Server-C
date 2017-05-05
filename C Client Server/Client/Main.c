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

STATUS main(int argc, char** argv)
{
	STATUS status;
	CLIENT client;
	status = SUCCESS;

	status |= CreateClient(&client, "nume");
	if(SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.OpenConnexion(&client);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.Run(&client, "input.txt", "output.txt","mykey");
	status |= client.RemoveClient(&client);

	getchar();

Exit:
	_CrtDumpMemoryLeaks();
	return status;
}