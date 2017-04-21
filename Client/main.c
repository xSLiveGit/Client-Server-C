#include "client.h"
int main()
{
	CLIENT client;
	
	CreateClient(&client, "nume");
	client.Run(&client);
	client.RemoveClient(&client);

	return 0;
}