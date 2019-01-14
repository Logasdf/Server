#include "Room.h"

Room::~Room()
{
	std::cout << "Room 객체가 삭제되었습니다." << std::endl;
}

void Room::AddClientInfo(SocketInfo * lpSocketInfo, string& userName)
{
	clientSockets.push_front(lpSocketInfo);
	clientMap[userName] = lpSocketInfo;
	std::cout << "클라이언트정보 추가 완료" << std::endl;
}

void Room::RemoveClientInfo(SocketInfo * lpSocketInfo, string& userName)
{
	clientSockets.remove(lpSocketInfo);
	if (clientMap.find(userName) != clientMap.end()) {
		clientMap.erase(userName);
	}
	std::cout << "클라이언트정보 삭제 완료" << std::endl;
}

std::forward_list<SocketInfo*>::const_iterator Room::ClientSocketsBegin()
{
	return clientSockets.cbegin();
}

std::forward_list<SocketInfo*>::const_iterator Room::ClientSocketsEnd()
{
	return clientSockets.cend();
}

void Room::ProcessReadyEvent(int position)
{
	Client* affectedClient = GetClient(position);
	bool toReady = !affectedClient->ready() ? true : false;
	if (toReady) 
	{
		roomInfo->set_readycount(roomInfo->readycount() + 1);
	} 
	else 
	{
		roomInfo->set_readycount(roomInfo->readycount() - 1);
	}	
	std::cout << "현재 방의 레디중인 유저 수 : " << roomInfo->readycount() << std::endl;
	affectedClient->set_ready(!affectedClient->ready());
}

int Room::ProcessTeamChangeEvent(int position)
{
	//1. 현재 팀이 어느 팀인가를 확인하고 타 팀에 존재하는 빈자리가 있는지 확인
	//   -> 빈자리가 없으면 -1을 반환.
	//2. 클라이언트의 자리를 옮기고, 각각 팀의 유저 수를 적절하게 증감시킨다. -> 자동으로 함
	//3. 새로운 자리의 인덱스를 반환한다.
	//방장인 경우에는 host위치를 변경해주어야함 ** 빼먹었다가 추가함
	bool isOnRedTeam = position < BLUEINDEXSTART ? true : false;
	
	int maxuser = roomInfo->limit() / 2;
	int nextIdx;
	if (isOnRedTeam) 
	{
		nextIdx = roomInfo->blueteam_size();
		if (nextIdx == maxuser)
			return -1;
		
		nextIdx += BLUEINDEXSTART;
		MoveClientToOppositeTeam(position, nextIdx, roomInfo->mutable_redteam(), roomInfo->mutable_blueteam());
	}
	else 
	{
		nextIdx = roomInfo->redteam_size();
		if (nextIdx == maxuser)
			return -1;

		MoveClientToOppositeTeam(position, nextIdx, roomInfo->mutable_blueteam(), roomInfo->mutable_redteam());
	}

	return nextIdx;
}


bool Room::ProcessLeaveGameroomEvent(int position, SocketInfo* lpSocketInfo, bool& hostChanged) // ret T - if the room obj needs to be destroyed, F - otherwise.
{
	bool isOnRedTeam = position < BLUEINDEXSTART ? true : false;
	bool isClosed = false;

	if (isOnRedTeam) // 클라이언트 객체를 리스트에서 삭제한다 ( 해제까지 해줌 )
		roomInfo->mutable_redteam()->DeleteSubrange(position, 1);
	else
		roomInfo->mutable_blueteam()->DeleteSubrange(position % BLUEINDEXSTART, 1);

	clientSockets.remove(lpSocketInfo); //클라이언트 소켓을 forward_list에서 제거. 게임종료가 아니라서 해제는 하면 안 됨.

	if (roomInfo->current() == 1)
	{
		isClosed = true; // 혼자있는 상황이라 방 깨지는 경우
	}
	else
	{
		if (roomInfo->host() == position)
		{ //2 - 방에 혼자가 아닌데, 방장인 경우
			hostChanged = true;
			ChangeGameroomHost(isOnRedTeam); //방장 변경
		}
	}
	roomInfo->set_current(roomInfo->current() - 1); //인원수 줄이기
	return isClosed;
}

SocketInfo*& Room::GetSocketUsingName(string & userName)
{
	return clientMap[userName];
}

Client* Room::GetClient(int position) 
{
	return position < BLUEINDEXSTART ? 
		roomInfo->mutable_redteam(position) : roomInfo->mutable_blueteam(position % BLUEINDEXSTART);
	
}

void Room::MoveClientToOppositeTeam(int prev_pos, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo)
{
	Client* affectedClient = GetClient(prev_pos); // position이 8 이상이어도 알아서 계산됨. 바로 위에 있는 함수
	Client* newClient = addTo->Add();
	newClient->CopyFrom(*affectedClient);

	deleteFrom->DeleteSubrange(prev_pos % BLUEINDEXSTART, 1);
	newClient->set_position(next_pos);

	if (prev_pos == roomInfo->host()) //방장일 경우에 방 정보도 수정해줘야한다
		roomInfo->set_host(next_pos);
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
}
