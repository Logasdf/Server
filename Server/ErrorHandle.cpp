#include "ErrorHandle.h"
#include <Windows.h>
#include <cstdio>
#include <cstdlib>

void ErrorHandling(int errCode, bool isExit = true) {
	WCHAR errMsg[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, 0, errMsg, 1024, NULL);
	fprintf(stderr, "[Error Code]: %d, [Error Message]: %s\n", errCode, errMsg);
	if (isExit) exit(1);
}

void ErrorHandling(const char* msg, bool isExit = true) {
	fprintf(stderr, "[Error Message]: %s\n", msg);
	if (isExit) exit(1);
}