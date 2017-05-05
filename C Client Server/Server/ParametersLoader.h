#ifndef _PARAMETERS_LOADER_H_
#define _PARAMETERS_LOADER_H_
#include "Status.h"
STATUS LoadParameters(char** argv, int argc, CHAR** nWorkers, CHAR** nMaxClients, CHAR** pipeName, CHAR** logger);
#endif//!_PARAMETERS_LOADER_H_
