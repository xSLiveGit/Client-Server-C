#pragma once
#include <Windows.h>
#include <string.h>
#if defined(_MYDLL_EXPORT)
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif


