#include "Room.h"

Room::~Room()
{
	if(roomInfo != nullptr)
		delete roomInfo;
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
		std::cout << "레디를 하려고 하고있군요?" << std::endl;
		roomInfo->set_readycount(roomInfo->readycount() + 1);
	} 
	else 
	{
		std::cout << "레디를 풀려고 하고있군요?" << std::endl;
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
}
