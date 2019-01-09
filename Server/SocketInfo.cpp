#include "SocketInfo.h"
#include "ErrorHandle.h"
#include <cassert>

SocketInfo::SocketInfo() 
{
	socket = INVALID_SOCKET;
	recvBuf = NULL;
	sendBuf = NULL;
}

SocketInfo::~SocketInfo() 
{
}

void SocketInfo::GetIpAndPort(char pIpAddress[], int & port)
{
	/* 필요가 없어진듯?
	SOCKADDR_IN addr;
	int addrSize = sizeof(addr);
	ZeroMemory(&addr, addrSize);
	getpeername(socket, (SOCKADDR*)&addr, &addrSize);
	//inet_ntop(AF_INET, &addr, pIpAddress, sizeof(pIpAddress));
	port = ntohs(addr.sin_port);
	CopyMemory(pIpAddress, inet_ntoa(addr.sin_addr), 20);
	printf("%s:%d\n", inet_ntoa(addr.sin_addr), port);
	*/
}

SocketInfo* SocketInfo::AllocateSocketInfo(const SOCKET& socket)
{
	SocketInfo* lpSocketInfo = new SocketInfo();
	assert(lpSocketInfo != NULL);
	lpSocketInfo->socket = socket;
	lpSocketInfo->recvBuf = IOInfo::AllocateIoInfo();
	lpSocketInfo->sendBuf = IOInfo::AllocateIoInfo();

	return lpSocketInfo;
}

void SocketInfo::DeallocateSocketInfo(SocketInfo* lpSocketInfo)
{
	assert(lpSocketInfo != NULL);
	if (lpSocketInfo->recvBuf != NULL) 
		IOInfo::DeallocateIoInfo(lpSocketInfo->recvBuf);
	if (lpSocketInfo->sendBuf != NULL)
		IOInfo::DeallocateIoInfo(lpSocketInfo->sendBuf);
	lpSocketInfo->socket = INVALID_SOCKET;
	delete lpSocketInfo;
}
