#include "ConsoleCommunication.h"
#include <stdio.h>

STATUS WINAPI ConsoleCommunicationThread(LPVOID parameters)
{
	STATUS status;
	CONSOLE_PARAMS params;
	PSERVER pserver;
	CHAR infoString[] = { "You can chose 1 of the next options:\n\t\t1 - Show server info\n\t\t2 - Exit\n" };
	pserver = NULL;
	status = SUCCESS;
	BOOL res;
	char c;
	char next;
	INT counter;
	INT index; 
	PUSER_STATE userState;

	counter = 0;
	next = 0;
	res = TRUE;
	if (NULL == parameters)
	{
		status = NULL_POINTER_ERROR;
		goto Exit;
	}

	params = *((CONSOLE_PARAMS*)parameters);
	pserver = params.pserver;
	while (TRUE)
	{
		printf_s("%s", infoString);
		scanf_s("%c", &c);
		if (next != '\n')
		{
			do
			{
				counter++;
				scanf_s("%c", &next);
			} while (next != '\n');
			if (counter > 1)
			{
				printf("Invalid option. %s", infoString);
			}
			next = 0;
			counter = 0;
		}
		if (c == '1')
		{
			printf_s("Thre are %d users.\n", pserver->pdynamicVector->size);
			for (index = 0; index < pserver->pdynamicVector->size;index++)
			{
				status = VectorGet(*(pserver->pdynamicVector), index, &userState);
				if(SUCCESS != status)
				{
					printf_s("An error has been occured.\n");
					continue;
				}
				printf_s("\tUsername: %s\n", userState->username);
				if(userState->type == ONLINE)
				{
					printf_s("\tStatus:%s\n", "ONLINE\n");
				}
				else
				{
					printf_s("\tStatus:%s\n", "OFFLINE\n");

				}
				printf_s("\tnEncryptedBytes: %ld\n", userState->nEncryptedBytes);
			}
			continue;
		}
		else if (c == '2')
		{
			pserver->SetStopFlag(pserver);
			printf_s("Dupa stop flag\n");
			Sleep(100);//sleep to avoid cancel de the pipe handle while a client request a connection;
			printf_s("A dormit.Urmeaza CancelIoEx\n");
			res = CancelIoEx(pserver->serverProtocol->pipeHandle, NULL);
			printf_s("A facut canceIoEx");
			if (!res)
			{
				printf("Operation failed. You should try again.\n");
			}
			//res = CloseHandle(pserver->serverProtocol->pipeHandle);
			goto Exit;
		}
		else
		{
			printf("Wrong option\n");
			continue;
		}
	}

Exit:
	return status;
}
