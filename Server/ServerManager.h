#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include "def.h"
#include "SocketInfo.h"
#include "protobuf/room.pb.h"

class ServerManager {
public:
	ServerManager();
	~ServerManager();

	void Start(int port=PORT);
	void Stop();

private:
	static unsigned __stdcall ThreadMain(void* pVoid);

private:
	void InitSocket(int port, int prime = 2, int sub = 2);
	void InitCompletionPort(int maxNumberOfThreads = 0);
	void AcceptClient();
	void CloseClient(SocketInfo* lpSocketInfo, bool graceful = false);
	void CreateThreadPool(int numOfThreads = 0);
	void ShutdownThreads();
	bool SendPacket(SocketInfo* lpSocketInfo);
	bool RecvPacket(SocketInfo* lpSocketInfo);
	bool HandleSendEvent(SocketInfo* lpSocketInfo, DWORD dwBytesTransferred);
	bool HandleRecvEvent(SocketInfo* lpSocketInfo, DWORD dwBytesTransferred);

	//Temperary Method
	void InitRoomList();

private:
	WSAData wsaData;
	HANDLE hCompPort;
	SOCKET servSock;

	int threadPoolSize;
	HANDLE hMutexObj;

	RoomList roomList;
};
