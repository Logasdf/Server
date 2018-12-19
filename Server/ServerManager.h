#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#include <WinSock2.h>
#include <Windows.h>
#include "SocketInfo.h"

class ServerManager {
public:
	ServerManager();
	~ServerManager();

	void Start(int port);
	void Stop();

private:
	void InitSocket(int port, int prime = 2, int sub = 2);
	void InitCompletionPort(int maxNumberOfThreads = 0);
	void AcceptClient();
	void CloseClient();
	void CreateThreadPool(int numOfThreads = 0);
	void ShutdownThreads();

private:
	WSAData wsaData;
	HANDLE hCompPort;
	SOCKET servSock;
};

unsigned __stdcall ThreadMain(void* pVoid);

#endif
