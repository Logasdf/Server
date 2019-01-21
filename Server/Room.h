#pragma once
#include "SocketInfo.h"
#include "protobuf/room.pb.h"
#include "protobuf/PlayState.pb.h"
#include "forward_list"
typedef google::protobuf::RepeatedPtrField<packet::Client>* Mutable_Team;

class Room
{
public:
	Room(RoomInfo* initVal);
	~Room();

	void AddClientInfo(SocketInfo* lpSocketInfo, string& userName);
	void RemoveClientInfo(SocketInfo* lpSocketInfo, string& userName);
	std::forward_list<SocketInfo*>::const_iterator ClientSocketsBegin();
	std::forward_list<SocketInfo*>::const_iterator ClientSocketsEnd();
	void ProcessReadyEvent(Client*& affectedClient);
	Client* ProcessTeamChangeEvent(Client*& affectedClient);
	bool ProcessLeaveGameroomEvent(Client*& affectedClient, SocketInfo* lpSocketInfo);
	//test�� �������� �� ���ٺҰ� �κ�
	void SetGameStartFlag(bool to);
	bool HasGameStarted() const;

	SocketInfo*& GetSocketUsingName(string& userName);

	// Test for broadcast
	void InitCompletionPort(int maxNumberOfThreads = 1);
	void CreateThreadPool(int numOfThreads = 1);

public :
	// Test for broadcast
	HANDLE hCompPort;

private:
	RoomInfo* roomInfo;
	std::forward_list<SocketInfo*> clientSockets;
	std::unordered_map<std::string, SocketInfo*> clientMap; // <Client_Name, Client_Socket>
	CRITICAL_SECTION csForClientSockets;
	CRITICAL_SECTION csForRoomInfo;

	const int BLUEINDEXSTART = 8;
	bool gameStarted;
	// Test for broadcast
	CRITICAL_SECTION csForBroadcast;

	static unsigned __stdcall ThreadMain(void* pVoid);

	Client* GetClient(int position);
	Client* MoveClientToOppositeTeam(Client*& affectedClient, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo);
	void AdjustClientsIndexes(int basePos); // ���� �����ų�, ���� �ٲ��� �� �ε��� ����Ǵ� Ŭ��� ������ �����ϴ� �Լ�
	void ChangeGameroomHost(bool isOnRedteam);
};

