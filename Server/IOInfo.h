#ifndef __IO_INFO_H__
#define __IO_INFO_H__

#include "Packet.h"
#include <WinSock2.h>
#include <Windows.h>

class IOInfo {
private:
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	LPPACKET pPacket;
public:
	IOInfo();
	IOInfo(int);
	~IOInfo();
	OVERLAPPED& getOverlapped();
	WSABUF& getWsaBuf();
};

typedef IOInfo IO_INFO;
typedef IOInfo* LPIO_INFO;

#endif