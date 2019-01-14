#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <process.h>
#include <string>
#include "ServerManager.h"
#include "ErrorHandle.h"
#include "def.h"
#include "Packet.h"

void ServerManager::InitRoom(RoomInfo* pRoomInfo, SocketInfo* lpSocketInfo, string& roomName, int& limits, string& userName)
{
	pRoomInfo->set_host(0);
	pRoomInfo->set_current(1);
	pRoomInfo->set_limit(limits);
	pRoomInfo->set_name(roomName);
	pRoomInfo->set_readycount(0);
	pRoomInfo->set_roomid(roomIdStatus);

	Client* client = pRoomInfo->add_redteam();
	client->set_clntid(roomIdStatus);
	client->set_name(userName);
	client->set_position(0);
	client->set_ready(false);
	EnterCriticalSection(&csForClientLocationTable);
	clientLocationTable[lpSocketInfo] = client;
	LeaveCriticalSection(&csForClientLocationTable);

	roomTable.insert(std::make_pair(roomName, roomIdStatus));
	EnterCriticalSection(&csForRoomList);
	(*roomList.mutable_rooms())[roomIdStatus] = *pRoomInfo;
	Room* room = new Room(&((*roomList.mutable_rooms())[roomIdStatus]));
	LeaveCriticalSection(&csForRoomList);
	room->AddClientInfo(lpSocketInfo);
	EnterCriticalSection(&csForServerRoomList);
	serverRoomList[roomIdStatus++] = room;
	LeaveCriticalSection(&csForServerRoomList);
}

ServerManager::ServerManager() 
{ 
	roomIdStatus = 100;
	hCompPort = NULL; 
	servSock = INVALID_SOCKET;
	hMutexObj = CreateMutex(NULL, FALSE, NULL);
	InitializeCriticalSection(&csForRoomList);
	InitializeCriticalSection(&csForServerRoomList);
	InitializeCriticalSection(&csForRoomTable);
	InitializeCriticalSection(&csForClientLocationTable);
}

ServerManager::~ServerManager() 
{ 
	if (hCompPort != NULL)
		CloseHandle(hCompPort);
	if (servSock != INVALID_SOCKET)
		closesocket(servSock);
	if (hMutexObj != NULL)
		CloseHandle(hMutexObj);

	DeleteCriticalSection(&csForRoomList);
	DeleteCriticalSection(&csForServerRoomList);
	DeleteCriticalSection(&csForRoomTable);
	DeleteCriticalSection(&csForClientLocationTable);

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
				//클라이언트 접속이 끊어지는 부분
				self->ProcessDisconnection(lpSocketInfo);
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
			
			SendInitData(lpSocketInfo);
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
	printf("Send Completed %dbytes\n", dwBytesTransferred);
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
	{ //여긴 동기화가 필요할까? 아직 잘 모르겠다. 0이아닌데 0이면 그냥 다시 refresh할꺼고, 0인데 가면 사라졌다는 메시지 뜰테니..
		if(roomList.rooms_size() == 0) 
		{
			lpSocketInfo->sendBuf->wsaBuf.len
				= lpSocketInfo->sendBuf->lpPacket->PackMessage(MessageType::EMPTY_ROOMLIST, NULL);
		}
		else
		{
			lpSocketInfo->sendBuf->wsaBuf.len
				= lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomList);
		}

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
			string userName = dataMap["userName"];
			int limits = stoi(dataMap["limits"]);

			std::cout << "RoomName: " << roomName << ", " << "Limits: " << limits << "Username: " << userName << std::endl;
			EnterCriticalSection(&csForRoomTable);
			if (roomTable.find(roomName) != roomTable.end()) 
			{   // Room Name duplicated!!
				LeaveCriticalSection(&csForRoomTable);
				Data response;
				(*response.mutable_datamap())["contentType"] = "REJECT_CREATE_ROOM";
				(*response.mutable_datamap())["errorCode"] = "400";
				(*response.mutable_datamap())["errorMessage"] = "Duplicated Room Name";
				lpSocketInfo->sendBuf->wsaBuf.len =
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &response);

				if (!SendPacket(lpSocketInfo))
					return false;
			}
			else 
			{  // 정상적으로 생성이 가능한 상황
				RoomInfo* newRoomInfo = new RoomInfo();
				InitRoom(newRoomInfo, lpSocketInfo, roomName, limits, userName);
				LeaveCriticalSection(&csForRoomTable);
				lpSocketInfo->sendBuf->wsaBuf.len = 
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, newRoomInfo);
				
				if (!SendPacket(lpSocketInfo))
					return false;
			}
			
		}
		else if(contentType == "ENTER_ROOM")
		{ // 입장하려는 시점에 방이 존재하지 않는 경우
			string roomName = dataMap["roomName"];
			EnterCriticalSection(&csForRoomTable);
			int roomIdToEnter = roomTable[roomName];
			LeaveCriticalSection(&csForRoomTable);

			EnterCriticalSection(&csForRoomList);
			if (roomList.rooms().find(roomIdToEnter) == roomList.rooms().end()) 
			{
				LeaveCriticalSection(&csForRoomList);
				Data response;
				(*response.mutable_datamap())["contentType"] = "REJECT_ENTER_ROOM";
				(*response.mutable_datamap())["errorCode"] = "401";
				(*response.mutable_datamap())["errorMessage"] = "Room already has been destroyed!";
				lpSocketInfo->sendBuf->wsaBuf.len =
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &response);

				if (!SendPacket(lpSocketInfo))
					return false;
			}
			else 
			{ //입장하고자 하는 방이 존재하는 상황.
				RoomInfo& roomInfo = (*roomList.mutable_rooms())[roomIdToEnter];
				
				if (roomInfo.current() == roomInfo.limit())
				{ // 인원수 꽉찬경우.
					LeaveCriticalSection(&csForRoomList);
					Data response;
					(*response.mutable_datamap())["contentType"] = "REJECT_ENTER_ROOM";
					(*response.mutable_datamap())["errorCode"] = "401";
					(*response.mutable_datamap())["errorMessage"] = "The Room is already full!";
					lpSocketInfo->sendBuf->wsaBuf.len =
						lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &response);

					if (!SendPacket(lpSocketInfo))
						return false;
				}
				else
				{ // 방 입장 처리
					Client* clnt;
					string userName = dataMap["userName"];

					if (roomInfo.redteam_size() > roomInfo.blueteam_size()) {
						clnt = roomInfo.add_blueteam();
						clnt->set_position(roomInfo.blueteam_size() + 7);
					}
					else {
						clnt = roomInfo.add_redteam();
						clnt->set_position(roomInfo.redteam_size() - 1);
					}
					clnt->set_clntid(roomInfo.roomid());
					clnt->set_name(userName);
					clnt->set_ready(false);
					clientLocationTable[lpSocketInfo] = clnt;

					roomInfo.set_current(roomInfo.current() + 1);
					lpSocketInfo->sendBuf->wsaBuf.len =
						lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomInfo);
					LeaveCriticalSection(&csForRoomList);
					if (!SendPacket(lpSocketInfo))
						return false;

					EnterCriticalSection(&csForServerRoomList);
					Room* room = serverRoomList[roomIdToEnter];
					BroadcastMessage(room, clnt); //혹시 데이터가 다 보내지기전에 data가 사라질 가능성이 있나?
					room->AddClientInfo(lpSocketInfo);
					LeaveCriticalSection(&csForServerRoomList);
				}
			}
		}
		else if (contentType == "READY_EVENT") 
		{
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["position"]);
			EnterCriticalSection(&csForServerRoomList);
			Room* room = serverRoomList[roomId];
			room->ProcessReadyEvent(position);
			BroadcastMessage(room, message);
			LeaveCriticalSection(&csForServerRoomList);
		}
		else if (contentType == "TEAM_CHANGE") 
		{
			std::cout << "team change called" << std::endl;
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["prev_position"]);
			EnterCriticalSection(&csForServerRoomList);
			Room* room = serverRoomList[roomId];
			int newPosition = room->ProcessTeamChangeEvent(position);
			if (newPosition == -1)
			{
				//이동이 불가능 할 경우. 아무것도 보내지않으면 아무행동도 일어나지 않을 것이기에 이대로 놔둬도 괜춘쓰
			}
			// 수정이 가능하다면 객체를 새로 만들 필요가 없어질듯.
			Data data;
			(*data.mutable_datamap())["contentType"] = dataMap["contentType"];
			(*data.mutable_datamap())["prev_position"] = dataMap["prev_position"];
			(*data.mutable_datamap())["next_position"] = std::to_string(newPosition);
			BroadcastMessage(room, &data);
			LeaveCriticalSection(&csForServerRoomList);
		}
		else if (contentType == "LEAVE_GAMEROOM") 
		{
			std::cout << "leave gameroom called" << std::endl;
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["position"]);
			EnterCriticalSection(&csForServerRoomList);
			Room* room = serverRoomList[roomId];
			
			BroadcastMessage(room, message);

			bool hostChanged = false;
			bool isClosed =  room->ProcessLeaveGameroomEvent(position, lpSocketInfo, hostChanged);
			EnterCriticalSection(&csForClientLocationTable);
			clientLocationTable[lpSocketInfo] = nullptr;
			LeaveCriticalSection(&csForClientLocationTable);

			RoomInfo& roomInfo = (*roomList.mutable_rooms())[roomId];
			if (isClosed)
			{ //방이 사라진 경우, 리소스 정리해야함
				serverRoomList.erase(roomId); // Room* 서버 방 리스트에서 제거 (해제 아님)
				EnterCriticalSection(&csForRoomTable);
				roomTable.erase(roomInfo.name()); // Map<방이름, roomId> 에서 제거
				LeaveCriticalSection(&csForRoomTable);
				EnterCriticalSection(&csForRoomList);
				(*roomList.mutable_rooms()).erase(roomId); // RoomInfo* 전송용 리스트에서 제거 (자동으로 해제됨)
				LeaveCriticalSection(&csForRoomList);
				delete room; // Room* 해제 여기서
			}
			else
			{
				if (hostChanged)
				{ //방장이 바뀐 경우. 바뀐 방장의 position 을 보내주면 된다.
					Data data;
					(*data.mutable_datamap())["contentType"] = "HOST_CHANGED";
					(*data.mutable_datamap())["newHost"] = std::to_string(roomInfo.host());
					BroadcastMessage(room, &data);
				}
			}
			LeaveCriticalSection(&csForServerRoomList);
		}
		else if (contentType == "CHAT_MESSAGE") 
		{
			std::cout << "chat message called" << std::endl;
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

void ServerManager::SendInitData(SocketInfo* lpSocketInfo) {
	static int indicator = 1;

	DWORD dwFlags, dwSendBytes;
	dwFlags = dwSendBytes = 0;
	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(OVERLAPPED));

	// Serialize Room List;
	EnterCriticalSection(&csForRoomList);
	lpSocketInfo->sendBuf->wsaBuf.len =
		lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomList);
	LeaveCriticalSection(&csForRoomList);

	if (WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("Init Send Operation(Send Room List) Error!!", errCode, false);
		}
	}
	
	dwFlags = dwSendBytes = 0;
	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(OVERLAPPED));
	//indicator에도 동기화가 필요하지만 서버에서 유저네임을 alloc하는 방법을 확정한게 아니니 일단 놔둠
	Data data;
	(*data.mutable_datamap())["contentType"] = "ASSIGN_USERNAME";
	(*data.mutable_datamap())["userName"] = "TempUser" + std::to_string(indicator);
	indicator++;

	lpSocketInfo->sendBuf->wsaBuf.len =
		lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &data);
	if (WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL) == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("Init Send Operation(Send User Name) Error!!", errCode, false);
		}
	}
	EnterCriticalSection(&csForClientLocationTable);
	clientLocationTable.insert(std::make_pair(lpSocketInfo, nullptr));
	LeaveCriticalSection(&csForClientLocationTable);
}

void ServerManager::BroadcastMessage(Room * room, MessageLite * message)
{
	auto begin = room->ClientSocketsBegin();
	auto end = room->ClientSocketsEnd();

	for (auto itr = begin; itr != end; itr++) {
		(*itr)->sendBuf->wsaBuf.len =
			(*itr)->sendBuf->lpPacket->PackMessage(-1, message);
		
		if (!SendPacket(*itr))
			std::cout << "메시지 전송 실패쓰" << std::endl;
	}
}

void ServerManager::ProcessDisconnection(SocketInfo * lpSocketInfo)
{   //상당히 많은 부분이 LEAVE_GAMEROOM 부분과 겹치기 때문에 중복을 어케 처리할 필요가 있을듯
	EnterCriticalSection(&csForClientLocationTable);
	Client* clientInstance = clientLocationTable[lpSocketInfo];
	if (clientInstance != nullptr)
	{
		int roomId = clientInstance->clntid();
		EnterCriticalSection(&csForServerRoomList);
		Room* currentLocation = serverRoomList[roomId];

		Data message;
		(*message.mutable_datamap())["contentType"] = "LEAVE_GAMEROOM";
		(*message.mutable_datamap())["position"] = std::to_string(clientInstance->position());

		bool hostChanged = false;
		bool isClosed = currentLocation->ProcessLeaveGameroomEvent(clientInstance->position(), lpSocketInfo, hostChanged);
		BroadcastMessage(currentLocation, &message);

		RoomInfo& roomInfo = (*roomList.mutable_rooms())[roomId];
		if (isClosed)
		{ //방이 사라진 경우, 리소스 정리해야함
			serverRoomList.erase(roomId); // Room* 서버 방 리스트에서 제거 (해제 아님)
			EnterCriticalSection(&csForRoomTable);
			roomTable.erase(roomInfo.name()); // Map<방이름, roomId> 에서 제거
			LeaveCriticalSection(&csForRoomTable);
			EnterCriticalSection(&csForRoomList);
			(*roomList.mutable_rooms()).erase(roomId); // RoomInfo* 전송용 리스트에서 제거 (자동으로 해제됨)
			LeaveCriticalSection(&csForRoomList);
			delete currentLocation; // Room* 해제 여기서
		}
		else
		{
			if (hostChanged)
			{ //방장이 바뀐 경우. 바뀐 방장의 position 을 보내주면 된다.
				Data data;
				(*data.mutable_datamap())["contentType"] = "HOST_CHANGED";
				(*data.mutable_datamap())["newHost"] = std::to_string(roomInfo.host());
				BroadcastMessage(currentLocation, &data);
			}
		}
		LeaveCriticalSection(&csForServerRoomList);
	}

	clientLocationTable.erase(lpSocketInfo);
	LeaveCriticalSection(&csForClientLocationTable);
}




