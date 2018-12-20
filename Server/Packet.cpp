#include "Packet.h"
#include <cassert>
#include <cstring>

Packet::Packet() {}

Packet::~Packet() {}

void Packet::ClearBuffer()
{
	memset(buffer, 0, MAX_SIZE);
}

void Packet::Serialize(std::string& stream)
{
	//this->buffer = stream;
}

Packet* Packet::AllocatePacket()
{
	Packet* lpPacket = new Packet();
	return lpPacket;
}

void Packet::DeallocatePacket(Packet* lpPacket)
{
	assert(lpPacket != NULL);
	free(lpPacket);
}