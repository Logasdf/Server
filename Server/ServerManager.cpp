#include <cstdio>
#include <cstdlib>
#include <process.h>
#include "ServerManager.h"
#include "ErrorHandle.h"

ServerManager::ServerManager() {}

ServerManager::~ServerManager() {}

void ServerManager::InitServer(int prime, int sub)
{
	if (WSAStartup(MAKEWORD(prime, sub), &wsaData) != 0) {
		ErrorHandling("WSAStartup() Error!");
	}
}

void ServerManager::CreateThread(HANDLE hComPort, unsigned int (WINAPI* ThreadMain)(void*))
{
	GetSystemInfo(&sysInfo);
	for (int i = 0; i < sysInfo.dwNumberOfProcessors; ++i) {
		_beginthreadex(NULL, 0, ThreadMain, (void*)hComPort, 0, NULL);
	}
}