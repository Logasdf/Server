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
	client->set_name(userName);
	client->set_position(0);
	client->set_ready(false);

	WaitForSingleObject(hMutexObj, INFINITE);

	(*roomList.mutable_rooms())[roomIdStatus] = *pRoomInfo;
	roomTable.insert(std::make_pair(roomName, roomIdStatus));
	Room* room = new Room(&((*roomList.mutable_rooms())[roomIdStatus]));
	room->AddClientInfo(lpSocketInfo, userName);
	serverRoomList[roomIdStatus++] = room;

	ReleaseMutex(hMutexObj);
}

ServerManager::ServerManager() 
{ 
	roomIdStatus = 100;
	hCompPort = NULL; 
	servSock = INVALID_SOCKET;
	hMutexObj = CreateMutex(NULL, FALSE, NULL);
	hMutexForSend = CreateMutex(NULL, FALSE, NULL);
	hMutexForRecv = CreateMutex(NULL, FALSE, NULL);
}

ServerManager::~ServerManager() 
{ 
	if (hCompPort != NULL)
		CloseHandle(hCompPort);
	if (servSock != INVALID_SOCKET)
		closesocket(servSock);
	if (hMutexObj != NULL)
		CloseHandle(hMutexObj);
	if (hMutexForSend != NULL)
		CloseHandle(hMutexForSend);
	if (hMutexForRecv != NULL)
		CloseHandle(hMutexForRecv);

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

			// socket context ����(Completion Key�� �ѱ�)
			lpSocketInfo = SocketInfo::AllocateSocketInfo(clntSock);
			if (lpSocketInfo == NULL)
			{
				ErrorHandling("Socket Info Object Allocation Failed...", false);
				continue;
			}

			// IOCP�� clnt socket ����
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
			if (SOCKET_ERROR == setsockopt(lpSocketInfo->socket, SOL_SOCKET, SO_LINGER,
				(char*)&LingerStruct, sizeof(LingerStruct))) {
				fprintf(stderr, "Invalid socket.... %d\n", WSAGetLastError());
				ReleaseMutex(hMutexObj);
				return;
			}
		}
		closesocket(lpSocketInfo->socket);

		SocketInfo::DeallocateSocketInfo(lpSocketInfo);
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

		bool rtn = GetQueuedCompletionStatus(self->hCompPort, &dwBytesTransferred,
			reinterpret_cast<ULONG_PTR*>(&lpSocketInfo), reinterpret_cast<LPOVERLAPPED*>(&lpIOInfo), INFINITE);
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
				fprintf(stderr, "[Current Thread #%d] => ", GetCurrentThreadId());
				fprintf(stderr, "#%d will close: %d\n", lpSocketInfo->socket, WSAGetLastError());
				//ErrorHandling("Client Connection Close, Socket will close", false);
				self->CloseClient(lpSocketInfo);
			}
			continue;
		}

		try
		{
			//fprintf(stderr, "[Current Thread #%d] => ", GetCurrentThreadId());
			if (dwBytesTransferred == 0)
			{
				ErrorHandling("dwBytesTransferred == 0...", WSAGetLastError(), false);
				fprintf(stderr, "[Log]: Client %d Connection Closed....\n", (HANDLE)lpSocketInfo->socket);
				throw "[Cause]: dwBytesTransferr == 0";
			}

			if (lpIOInfo == lpSocketInfo->recvBuf)
			{
				//LOG("Complete Receiving Message!!");
				if (!(self->HandleRecvEvent(lpSocketInfo, dwBytesTransferred)))
				{
					throw "[Cause]: RecvEvent Handling Error!!";
				}
			}
			else if (lpIOInfo == lpSocketInfo->sendBuf)
			{
				//LOG("Complete Sending Message!!");
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

bool ServerManager::SendPacket(SocketInfo * lpSocketInfo)
{
	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;

	//WaitForSingleObject(hMutexForSend, INFINITE);
	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL);
	//ReleaseMutex(hMutexForSend);

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

	checkCall[(int)lpSocketInfo->socket]++;
	//fprintf(stderr, "[Socket #%d]: %d\n", lpSocketInfo->socket, checkCall[(int)lpSocketInfo->socket]);
	if (checkCall[(int)lpSocketInfo->socket] > 1) {
		fprintf(stderr, "[Socket #%d]: %d\n", lpSocketInfo->socket, checkCall[(int)lpSocketInfo->socket]);
	}

	//WaitForSingleObject(hMutexForRecv, INFINITE);
	ZeroMemory(&lpSocketInfo->recvBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSARecv(lpSocketInfo->socket, &(lpSocketInfo->recvBuf->wsaBuf), 1,
		&dwRecvBytes, &dwFlags, &(lpSocketInfo->recvBuf->overlapped), NULL);
	//ReleaseMutex(hMutexForRecv);

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
	WaitForSingleObject(hMutexObj, INFINITE);

	lpSocketInfo->sendBuf->lpPacket->ClearBuffer();
	//fprintf(stderr, "Send Bytes: %d\n", dwBytesTransferred);

	lpSocketInfo->recvBuf->lpPacket->ClearBuffer();
	if (!RecvPacket(lpSocketInfo)) {
		ReleaseMutex(hMutexObj);
		return false;
	}

	ReleaseMutex(hMutexObj);

	return true;
}

bool ServerManager::HandleRecvEvent(SocketInfo* lpSocketInfo, DWORD dwBytesTransferred)
{

	//fprintf(stderr, "[Socket #%d]: %d\n", lpSocketInfo->socket, checkCall[(int)lpSocketInfo->socket]);

	int type, length;
	MessageLite* pMessage;

	WaitForSingleObject(hMutexObj, INFINITE);
	lpSocketInfo->recvBuf->called = false;
	checkCall[(int)lpSocketInfo->socket]--;

	//fprintf(stderr, "[Current Thread %d]=> ", GetCurrentThreadId());
	//fprintf(stderr, "Socket: %d, Transferred: %d\n", lpSocketInfo->socket, dwBytesTransferred);

	//rtn�� false�� ���, Unpack�ϱ⿡�� ������ ������ ���̹Ƿ� Re-recv Call
	//bool rtn = lpSocketInfo->recvBuf->lpPacket->UnpackMessage(dwBytesTransferred, type, length, pMessage);
	//if (!rtn) {
	//	rtn = RecvPacket(lpSocketInfo);
	//	ReleaseMutex(hMutexObj);
	//	return rtn;
	//}

	lpSocketInfo->recvBuf->lpPacket->UnpackMessage(type, length, pMessage);

	bool rtn = (length == 0) ? HandleWithoutBody(lpSocketInfo, type) : HandleWithBody(lpSocketInfo, pMessage, type);
	if (!rtn) {
		ReleaseMutex(hMutexObj);
		return false;
	}

	lpSocketInfo->recvBuf->lpPacket->ClearBuffer();
	if (!RecvPacket(lpSocketInfo)) {
		ReleaseMutex(hMutexObj);
		return false;
	}

	ReleaseMutex(hMutexObj);

	return true;
}

bool ServerManager::HandleWithoutBody(SocketInfo* lpSocketInfo, int& type)
{
	if (type == MessageType::REFRESH)
	{
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
		// content type���� <key, value>�� �ٸ� ���̹Ƿ� �б�ó���ؾ���
		if (contentType == "CREATE_ROOM")
		{
			string roomName = dataMap["roomName"];
			string userName = dataMap["userName"];
			int limits = stoi(dataMap["limits"]);

			std::cout << "RoomName: " << roomName << ", " << "Limits: " << limits << "Username: " << userName << std::endl;

			if (roomTable.find(roomName) != roomTable.end()) 
			{   // Room Name duplicated!!
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
			{  // ���������� ������ ������ ��Ȳ
				RoomInfo* newRoomInfo = new RoomInfo();
				InitRoom(newRoomInfo, lpSocketInfo, roomName, limits, userName);
				lpSocketInfo->sendBuf->wsaBuf.len = 
					lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, newRoomInfo);

				if (!SendPacket(lpSocketInfo))
					return false;
			}
		}
		else if(contentType == "ENTER_ROOM")
		{ // �����Ϸ��� ������ ���� �������� �ʴ� ���
			string roomName = dataMap["roomName"];
			int roomIdToEnter = roomTable[roomName];
			if (roomList.rooms().find(roomIdToEnter) == roomList.rooms().end()) 
			{
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
			{ //�����ϰ��� �ϴ� ���� �����ϴ� ��Ȳ.
				RoomInfo& roomInfo = (*roomList.mutable_rooms())[roomIdToEnter];
				
				if (roomInfo.current() == roomInfo.limit())
				{ // �ο��� �������.
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
				{ // �� ���� ó��
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
					clnt->set_name(userName);
					clnt->set_ready(false);
					roomInfo.set_current(roomInfo.current() + 1);
					lpSocketInfo->sendBuf->wsaBuf.len =
						lpSocketInfo->sendBuf->lpPacket->PackMessage(-1, &roomInfo);

					if (!SendPacket(lpSocketInfo))
						return false;

					Room* room = serverRoomList[roomIdToEnter];
					BroadcastMessage(room, clnt); //Ȥ�� �����Ͱ� �� ������������ data�� ����� ���ɼ��� �ֳ�?
					room->AddClientInfo(lpSocketInfo);
				}
			}
		}
		else if (contentType == "READY_EVENT") 
		{
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["position"]);
			Room* room = serverRoomList[roomId];
			room->ProcessReadyEvent(position);
			BroadcastMessage(room, message);
		}
		else if (contentType == "TEAM_CHANGE") 
		{
			std::cout << "team change called" << std::endl;
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["prev_position"]);
			Room* room = serverRoomList[roomId];
			int newPosition = room->ProcessTeamChangeEvent(position);
			if (newPosition == -1)
			{
				//�̵��� �Ұ��� �� ���. �ƹ��͵� ������������ �ƹ��ൿ�� �Ͼ�� ���� ���̱⿡ �̴�� ���ֵ� ���ᾲ
			}
			// ������ �����ϴٸ� ��ü�� ���� ���� �ʿ䰡 ��������.
			Data data;
			(*data.mutable_datamap())["contentType"] = dataMap["contentType"];
			(*data.mutable_datamap())["prev_position"] = dataMap["prev_position"];
			(*data.mutable_datamap())["next_position"] = std::to_string(newPosition);
			BroadcastMessage(room, &data);
		}
		else if (contentType == "LEAVE_GAMEROOM") 
		{
			std::cout << "leave gameroom called" << std::endl;
			int roomId = stoi(dataMap["roomId"]);
			int position = stoi(dataMap["position"]);
			Room* room = serverRoomList[roomId];
			
			BroadcastMessage(room, message);

			bool hostChanged = false;
			bool isClosed =  room->ProcessLeaveGameroomEvent(position, lpSocketInfo, hostChanged);
			RoomInfo& roomInfo = (*roomList.mutable_rooms())[roomId];

			if (isClosed)
			{ //���� ����� ���, ���ҽ� �����ؾ���
				serverRoomList.erase(roomId); // Room* ���� �� ����Ʈ���� ���� (���� �ƴ�)
				roomTable.erase(roomInfo.name()); // Map<���̸�, roomId> ���� ����
				(*roomList.mutable_rooms()).erase(roomId); // RoomInfo* ���ۿ� ����Ʈ���� ���� (�ڵ����� ������)
				delete room; // Room* ���� ���⼭
			}
			else
			{
				if (hostChanged)
				{ //������ �ٲ� ���. �ٲ� ������ position �� �����ָ� �ȴ�.
					Data data;
					(*data.mutable_datamap())["contentType"] = "HOST_CHANGED";
					(*data.mutable_datamap())["newHost"] = std::to_string(roomInfo.host());
					BroadcastMessage(room, &data);
				}
			}
		}
		else if (contentType == "CHAT_MESSAGE") 
		{
			std::cout << "chat message called" << std::endl;
		}
		else if (contentType == "START_GAME")
		{
			std::cout << "start game called" << std::endl;
			int roomId = stoi(dataMap["roomId"]);

			//WaitForSingleObject(hMutexObj, INFINITE);
			Room*& _room = serverRoomList[roomId];
			BroadcastMessage(_room, nullptr, START_GAME);
			//ReleaseMutex(hMutexObj);
		}
		else if (contentType == "FIRE_BULLET")
		{
			//std::cout << "fire_bullet" << std::endl;
			string& strRoomId = dataMap["roomId"];
			string& fromClnt = dataMap["fromClnt"];
			if (strRoomId == "" || fromClnt == "") return true;
			int roomId = stoi(strRoomId);

			Data response;
			(*response.mutable_datamap())["contentType"] = "FIRE_BULLET";
			(*response.mutable_datamap())["fromClnt"] = fromClnt;

			fprintf(stderr, "[#%d]: Fire Bullet\n", GetCurrentThreadId());
			Room*& _room = serverRoomList[roomId];
			BroadcastMessage(_room, &response);
		}
		else if (contentType == "BE_SHOT") {
			//std::cout << "be_shot" << std::endl;
			//int roomId = stoi(dataMap["roomId"]);
			//string fromClnt = dataMap["fromClnt"];
			//string toClnt = dataMap["toClnt"];
			////string hitType = dataMap["hitType"];

			//Data response;
			//(*response.mutable_datamap())["contentType"] = "BE_SHOT";
			//(*response.mutable_datamap())["fromClnt"] = fromClnt;
			//(*response.mutable_datamap())["toClnt"] = toClnt;
			////(*response.mutable_datamap)["hitType"] = toClnt;

			////WaitForSingleObject(hMutexObj, INFINITE);
			//Room*& _room = serverRoomList[roomId];
			//SocketInfo*& toClntSock = _room->GetSocketUsingName(toClnt);

			//toClntSock->sendBuf->wsaBuf.len =
			//	toClntSock->sendBuf->lpPacket->PackMessage(-1, &response);
			////ReleaseMutex(hMutexObj);
			//if (!SendPacket(toClntSock))
			//	return false;
		}

		//delete message;
		return true;
	}
	else
	{
		if (type == MessageType::PLAY_STATE)
		{
			//fprintf(stderr, "[#%d]: PlayState\n", GetCurrentThreadId());
			int roomId = ((PlayState*)message)->roomid();
			if (serverRoomList.find(roomId) != serverRoomList.end()) {
				BroadcastMessage(serverRoomList[roomId], message);
			}
			//delete message;
		}

		return true;
	}
}

void ServerManager::SendInitData(SocketInfo* lpSocketInfo) {
	static int indicator = 1;

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
		}
	}
	
	dwFlags = dwSendBytes = 0;
	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(OVERLAPPED));

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
}

void ServerManager::BroadcastMessage(Room * room, MessageLite * message, int type)
{
	auto begin = room->ClientSocketsBegin();
	auto end = room->ClientSocketsEnd();

	for (auto itr = begin; itr != end; itr++) {
		if (message != nullptr) {
			(*itr)->sendBuf->wsaBuf.len =
				(*itr)->sendBuf->lpPacket->PackMessage(-1, message);
			//fprintf(stderr, "Length: %d\n", (*itr)->sendBuf->wsaBuf.len);
		}
		else if(type != -1) {
			(*itr)->sendBuf->wsaBuf.len =
				(*itr)->sendBuf->lpPacket->PackMessage(type, nullptr);
		}
		
		if (!SendPacket(*itr))
			std::cout << "�޽��� ���� ���о�" << std::endl;
	}
}




