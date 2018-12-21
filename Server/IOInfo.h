#ifndef __IO_INFO_H__
#define __IO_INFO_H__

#include "Packet.h"
#include <WinSock2.h>
#include <Windows.h>

class IOInfo {
public:
	IOInfo();
	~IOInfo();

public:
	static IOInfo* AllocateIoInfo();
	static void DeallocateIoInfo(IOInfo* lpIoInfo);

public:
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	Packet* lpPacket;
};

#endif