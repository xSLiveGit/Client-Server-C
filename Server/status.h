#ifndef _STATUS_H_
#define _STATUS_H_

#define STATUS DWORD
#define SUCCESS 0
#define NULL_POINTER_ERROR 1
#define CONNEXION_ERROR (1<<1)
#define FILE_ERROR (1<<2)
#define COMUNICATION_ERROR (1<<3)

typedef struct
{
	int size;
	char buffer[4096];
} PACKAGE,*PPACKAGE;
#endif //_STATUS_H_