#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <process.h>
#include <string>
#include "ServerManager.h"
#include "ErrorHandle.h"
#include "def.h"
#include "Packet.h"

void ServerManager::InitRoomList()
{
	Room* newRoom = new Room();
	newRoom->set_roomid(roomIdStatus);
	newRoom->set_host(0);
	newRoom->set_current(3);
	newRoom->set_limit(8);
	newRoom->set_name("Room #1");
	newRoom->set_readycount(0);

	Client* client = newRoom->add_redteam();
	client->set_ip("192.168.0.123");
	client->set_port(35467);
	client->set_position(0);
	client->set_ready(false);

	client = newRoom->add_redteam();
	client->set_ip("192.168.0.127");
	client->set_port(35489);
	client->set_position(1);
	client->set_ready(false);

	client = newRoom->add_blueteam();
	client->set_ip("192.168.0.190");
	client->set_port(30089);
	client->set_position(8);
	client->set_ready(false);

	(*roomList.mutable_rooms())[roomIdStatus] = *newRoom;
	roomTable.insert(std::make_pair("Room #1", roomIdStatus++));
}

void ServerManager::InitRoom(Room* pRoom, SocketInfo* lpSocketInfo, string& roomName, int& limits)
{
	pRoom->set_host(0);
	pRoom->set_current(1);
	pRoom->set_limit(limits);
	pRoom->set_name(roomName);
	pRoom->set_readycount(0);
	pRoom->set_roomid(roomIdStatus++);

	char ip[20]; 
	int port;
	lpSocketInfo->GetIpAndPort(ip, port);
	Client* client = pRoom->add_redteam();
	client->set_ip(string(ip));
	client->set_port(port);
	client->set_position(0);
	client->set_ready(false);
}

ServerManager::ServerManager() 
{ 
	roomIdStatus = 100;
	hCompPort = NULL; 
	servSock = INVALID_SOCKET;
	hMutexObj = CreateMutex(NULL, FALSE, NULL);

	InitRoomList();
}

ServerManager::~ServerManager() 
{ 
	if (hCompPort != NULL)
		CloseHandle(hCompPort);
	if (servSock != INVALID_SOCKET)
		closesocket(servSock);
	if (hMutexObj != NULL)
		CloseHandle(hMutexObj);

	WSACleanup();
}

void ServerManager::Start(int port)
{
	InitSocket(port);
	InitCompletionPort();
	CreateThreadPool();
	AcceptClient();
}

void ServerManager::Stop() 
{
	ShutdownThreads();
	CloseHandle(hCompPort);
	closesocket(servSock);
	CloseHandle(hMutexObj);

	hCompPort = NULL;
	servSock = INVALID_SOCKET;
	hMutexObj = NULL;
}

unsigned __stdcall ServerManager::ThreadMain(void * pVoid)
{
	ServerManager* self = (ServerManager*)pVoid;
	SocketInfo* lpSocketInfo;
	IOInfo* lpIOInfo;
	DWORD dwBytesTransferred = 0;

	while (true)
	{
		lpSocketInfo = NULL;
		lpIOInfo = NULL;

		//fprintf(stderr, "[Current Thread #%d] => Enter\n", (HANDLE)GetCurrentThreadId());
		bool rtn = GetQueuedCompletionStatus(self->hCompPort, &dwBytesTransferred,
			reinterpret_cast<ULONG_PTR*>(&lpSocketInfo), reinterpret_cast<LPOVERLAPPED*>(&lpIOInfo), INFINITE);
		//fprintf(stderr, "[Current Thread #%d] => Exit\n", (HANDLE)GetCurrentThreadId());
		if (rtn)
		{
			if (((DWORD)lpSocketInfo) == KILL_THREAD) break;

			if (lpIOInfo == NULL) {
				ErrorHandling("Getting IO Information Failed...", WSAGetLastError(), false);
				continue;
			}
		}
		else
		{
			if (lpIOInfo == NULL) {
				ErrorHandling("Getting IO Information Failed...", WSAGetLastError(), false);
			}
			else
			{
				ErrorHandling("Client Connection Close, Socket will close", false);
				self->CloseClient(lpSocketInfo);
			}
			continue;
		}

		try
		{
			fprintf(stderr, "[Current Thread #%d] => ", GetCurrentThreadId());
			if (dwBytesTransferred == 0)
			{
				fprintf(stderr, "[Log]: Client %d Connection Closed....\n", (HANDLE)lpSocketInfo->socket);
				throw "[Cause]: dwBytesTransferr == 0";
			}

			if (lpIOInfo == lpSocketInfo->recvBuf)
			{
				LOG("Complete Receiving Message!!");
				if (!(self->HandleRecvEvent(lpSocketInfo, dwBytesTransferred)))
				{
					throw "[Cause]: RecvEvent Handling Error!!";
				}
			}
			else if (lpIOInfo == lpSocketInfo->sendBuf)
			{
				LOG("Complete Sending Message!!");
				if (!(self->HandleSendEvent(lpSocketInfo, dwBytesTransferred)))
				{
					throw "[Cause]: SendEvent Handling Error!!";
				}
			}
			else 
			{
				throw "[Cause]: UnknownEvent Exception...";
			}
		}
		catch (const char* msg)
		{
			ErrorHandling(msg, WSAGetLastError(), false);
			self->CloseClient(lpSocketInfo);
		}
	}

	return 0;
}

void ServerManager::InitSocket(int port, int prime, int sub)
{
	if (WSAStartup(MAKEWORD(prime, sub), &wsaData) != 0) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	servSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (servSock == INVALID_SOCKET) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(port);

	if (bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling(WSAGetLastError());
		closesocket(servSock);
		servSock = NULL;
		return;
	}

	if (listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
		ErrorHandling(WSAGetLastError());
		closesocket(servSock);
		servSock = NULL;
		return;
	}

	LOG("Server Socket Initiation Success!!");
}

void ServerManager::InitCompletionPort(int maxNumberOfThreads)
{
	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxNumberOfThreads);
	if (hCompPort == NULL) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	LOG("IOCP Kernel Object Created!!");
}

void ServerManager::AcceptClient()
{
	SocketInfo* lpSocketInfo;
	SOCKET clntSock;
	SOCKADDR_IN clntAdr;
	int clntAdrSz = sizeof(SOCKADDR_IN);

	LOG("Accept Process Start");

	while (true)
	{
		__try
		{
			clntSock = accept(servSock, (SOCKADDR*)&clntAdr, &clntAdrSz);
			if (clntSock == INVALID_SOCKET)
			{
				int errCode = WSAGetLastError();
				if (errCode == WSAEINTR)
				{
					return;
				}
				ErrorHandling(errCode, false);
				__leave;
			}

			printf("Client %d (%s::%d) connected\n", clntSock, inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

			int zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}
			zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}

			// socket context 생성(Completion Key로 넘김)
			lpSocketInfo = SocketInfo::AllocateSocketInfo(clntSock);
			if (lpSocketInfo == NULL)
			{
				ErrorHandling("Socket Info Object Allocation Failed...", false);
				continue;
			}

			// IOCP와 clnt socket 연결
			if (CreateIoCompletionPort((HANDLE)clntSock, hCompPort, (ULONG_PTR)lpSocketInfo, 0)
				== NULL) {
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}

			// Connect시 Client에서 RoomList을 전달
			DWORD dwFlags, dwSendBytes;
			dwFlags = dwSendBytes = 0;
			ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(OVERLAPPED));

			// Serialize Room List;
			lpSocketInfo->sendBuf->wsaBuf.len =
				lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomList);
			if (WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
				&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL) == SOCKET_ERROR)
			{
				int errCode = WSAGetLastError();
				if (errCode != WSA_IO_PENDING)
				{
					ErrorHandling("Init Send Operation(Send Room List) Error!!", errCode, false);
					continue;
				}
			}
		}
		__finally
		{
			if (AbnormalTermination())
			{
				// CloseClient
				CloseClient(lpSocketInfo);
			}
		}
	}
}

void ServerManager::CloseClient(SocketInfo* lpSocketInfo, bool graceful)
{
	WaitForSingleObject(hMutexObj, INFINITE);
	if (lpSocketInfo != NULL && lpSocketInfo->socket != INVALID_SOCKET)
	{
		if (!graceful)
		{
			LINGER LingerStruct;
			LingerStruct.l_onoff = 1;
			LingerStruct.l_linger = 0;
			assert(SOCKET_ERROR 
				!= setsockopt(lpSocketInfo->socket, SOL_SOCKET, SO_LINGER, 
				(char*)&LingerStruct, sizeof(LingerStruct)));
		}
		closesocket(lpSocketInfo->socket);
		SocketInfo::DeallocateSocketInfo(lpSocketInfo);
		lpSocketInfo = NULL;
	}
	ReleaseMutex(hMutexObj);
}

void ServerManager::CreateThreadPool(int numOfThreads)
{
	if (numOfThreads == 0)
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		threadPoolSize = sysinfo.dwNumberOfProcessors * 2 + 2;
	}
	else
	{
		threadPoolSize = numOfThreads;
	}

	for (int i = 0; i < threadPoolSize; ++i)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = BEGINTHREADEX(NULL, 0, ServerManager::ThreadMain, this, 0, &dwThreadId);
		CloseHandle(hThread);
	}
}

void ServerManager::ShutdownThreads()
{
	for (int i = 0; i < threadPoolSize; ++i)
	{
		PostQueuedCompletionStatus(hCompPort, 0, KILL_THREAD, NULL);
	}
}

bool ServerManager::SendPacket(SocketInfo * lpSocketInfo)
{
	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL);

	if (rtn == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("WSASend Failed...", errCode, false);
			return false;
		}
	}

	return true;
}

bool ServerManager::RecvPacket(SocketInfo * lpSocketInfo)
{
	if (lpSocketInfo->recvBuf->called)
		return true;

	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&lpSocketInfo->recvBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSARecv(lpSocketInfo->socket, &(lpSocketInfo->recvBuf->wsaBuf), 1,
		&dwRecvBytes, &dwFlags, &(lpSocketInfo->recvBuf->overlapped), NULL);

	if (rtn == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("WSARecv Failed...", errCode, false);
			return false;
		}
	}

	lpSocketInfo->recvBuf->called = true;
	return true;
}

bool ServerManager::HandleSendEvent(SocketInfo * lpSocketInfo, DWORD dwBytesTransferred)
{
	printf("Send Completed %dbytes", dwBytesTransferred);
	// Event에 따라 분기처리
	// 1. 초기 Connection 이후 RoomList Send Completion -> nothing.
	// 2. Response RoomList -> nothing
	// 3. Response Created Rooom -> Client state update
	// 4. Response Enter Room -> Client state update

	lpSocketInfo->recvBuf->lpPacket->ClearBuffer();
	if (!RecvPacket(lpSocketInfo))
		return false;

	return true;
}

// TCP는 데이터 경계가 없다는 거 기억하고, 그에 대한 처리 구현해야함... 
// 일단은 중간에 잘리지 않고 받는다고 가정하고 구현하고 있음...
// Event에 따라 분기처리
// 1. Request RoomList(by refreshing the room list)
// 2. Request Create Room (by creating a room)
// 3. Request Enter Room (by clicking a room in the room list)
// 4.
// 그외. Echo
bool ServerManager::HandleRecvEvent(SocketInfo* lpSocketInfo, DWORD dwBytesTransferred)
{
	lpSocketInfo->recvBuf->called = false;
	//if (dwBytesTransferred < 8)
	//{
	//	lpSocketInfo->recvBuf->lpPacket->ClearBuffer();
	//	if (!RecvPacket(lpSocketInfo))
	//		return false;
	//}

	int type, length;
	MessageLite* pMessage;
	// type, length만 역직렬화하는 함수도 만들어야겠다.
	lpSocketInfo->recvBuf->lpPacket->UnpackMessage(type, length, pMessage);
	
	bool rtn = (length == 0) ? HandleWithoutBody(lpSocketInfo, type)
		: HandleWithBody(lpSocketInfo, pMessage, type);
	if (!rtn)
		return false;

	if (!RecvPacket(lpSocketInfo))
		return false;

	return true;
}

bool ServerManager::HandleWithoutBody(SocketInfo* lpSocketInfo, int& type)
{
	if (type == MessageType::REFRESH)
	{
		lpSocketInfo->sendBuf->wsaBuf.len
			= lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomList);
		if (!SendPacket(lpSocketInfo))
			return false;
	}

	return true;
}

bool ServerManager::HandleWithBody(SocketInfo* lpSocketInfo, MessageLite* message, int& type)
{
	if (type == MessageType::DATA)
	{
		auto dataMap = ((Data*)message)->datamap();
		string contentType = dataMap["contentType"];
		// content type마다 <key, value>가 다를 것이므로 분기처리해야함
		if (contentType == "CREATE_ROOM")
		{
			string roomName = dataMap["roomName"];
			int limits = stoi(dataMap["limits"]);

			std::cout << "RoomName: " << roomName << ", " << "Limits: " << limits << std::endl;

			if (roomTable.find(roomName) != roomTable.end()) {
				// Room Name duplicated!!
				Data response;
				(*response.mutable_datamap())["contentType"] = "REJECT_CREATE_ROOM";
				(*response.mutable_datamap())["errorCode"] = "400";
				(*response.mutable_datamap())["errorMessage"] = "Duplicated Room Name";
				lpSocketInfo->sendBuf->wsaBuf.len =
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &response);
			}
			else 
			{
				Room* newRoom = new Room();
				InitRoom(newRoom, lpSocketInfo, roomName, limits);
				(*roomList.mutable_rooms())[newRoom->roomid()] = *newRoom;
				roomTable.insert(std::make_pair(roomName, newRoom->roomid()));
				lpSocketInfo->sendBuf->wsaBuf.len = 
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, newRoom);
			}
			if (!SendPacket(lpSocketInfo))
				return false;
		}
		else if(contentType == "ENTER_ROOM")
		{
			string roomName = dataMap["roomName"];
			int roomIdToEnter = roomTable[roomName];
			if (roomList.rooms().find(roomIdToEnter) == roomList.rooms().end()) {
				Data response;
				(*response.mutable_datamap())["contentType"] = "REJECT_ENTER_ROOM";
				(*response.mutable_datamap())["errorCode"] = "401";
				(*response.mutable_datamap())["errorMessage"] = "Room already has been destroyed!";
				lpSocketInfo->sendBuf->wsaBuf.len =
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &response);
			}
			else {
				int port;
				char ip[20];
			
				Room& room = (*roomList.mutable_rooms())[roomIdToEnter];
				lpSocketInfo->GetIpAndPort(ip, port);
				//Setting Client State
				Client* clnt;
				if (room.current() % 2) {
					clnt = room.add_blueteam();
					clnt->set_position(room.blueteam_size() + 7);
				}
				else {
					clnt = room.add_redteam();
					clnt->set_position(room.redteam_size() - 1);
				}
				clnt->set_ip(ip);
				clnt->set_port(port);
				clnt->set_ready(false);
				room.set_current(room.current() + 1);
				lpSocketInfo->sendBuf->wsaBuf.len =
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &room);
			}
			if (!SendPacket(lpSocketInfo))
				return false;
		}

		delete message;
		return true;
	}
	else
	{
		if (type == MessageType::ROOMLIST)
		{

		}
		// ....

		return true;
	}
}




