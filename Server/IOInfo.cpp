#include "IOInfo.h"

IOInfo::IOInfo()
{
	memset(&(overlapped), 0, sizeof(OVERLAPPED));
	wsaBuf.len = pPacket->len;
	wsaBuf.buf = pPacket->buffer;
}

IOInfo::IOInfo(int size)
{
	pPacket = new Packet(size);
	memset(&(overlapped), 0, sizeof(OVERLAPPED));
	wsaBuf.len = pPacket->len;
	wsaBuf.buf = pPacket->buffer;
}

IOInfo::~IOInfo()
{

}

OVERLAPPED& IOInfo::getOverlapped() {
	return this->overlapped;
}

WSABUF& IOInfo::getWsaBuf() {
	return this->wsaBuf;
}