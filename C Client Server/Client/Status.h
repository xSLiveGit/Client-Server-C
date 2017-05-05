#ifndef _STATUS_H_
#define _STATUS_H_
#include "Windows.h"
typedef  enum { LOGIN_REQUEST, ENCRYPTED_MESSAGE_REQUEST, LOGOUT_REQUEST, FINISH_CONNECTION_REQUEST, INITIALIZE_REQUEST, END_ENCRYPTED_MESSAGE_REQUEST, GET_ENCRYPTED_MESSAGE_REQUEST } REQUEST_TYPE;
typedef  enum { SUCCESS_LOGIN_RESPONSE, WRONG_CREDENTIALS_RESPONSE, ACCEPTED_CONNECTION_RESPONSE, REJECTED_CONNECTION_RESPONSE, WRONG_PROTOCOL_BEHAVIOR_RESPONSE, OK_RESPONSE, FAILED_RESPONSE } RESPONSE_TYPE;

#define DEFAULT_NAMED_PIPE "NamedPipe"

typedef DWORD STATUS;
#define MAX_BUFFER_SIZE 4096
#define MAX_OPT_BUFFER_SIZE 20
#define MAX_MESSAGE_BYTES (sizeof(PACKAGE))
#define SUCCESS 0
#define NULL_POINTER_ERROR 1
#define CONNECTION_ERROR (1<<1)
#define FILE_ERROR (1<<2)
#define COMUNICATION_ERROR (1<<3)
#define MALLOC_FAILED_ERROR (1<<4)
#define TIME_OUT (1 << 7)
#define INSUFFICEINT_MANDATORY_ARGUMENTS (1 << 8)
#define WRONG_ARGUMENTS_STRUCTURE (1 << 9)

#define VALID_USER 0
#define WRONG_CREDENTIALS (1<<5)

//Particual status
#define SUCCESS_LOGIN 0
#define FAILED_LOGIN_WRONG_CREDENTIALS (1 | WRONG_CREDENTIALS)
#define FAILED_LOGIN_SERVER_REFUSED_CONNECTION (1 | (1<<6))

#define STRING_ERROR (1<<17)
#define THREAD_ERROR (1<<19)
#define INDEX_OUT_OF_BOUNDS (1<<20)
#define ELEMENT_NOT_FOUND (1<<21)

#define  WRONG_BEHAVIOR (1<<14)

#define ON_SUCCESS_LOGIN(a) ((a) == SUCCESS_LOGIN)
#define ON_REFUSED_CONNECTION(a) (((a) & (FAILED_LOGIN_SERVER_REFUSED_CONNECTION)) == FAILED_LOGIN_SERVER_REFUSED_CONNECTION)
#define ON_WRONG_CREDENTIALS(a) ((a) & FAILED_LOGIN_WRONG_CREDENTIALS == FAILED_LOGIN_WRONG_CREDENTIALS)

#define PERMISED_LOGIN_MESSAGE "OK"
#define REFUSED_BY_WRONG_CREDENTIALS_MESSAGE "N1"
#define REFUSED_BY_SERVER_REFUSED_CONNECTION_MESSAGE "N2"


typedef struct
{
	DWORD size;
	char buffer[MAX_BUFFER_SIZE];
	char optBuffer[MAX_OPT_BUFFER_SIZE];
} PACKAGE, *PPACKAGE;
#endif //_STATUS_H_