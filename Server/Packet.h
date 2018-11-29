#ifndef __PACKET_H__
#define __PACKET_H__

class Packet {
public:
	int len;
	char* buffer;

	Packet(int size);
	~Packet();
};

typedef Packet PACKET;
typedef Packet* LPPACKET;

#endif
