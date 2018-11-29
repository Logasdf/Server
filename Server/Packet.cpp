#include "Packet.h"
#include <cstdlib>

Packet::Packet(int size)
{
	len = size;
	buffer = (char*)malloc(size);
}

Packet::~Packet()
{
	free(buffer);
}