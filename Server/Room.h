#pragma once
#include "SocketInfo.h"
#include "protobuf/room.pb.h"
#include "forward_list"
typedef google::protobuf::RepeatedPtrField<packet::Client>* Mutable_Team;

class Room
{
public:
	Room(RoomInfo* initVal) : roomInfo(initVal) {};
	~Room();

	void AddClientInfo(SocketInfo* lpSocketInfo, string& userName);
	void RemoveClientInfo(SocketInfo* lpSocketInfo, string& userName);
	std::forward_list<SocketInfo*>::const_iterator ClientSocketsBegin();
	std::forward_list<SocketInfo*>::const_iterator ClientSocketsEnd();
	void ProcessReadyEvent(int position);
	int ProcessTeamChangeEvent(int position);
	bool ProcessLeaveGameroomEvent(int position, SocketInfo* lpSocketInfo, bool& hostChanged);

	SocketInfo*& GetSocketUsingName(string& userName);

private:
	RoomInfo* roomInfo;
	std::forward_list<SocketInfo*> clientSockets;
	std::unordered_map<std::string, SocketInfo*> clientMap; // <Client_Name, Client_Socket>
	const int BLUEINDEXSTART = 8;

	Client* GetClient(int position);
	void MoveClientToOppositeTeam(int prev_pos, int next_pos, Mutable_Team deleteFrom, Mutable_Team addTo);
	void ChangeGameroomHost(bool isOnRedteam);
};

