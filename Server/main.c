#include "server.h"

int main()
{
	SERVER server;

	CreateServer(&server,"nume");
	server.OpenConnexion(&server);
	server.Run(&server);
	server.RemoveServer(&server);

	return 0;
}