#include "Packet.h"

Packet::TypeMap Packet::typeMap = {
	{typeid(RoomList), TYPE_ROOMLIST}
};

Packet::Packet() 
{
	memset(buffer, 0, MAX_SIZE);

	aos = new ArrayOutputStream(buffer, MAX_SIZE);
	cos = new CodedOutputStream(aos);
	//ais = new ArrayInputStream(inBuffer, MAX_SIZE);
	//aos = new ArrayOutputStream(outBuffer, MAX_SIZE);
	//cis = new CodedInputStream(ais);
	//cos = new CodedOutputStream(aos);
}

Packet::~Packet() 
{
	delete cos;
	delete aos;
}

void Packet::ClearBuffer(bool isOut)
{
	memset(buffer, 0, MAX_SIZE);
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