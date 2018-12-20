#ifndef __PACKET_H__
#define __PACKET_H__

#include <string>

#define MAX_SIZE 4096

class Packet {
public:
	Packet();
	~Packet();

	void ClearBuffer();
	void Serialize(std::string& stream);

public:
	static Packet* AllocatePacket();
	static void DeallocatePacket(Packet* lpPacket);

public:
	char buffer[MAX_SIZE];
};

#endif
