#pragma once

#include <google/protobuf/util/time_util.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <cassert>
#include <cstring>

#include "ErrorHandle.h"
#include "protobuf/room.pb.h"

using namespace packet;
using namespace google::protobuf::io;

#define MAX_SIZE 4096 // protocol buffer encode/decode maximum size at once time

enum {TYPE_ROOMLIST, }; // temporary type declaration

class Packet {
public:
	Packet();
	~Packet();

	void ClearBuffer(bool isOut = true);
	template<typename T> int Serialize(T& message);

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
};

template<typename T>
inline int Packet::Serialize(T& message)
{
	ClearBuffer();
	int type = typeMap[typeid(T)];
	int contentLength = message.ByteSize();
	cos->WriteLittleEndian32(type);
	cos->WriteLittleEndian32(contentLength);
	message.SerializeToCodedStream(cos);

	return cos->ByteCount();


	//if (type == TYPE_ROOMLIST) {
	//	RoomList& ref = dynamic_cast<RoomList&>(message);
	//	int contentLength = ref.ByteSize();
	//	cos->WriteLittleEndian32(type);
	//	cos->WriteLittleEndian32(contentLength);
	//	ref.SerializeToCodedStream(cos);
	//}
}