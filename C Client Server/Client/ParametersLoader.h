#ifndef _PARAMETERS_LOADER_H_
#define _PARAMETERS_LOADER_H_
#include "Status.h"

STATUS LoadParameters(char** argv, int argc, CHAR** inputFilePath, CHAR** outputFilePath, CHAR** encryptionKey, CHAR **username, CHAR** password, CHAR** pipeName);
#endif//!_PARAMETERS_LOADER_H_
