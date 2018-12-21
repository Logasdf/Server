#include "IOInfo.h"
#include <cassert>

IOInfo::IOInfo()
{
	memset(&(overlapped), 0, sizeof(WSAOVERLAPPED));
	wsaBuf.len = 0;
	wsaBuf.buf = NULL;
}

IOInfo::~IOInfo() {}

IOInfo* IOInfo::AllocateIoInfo()
{
	IOInfo* lpIoInfo = new IOInfo();
	lpIoInfo->lpPacket = Packet::AllocatePacket();
	lpIoInfo->wsaBuf.buf = lpIoInfo->lpPacket->buffer;
	lpIoInfo->wsaBuf.len = MAX_SIZE;
	return lpIoInfo;
}

void IOInfo::DeallocateIoInfo(IOInfo* lpIoInfo)
{
	assert(lpIoInfo != NULL);
	if (lpIoInfo->lpPacket != NULL)
		Packet::DeallocatePacket(lpIoInfo->lpPacket);
	free(lpIoInfo);
}