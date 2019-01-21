#include <process.h>
#include "def.h"
#include "Room.h"
#include "ServerManager.h"

Room::Room(RoomInfo * initVal) : roomInfo(initVal)
{
	InitializeCriticalSection(&csForClientSockets);
	InitializeCriticalSection(&csForRoomInfo);
	InitializeCriticalSection(&csForBroadcast);
	gameStarted = false;
}

Room::~Room()
{
	DeleteCriticalSection(&csForClientSockets);
	DeleteCriticalSection(&csForRoomInfo);
	DeleteCriticalSection(&csForBroadcast);
	CloseHandle(hCompPort);

	std::cout << "~Room() called" << std::endl;
}

void Room::AddClientInfo(SocketInfo * lpSocketInfo, string& userName)
{
	EnterCriticalSection(&csForClientSockets);
	clientSockets.push_front(lpSocketInfo);
	LeaveCriticalSection(&csForClientSockets);
	clientMap[userName] = lpSocketInfo;
}

void Room::RemoveClientInfo(SocketInfo * lpSocketInfo, string& userName)
{
	EnterCriticalSection(&csForClientSockets);
	clientSockets.remove(lpSocketInfo);
	LeaveCriticalSection(&csForClientSockets);
	if (clientMap.find(userName) != clientMap.end()) {
		clientMap.erase(userName);
	}
}

std::forward_list<SocketInfo*>::const_iterator Room::ClientSocketsBegin()
{
	EnterCriticalSection(&csForClientSockets);
	auto itr = clientSockets.cbegin();
	LeaveCriticalSection(&csForClientSockets);
	return itr;
}

std::forward_list<SocketInfo*>::const_iterator Room::ClientSocketsEnd()
{
	EnterCriticalSection(&csForClientSockets);
	auto itr = clientSockets.cend();
	LeaveCriticalSection(&csForClientSockets);
	return itr;
}

void Room::ProcessReadyEvent(Client*& affectedClient)
{
	EnterCriticalSection(&csForRoomInfo);
	bool toReady = !affectedClient->ready() ? true : false;
	if (toReady) 
	{
		roomInfo->set_readycount(roomInfo->readycount() + 1);
	} 
	else 
	{
		roomInfo->set_readycount(roomInfo->readycount() - 1);
	}	
	affectedClient->set_ready(toReady);
	LeaveCriticalSection(&csForRoomInfo);
}

Client* Room::ProcessTeamChangeEvent(Client*& affectedClient)
{
	//1. 현재 팀이 어느 팀인가를 확인하고 타 팀에 존재하는 빈자리가 있는지 확인
	//   -> 빈자리가 없으면 -1을 반환.
	//2. 클라이언트의 자리를 옮기고, 각각 팀의 유저 수를 적절하게 증감시킨다. -> 자동으로 함
	//3. 새로운 자리의 인덱스를 반환한다.
	//방장인 경우에는 host위치를 변경해주어야함 ** 빼먹었다가 추가함
	bool isOnRedTeam = affectedClient->position() < BLUEINDEXSTART ? true : false;
	
	int maxuser = roomInfo->limit() / 2;
	int nextIdx;
	Client* newClient;
	EnterCriticalSection(&csForRoomInfo);
	if (isOnRedTeam) 
	{
		nextIdx = roomInfo->blueteam_size();
		if (nextIdx == maxuser)
			return nullptr;
		
		nextIdx += BLUEINDEXSTART;
		newClient = MoveClientToOppositeTeam(affectedClient, nextIdx, roomInfo->mutable_redteam(), roomInfo->mutable_blueteam());
	}
	else 
	{
		nextIdx = roomInfo->redteam_size();
		if (nextIdx == maxuser)
			return nullptr;

		newClient = MoveClientToOppositeTeam(affectedClient, nextIdx, roomInfo->mutable_blueteam(), roomInfo->mutable_redteam());
	}
	LeaveCriticalSection(&csForRoomInfo);
	return newClient;
}

bool Room::ProcessLeaveGameroomEvent(Client*& affectedClient, SocketInfo* lpSocketInfo) // ret T - if the room obj needs to be destroyed, F - otherwise.
{
	int position = affectedClient->position();
	bool isOnRedTeam = position < BLUEINDEXSTART ? true : false;
	bool isClosed = false;

	EnterCriticalSection(&csForRoomInfo);
	if (affectedClient->ready())
		roomInfo->set_readycount(roomInfo->readycount() - 1);

	if (isOnRedTeam) // 클라이언트 객체를 리스트에서 삭제한다 ( 해제까지 해줌 )
		roomInfo->mutable_redteam()->DeleteSubrange(position, 1);
	else
		roomInfo->mutable_blueteam()->DeleteSubrange(position % BLUEINDEXSTART, 1);

	EnterCriticalSection(&csForClientSockets);
	clientSockets.remove(lpSocketInfo); //클라이언트 소켓을 forward_list에서 제거. 게임종료가 아니라서 해제는 하면 안 됨.
	LeaveCriticalSection(&csForClientSockets);

	if (roomInfo->current() == 1)
	{
		isClosed = true; // 혼자있는 상황이라 방 깨지는 경우
	}
	else
	{
		if (roomInfo->host() == position)
		{ //2 - 방에 혼자가 아닌데, 방장인 경우
			ChangeGameroomHost(isOnRedTeam); //방장 변경
		}
	}

	AdjustClientsIndexes(position);
	roomInfo->set_current(roomInfo->current() - 1); //인원수 줄이기
	LeaveCriticalSection(&csForRoomInfo);
	return isClosed;
}

bool Room::CanStart()
{	// current == ready_cnt + 1 인 경우면 모든 사람이 레디 완료된 상황
	EnterCriticalSection(&csForRoomInfo);
	bool ret = (roomInfo->current() - 1) == roomInfo->readycount() ? true : false;
	LeaveCriticalSection(&csForRoomInfo);
	return ret;
}

void Room::SetGameStartFlag(bool to)
{
	gameStarted = to;
}

bool Room::HasGameStarted() const
{
	return gameStarted;
}

SocketInfo*& Room::GetSocketUsingName(string & userName)
{
	return clientMap[userName];
}

void Room::InitCompletionPort(int maxNumberOfThreads)
{
	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxNumberOfThreads);
	if (hCompPort == NULL) {
		ErrorHandling(WSAGetLastError());
		return;
	}
}

void Room::CreateThreadPool(int numOfThreads)
{
	for (int i = 0; i < numOfThreads; ++i) {
		DWORD dwThreadId = 0;
		HANDLE hThread = BEGINTHREADEX(NULL, 0, Room::ThreadMain, this, 0, &dwThreadId);
		CloseHandle(hThread);
	}
}

unsigned __stdcall Room::ThreadMain(void * pVoid)
{
	Room* self = (Room*)pVoid;
	MessageLite* pMessage;
	LPOVERLAPPED lpOverlapped;

	DWORD dwBytesTransferred = 0;
	ServerManager& servManager = ServerManager::getInstance();

	while (true)
	{
		pMessage = NULL;
		lpOverlapped = NULL;

		bool rtn = GetQueuedCompletionStatus(self->hCompPort, &dwBytesTransferred,
			reinterpret_cast<ULONG_PTR*>(&pMessage), &lpOverlapped, INFINITE);
		if (!rtn) {
			if (lpOverlapped != NULL) {
				printf("lpOverlapped is not NULL!: %d\n", GetLastError());
			}
			else {
				printf("lpOverlapped is NULL!: %d\n", GetLastError());
			}
			continue;
		}

		if (pMessage == NULL) {
			ErrorHandling("Message is NULL....", false);
			continue;
		}

		// Broadcast
		EnterCriticalSection(&self->csForBroadcast);

		auto begin = self->ClientSocketsBegin();
		auto end = self->ClientSocketsEnd();
		printf("Broadcast!!\n");
		for (auto itr = begin; itr != end; itr++) {
			if (pMessage != nullptr) {
				(*itr)->sendBuf->wsaBuf.len =
					(*itr)->sendBuf->lpPacket->PackMessage(-1, pMessage);
			}

			if (!servManager.SendPacket(*itr))
				std::cout << "Send Message Failed\n";
		}

		LeaveCriticalSection(&self->csForBroadcast);
		//test 
		delete pMessage;
	}

	return 0;
}

Client* Room::GetClient(int position)
{
	return position < BLUEINDEXSTART ? 
		roomInfo->mutable_redteam(position) : roomInfo->mutable_blueteam(position % BLUEINDEXSTART);
}

Client* Room::MoveClientToOppositeTeam(Client*& affectedClient, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo)
{
	Client* newClient = addTo->Add();
	newClient->CopyFrom(*affectedClient);
	int prev_pos = affectedClient->position();
	if (affectedClient->position() == roomInfo->host()) //방장일 경우에 방 정보도 수정해줘야한다
	{
		roomInfo->set_host(next_pos);
	}
	deleteFrom->DeleteSubrange(prev_pos % BLUEINDEXSTART, 1);
	AdjustClientsIndexes(prev_pos);
	newClient->set_position(next_pos);
	return newClient;
}

void Room::AdjustClientsIndexes(int basePos)
{ 
	int size;
	Mutable_Team team;
	bool needHostDecrement = false;
	int hostPos = roomInfo->host();

	if (basePos < BLUEINDEXSTART)
	{
		if (hostPos < BLUEINDEXSTART && hostPos >= basePos)
			needHostDecrement = true;
		size = roomInfo->redteam_size();
		team = roomInfo->mutable_redteam();
	}
	else
	{
		if (hostPos >= BLUEINDEXSTART && hostPos >= basePos)
			needHostDecrement = true;
		basePos -= BLUEINDEXSTART;
		size = roomInfo->blueteam_size();
		team = roomInfo->mutable_blueteam();
	}

	if (basePos >= size)
		return;

	for (int i = basePos; i < size; i++)
	{
		(*team)[i].set_position(team->Get(i).position() - 1);
	}

	if (needHostDecrement)
		roomInfo->set_host(roomInfo->host() - 1);
}

void Room::ChangeGameroomHost(bool isOnRedteam)
{
	int nextHost;
	if (isOnRedteam) 
	{
		if (roomInfo->blueteam_size() != 0)
			nextHost = BLUEINDEXSTART;
		else
			nextHost = 0;
	}
	else
	{
		if (roomInfo->redteam_size() != 0)
			nextHost = 0;
		else
			nextHost = BLUEINDEXSTART;
	}
	roomInfo->set_host(nextHost);
	Client* nextHostClnt = GetClient(nextHost);
	if (nextHostClnt->ready())
	{
		GetClient(nextHost)->set_ready(false);
		roomInfo->set_readycount(roomInfo->readycount() - 1);
	}
}
