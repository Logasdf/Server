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
	void ProcessReadyEvent(Client*& client);
	Client* ProcessTeamChangeEvent(Client*& position);
	bool ProcessLeaveGameroomEvent(int position, SocketInfo* lpSocketInfo);

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
	const int BLUEINDEXSTART = 8;
	CRITICAL_SECTION csForClientSockets;
	CRITICAL_SECTION csForRoomInfo;

	// Test for broadcast
	CRITICAL_SECTION csForBroadcast;

	static unsigned __stdcall ThreadMain(void* pVoid);

	Client* GetClient(int position);
	Client* MoveClientToOppositeTeam(Client*& affectedClient, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo);
	void AdjustClientsIndexes(int basePos); // 누가 나가거나, 팀을 바꿨을 때 인덱스 변경되는 클라들 포지션 조정하는 함수
	void ChangeGameroomHost(bool isOnRedteam);
};

