#ifndef __SOCKET_INFO_H__
#define __SOCKET_INFO_H__

#include <WinSock2.h>

class SocketInfo {
private:
	SOCKET socket;
	SOCKADDR_IN sockAdr;
public :
	SocketInfo();
	SocketInfo(const SOCKET& sock, const SOCKADDR_IN& sockAdr);
	~SocketInfo();
	
	void SocketSetup(int port);
	const SOCKET& getSocket();
	const SOCKADDR_IN& getSocketAdr();
};

typedef SocketInfo SOCKET_INFO;
typedef SocketInfo* LPSOCKET_INFO;

#endif
