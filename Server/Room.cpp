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
	std::cout << "Ŭ���̾�Ʈ���� �߰� �Ϸ�" << std::endl;
}

void Room::RemoveClientInfo(SocketInfo * lpSocketInfo, string& userName)
{
	clientSockets.remove(lpSocketInfo);
	if (clientMap.find(userName) != clientMap.end()) {
		clientMap.erase(userName);
	}
	std::cout << "Ŭ���̾�Ʈ���� ���� �Ϸ�" << std::endl;
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
		std::cout << "���� �Ϸ��� �ϰ��ֱ���?" << std::endl;
		roomInfo->set_readycount(roomInfo->readycount() + 1);
	} 
	else 
	{
		std::cout << "���� Ǯ���� �ϰ��ֱ���?" << std::endl;
		roomInfo->set_readycount(roomInfo->readycount() - 1);
	}	
	std::cout << "���� ���� �������� ���� �� : " << roomInfo->readycount() << std::endl;
	affectedClient->set_ready(!affectedClient->ready());
}

int Room::ProcessTeamChangeEvent(int position)
{
	//1. ���� ���� ��� ���ΰ��� Ȯ���ϰ� Ÿ ���� �����ϴ� ���ڸ��� �ִ��� Ȯ��
	//   -> ���ڸ��� ������ -1�� ��ȯ.
	//2. Ŭ���̾�Ʈ�� �ڸ��� �ű��, ���� ���� ���� ���� �����ϰ� ������Ų��. -> �ڵ����� ��
	//3. ���ο� �ڸ��� �ε����� ��ȯ�Ѵ�.
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
	Client* affectedClient = GetClient(prev_pos); // position�� 8 �̻��̾ �˾Ƽ� ����. �ٷ� ���� �ִ� �Լ�
	Client* newClient = addTo->Add();
	newClient->CopyFrom(*affectedClient);

	deleteFrom->DeleteSubrange(prev_pos % BLUEINDEXSTART, 1);
	newClient->set_position(next_pos);
}
