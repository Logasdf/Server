#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <Windows.h>
#include <unordered_map>
#include <utility>
#include "def.h"
#include "SocketInfo.h"
#include "protobuf/room.pb.h"
#include "Room.h"

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
	bool HandleWithoutBody(SocketInfo* lpSocketInfo, int& type);
	bool HandleWithBody(SocketInfo* lpSocketInfo, MessageLite* message, int& type);

	//Temperary Method
	void InitRoom(RoomInfo* pRoom, SocketInfo* lpSocketInfo, string& roomName, int& limits, string& userName);
	void SendInitData(SocketInfo*);
	void BroadcastMessage(Room* room, MessageLite* message);

private:
	WSAData wsaData;
	HANDLE hCompPort;
	SOCKET servSock;

	int threadPoolSize;
	HANDLE hMutexObj;

	int roomIdStatus;
	RoomList roomList;
	std::unordered_map<int, Room*> serverRoomList;
	std::unordered_map<string, int> roomTable;
};
