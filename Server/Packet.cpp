#include "Packet.h"
#include "def.h"

Packet::TypeMap Packet::typeMap = {
	{typeid(RoomList), TYPE_ROOMLIST}
};

Packet::Packet() 
{
	memset(buffer, 0, MAX_SIZE);
}

Packet::~Packet() 
{}

void Packet::ClearBuffer(bool isOut)
{
	memset(buffer, 0, MAX_SIZE);
}

int Packet::PackMessage(int type, MessageLite* message)
{
	ClearBuffer();
	aos = new ArrayOutputStream(buffer, MAX_SIZE);
	cos = new CodedOutputStream(aos);

	char method;
	if (message == NULL)
	{
		method = GET;
		cos->WriteRaw(&method, 1);
		cos->WriteLittleEndian32(type);
	}
	else
	{
		method = POST;
		cos->WriteRaw(&method, 1);
		Serialize(cos, message);
	}

	int rtn = cos->ByteCount();
	delete cos;
	delete aos;
	return rtn;
}

void Packet::UnpackMessage(char& method, int& type, MessageLite*& message)
{
	ais = new ArrayInputStream(buffer, MAX_SIZE);
	cis = new CodedInputStream(ais);

	cis->ReadRaw(&method, 1);
	cis->ReadLittleEndian32((UINT*)&type);
	if (method == GET)
	{
		//LOG("GET Method");
	}
	else if(method == POST)
	{
		//LOG("POST Method");
		int length;
		cis->ReadLittleEndian32((UINT*)&length);
		if (type == TYPE_ROOM)
		{
			message = new Room();
			message->ParseFromCodedStream(cis);
		}
	}

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