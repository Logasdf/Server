#ifndef __SOCKET_INFO_H__
#define __SOCKET_INFO_H__

#include <WinSock2.h>
#include "IOInfo.h"

class SocketInfo {
public:
	SocketInfo();
	~SocketInfo();
	
public:
	static SocketInfo* AllocateSocketInfo(const SOCKET& socket);
	static void DeallocateSocketInfo(SocketInfo* lpSocketInfo);

public:
	SOCKET socket;
	IOInfo* recvBuf;
	IOInfo* sendBuf;
};
#endif
