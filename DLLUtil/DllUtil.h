#pragma once
#include "status.h"
#include <Windows.h>
#include <stdio.h>
#if defined(_DLL_UTIL_EXPORT)
#define DLL_UTIL_API __declspec(dllexport)
#else
#define DLL_UTIL_API __declspec(dllimport)
#endif

DLL_UTIL_API STATUS MemoryCopy(const void* source, DWORD size, void* destination);
DLL_UTIL_API STATUS StringToInt(char * s1, int *numar);
DLL_UTIL_API STATUS IntToString(int number, char * result);
DLL_UTIL_API BOOL ExportFirstNCharacters(const char* string, const FILE* outputStream, unsigned int exportedNumerCharacters);