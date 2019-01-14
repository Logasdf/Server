#pragma once

#include <google/protobuf/util/time_util.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/message_lite.h>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <cassert>
#include <cstring>

#include "ErrorHandle.h"
#include "protobuf/room.pb.h"
#include "protobuf/PlayState.pb.h"
#include "protobuf/data.pb.h"

using namespace packet;
using namespace state;
using namespace google::protobuf;
using namespace google::protobuf::io;

#define FOR_IO_SIZE 2048
#define FOR_BAKCUP_SIZE 2048
#define FOR_PACK_SIZE 4096
#define MAX_SIZE 2048

class Packet {
public:
	Packet();
	~Packet();

	void ClearBuffer();

	int PackMessage(int type = -1, MessageLite* message = nullptr);
	void UnpackMessage(int& type, int& length, MessageLite*& message);
	//bool UnpackMessage(DWORD& bytesTransferred, int& type, int& length, MessageLite*& message);

public:
	static Packet* AllocatePacket();
	static void DeallocatePacket(Packet* lpPacket);

public:
	char buffer[MAX_SIZE];
	/*char buffer[FOR_IO_SIZE];
	char backup[FOR_BAKCUP_SIZE];
	char pack[FOR_PACK_SIZE];

	int backupBufLength;
	int packBufLength;
	int packBufOffset;*/

private:
	ArrayInputStream* ais;
	ArrayOutputStream* aos;
	CodedInputStream* cis;
	CodedOutputStream* cos;

	HANDLE hMutexObj;

private:
	typedef std::unordered_map<std::type_index, int> TypeMap;
	typedef std::unordered_map<int, std::type_index> InvTypeMap;
	static TypeMap typeMap;
	static InvTypeMap invTypeMap;

private:
	bool GetTypeAndLength(int& type, int& length);
	bool CheckValidType(int& type);

	void Serialize(CodedOutputStream*&, MessageLite*&);
	void Deserialize(int&, CodedInputStream*&, MessageLite*&);
};