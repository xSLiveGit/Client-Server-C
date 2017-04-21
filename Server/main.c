#include "server.h"

int main()
{
	SERVER server;

	CreateServer(&server,"nume\0");
	//server.OpenConnexion(&server);
	server.Run(&server);
	server.RemoveServer(&server);

	return 0;
}