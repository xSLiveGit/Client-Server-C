#ifndef _PARAMETERS_LOADER_H_
#define _PARAMETERS_LOADER_H_
#include "Status.h"

STATUS LoadParameters(
	_In_ char** argv,
	_In_ int argc,
	_In_ CHAR** inputFilePath,
	_In_ CHAR** outputFilePath,
	_In_ CHAR** encryptionKey,
	_In_ CHAR** username,
	_In_ CHAR** password,
	_In_ CHAR** pipeName);

#endif//!_PARAMETERS_LOADER_H_
