#pragma once
#include <Windows.h>
#include <string.h>
#include <tchar.h> 
#if defined(_MYDLL_EXPORT)
#define MYDLL_API __declspec(dllexport)
#else
#define MYDLL_API __declspec(dllimport)
#endif

 MYDLL_API int Sum(int a, int b);
