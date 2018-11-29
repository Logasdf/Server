#include "ErrorHandle.h"

void ErrorHandling(const char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void ErrorHandlingNotExit(const char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
}