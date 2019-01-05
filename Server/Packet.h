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
#include "data.pb.h"

using namespace packet;
using namespace google::protobuf;
using namespace google::protobuf::io;

#define MAX_SIZE 4096 // protocol buffer encode/decode maximum size at once time

class Packet {
public:
	Packet();
	~Packet();

	void ClearBuffer();
	int PackMessage(int type = -1, MessageLite* message = NULL);
	void UnpackMessage(int& type, int& length, MessageLite*& message);

public:
	static Packet* AllocatePacket();
	static void DeallocatePacket(Packet* lpPacket);

public:
	char buffer[MAX_SIZE]; // header and body

private:
	ArrayInputStream* ais;
	ArrayOutputStream* aos;
	CodedInputStream* cis;
	CodedOutputStream* cos;

private:
	typedef std::unordered_map<std::type_index, int> TypeMap;
	static TypeMap typeMap;

private:
	void Serialize(CodedOutputStream*&, MessageLite*&);
	void Deserialize(int&, CodedInputStream*&, MessageLite*&);
};