#ifndef __ERROR_HANDLE_H__
#define __ERROR_HANDLE_H__

void ErrorHandling(int errCode, bool isExit = true);
void ErrorHandling(const char* msg, bool isExit = true);
void ErrorHandling(const char* msg, int errCode, bool isExit = true);

#endif
