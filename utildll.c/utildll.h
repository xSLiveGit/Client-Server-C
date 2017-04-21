#pragma once

#if defined(_UTILDLL_EXPORT)
#define UTILDLL_API __declspec(dllexport)
#else
#define UTILDLL_API __declspec(dllimport)
#endif

#define STATUS DWORD
#define SUCCESS						0
#define PIPE_CONNEXION_ERROR		1
#define INVALID_POINTER_ERROR		(1<<1)