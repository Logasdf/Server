#include "Packet.h"
#include "def.h"

Packet::TypeMap Packet::typeMap = {
	{typeid(Data), MessageType::DATA},
	{typeid(RoomList), MessageType::ROOMLIST},
	{typeid(Room), MessageType::ROOM},
	{typeid(Client), MessageType::CLIENT}
};

Packet::Packet() 
{
	memset(buffer, 0, MAX_SIZE);
}

Packet::~Packet() 
{}

void Packet::ClearBuffer()
{
	memset(buffer, 0, MAX_SIZE);
}

int Packet::PackMessage(int type, MessageLite* message)
{
	ClearBuffer();
	aos = new ArrayOutputStream(buffer, MAX_SIZE);
	cos = new CodedOutputStream(aos);

	if (message != NULL)
	{
		Serialize(cos, message);
	}
	else 
	{	
		assert(type != -1);
		cos->WriteLittleEndian32(type);
		cos->WriteLittleEndian32(0);
	}

	int rtn = cos->ByteCount();

	delete cos;
	delete aos;
	return rtn;
}

void Packet::UnpackMessage(int& type, int& length, MessageLite*& message)
{
	ais = new ArrayInputStream(buffer, MAX_SIZE);
	cis = new CodedInputStream(ais);

	cis->ReadLittleEndian32((UINT*)&type);
	cis->ReadLittleEndian32((UINT*)&length);
	if(length != 0)
	{
		Deserialize(type, cis, message);
	}

	delete cis;
	delete ais;
	ClearBuffer();
}

void Packet::Serialize(CodedOutputStream*& cos, MessageLite*& message)
{
	int contentType = typeMap[typeid(*message)];
	int contentLength = message->ByteSize();

	cos->WriteLittleEndian32(contentType);
	cos->WriteLittleEndian32(contentLength);
	message->SerializeToCodedStream(cos);
}

void Packet::Deserialize(int& type, CodedInputStream*& cis, MessageLite*& message)
{
	if (type == MessageType::DATA)
	{
		message = new Data();
	}
	else
	{
		LOG("Type is not defined... Check Type!");
		return;
	}
	message->ParseFromCodedStream(cis);
}

Packet* Packet::AllocatePacket()
{
	Packet* lpPacket = new Packet();
	return lpPacket;
}

void Packet::DeallocatePacket(Packet* lpPacket)
{
	assert(lpPacket != NULL);
	delete lpPacket;
}