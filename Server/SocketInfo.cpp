#include "SocketInfo.h"
#include "ErrorHandle.h"

SocketInfo::SocketInfo() {}

SocketInfo::SocketInfo(const SOCKET& sock, const SOCKADDR_IN& addr)
{
	this->socket = sock;
	memcpy(&(this->sockAdr), &addr, sizeof(this->sockAdr));
}

SocketInfo::~SocketInfo()
{
	closesocket(socket);
}

void SocketInfo::SocketSetup(int port)
{
	socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockAdr.sin_family = AF_INET;
	sockAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAdr.sin_port = htons(port);
}

const SOCKET& SocketInfo::getSocket()
{
	return this->socket;
}

const SOCKADDR_IN& SocketInfo::getSocketAdr()
{
	return this->sockAdr;
}