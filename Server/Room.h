#pragma once
#include "SocketInfo.h"
#include "protobuf/room.pb.h"
#include "protobuf/PlayState.pb.h"
#include "forward_list"
typedef google::protobuf::RepeatedPtrField<packet::Client>* Mutable_Team;
typedef std::forward_list<SocketInfo*>::const_iterator SocketIterator;
class ServerManager;

class Room
{
public:
	Room(RoomInfo* initVal);
	~Room();

	void AddClientInfo(SocketInfo* lpSocketInfo, string& userName);
	void RemoveClientInfo(SocketInfo* lpSocketInfo, string& userName);

	std::forward_list<SocketInfo*>::const_iterator ClientSocketsBegin();
	std::forward_list<SocketInfo*>::const_iterator ClientSocketsEnd();
	void InsertDataIntoBroadcastQueue(DWORD, ULONG_PTR);

	void ProcessReadyEvent(Client*& affectedClient);
	Client* ProcessTeamChangeEvent(Client*& affectedClient);
	bool ProcessLeaveGameroomEvent(Client*& affectedClient, SocketInfo* lpSocketInfo);

	bool CanStart(string& errorMessage);
	bool HasGameStarted() const;
	void SetGameStartFlag(bool to);
	
	SocketInfo*& GetSocketUsingName(string& userName);

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
	// Test for broadcast
	CRITICAL_SECTION csForBroadcast;

	const int BLUEINDEXSTART = 8;
	bool gameStarted;

	static unsigned __stdcall ThreadMain(void* pVoid);

	Client* GetClient(int position);
	Client* MoveClientToOppositeTeam(Client*& affectedClient, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo);
	void AdjustClientsIndexes(int basePos); // 누가 나가거나, 팀을 바꿨을 때 인덱스 변경되는 클라들 포지션 조정하는 함수
	void ChangeGameroomHost(bool isOnRedteam);
};

