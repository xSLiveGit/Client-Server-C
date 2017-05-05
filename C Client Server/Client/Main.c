#define _CRT_SECURE_NO_WARNINGS

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "client.h"
#include <stdio.h>
#include <strsafe.h>
#include "ParametersLoader.h"
#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

void ThreatError(STATUS status)
{
	if (MALLOC_FAILED_ERROR == status)
	{
		printf_s("Malloc error\n");
		return;
	}
	if(STRING_ERROR == status)
	{
		printf_s("String error\n");
		return;
	}
	if(WRONG_ARGUMENTS_STRUCTURE == status)
	{
		printf_s("Wrong arguments. The arguments are:\n"
				"\tMandatory arguments:\n"
				"\t\t-i=\"val\" where val is input file path\n"
				"\t\t-u=\"val\" where val us username\n"
				"\t\t-p=\"val\" where val is password\n"
				"\t\t-k=\"val\" where val is encryption key\n"
				"\tOptional argument :\n"
				"\t\t-o=\"val\" where val is output file path\n"
				"\t\t-n=\"val\" where val is pipe name\n"
			"Obs: arguments must be by next form: -k=val\n"
			);
	}
}

STATUS main(int argc, char** argv)
{
	STATUS status;
	CLIENT client;
	CHAR* inputFilePath;
	CHAR* outputFilePath;
	CHAR* pipeName;
	CHAR* username;
	CHAR* password;
	CHAR* encryptionKey;

	status = SUCCESS;
	inputFilePath = NULL;
	outputFilePath = NULL;
	pipeName = NULL;
	username = NULL;
	password = NULL;
	encryptionKey = NULL;

	status = LoadParameters(argv, argc, &inputFilePath, &outputFilePath, &encryptionKey, &username, &password, &pipeName);
	if(SUCCESS != status)
	{
		ThreatError(status);
		goto Exit;
	}
	printf_s("Username: %s\nPassword:%s\nInput: %s\nOutput: %s\nKey: %s\nPipe: %s\n", username, password, inputFilePath, outputFilePath, encryptionKey, pipeName);
	
	status |= CreateClient(&client, pipeName);
	if(SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.OpenConnexion(&client);
	if (SUCCESS != status)
	{
		goto Exit;
	}
	status |= client.Run(&client, inputFilePath, outputFilePath,encryptionKey,username,password);
	status |= client.RemoveClient(&client);

	getchar();

Exit:
	free(inputFilePath);
	free(outputFilePath);
	free(encryptionKey);
	free(username);
	free(password);
	free(pipeName);
	_CrtDumpMemoryLeaks();
	return status;
}