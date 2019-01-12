#include "Room.h"

Room::~Room()
{
	std::cout << "Room ��ü�� �����Ǿ����ϴ�." << std::endl;
}

void Room::AddClientInfo(SocketInfo * lpSocketInfo)
{
	clientSockets.push_front(lpSocketInfo);
	std::cout << "Ŭ���̾�Ʈ���� �߰� �Ϸ�" << std::endl;
}

void Room::RemoveClientInfo(SocketInfo * lpSocketInfo)
{
	clientSockets.remove(lpSocketInfo);
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
		roomInfo->set_readycount(roomInfo->readycount() + 1);
	} 
	else 
	{
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
	//������ ��쿡�� host��ġ�� �������־���� ** ���Ծ��ٰ� �߰���
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

	if (isOnRedTeam) // Ŭ���̾�Ʈ ��ü�� ����Ʈ���� �����Ѵ� ( �������� ���� )
		roomInfo->mutable_redteam()->DeleteSubrange(position, 1);
	else
		roomInfo->mutable_blueteam()->DeleteSubrange(position % BLUEINDEXSTART, 1);

	clientSockets.remove(lpSocketInfo); //Ŭ���̾�Ʈ ������ forward_list���� ����. �������ᰡ �ƴ϶� ������ �ϸ� �� ��.

	if (roomInfo->current() == 1)
	{
		isClosed = true; // ȥ���ִ� ��Ȳ�̶� �� ������ ���
	}
	else
	{
		if (roomInfo->host() == position)
		{ //2 - �濡 ȥ�ڰ� �ƴѵ�, ������ ���
			hostChanged = true;
			ChangeGameroomHost(isOnRedTeam); //���� ����
		}
	}
	roomInfo->set_current(roomInfo->current() - 1); //�ο��� ���̱�
	return isClosed;
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

	if (prev_pos == roomInfo->host()) //������ ��쿡 �� ������ ����������Ѵ�
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