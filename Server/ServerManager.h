#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#include <WinSock2.h>
#include <Windows.h>
#include "SocketInfo.h"

class ServerManager {
private:
	WSAData wsaData;
	SYSTEM_INFO sysInfo;

public:
	ServerManager();
	~ServerManager();
	void InitServer(int prime, int sub);
	void CreateThread(HANDLE hComPort, unsigned int (WINAPI* ThreadMain)(void*));
};

#endif
