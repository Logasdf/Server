#include <cstdio>
#include <cstdlib>
#include <WinSock2.h>
#include <Windows.h>
#include "IOInfo.h"
#include "SocketInfo.h"
#include "ServerManager.h"
#include "ErrorHandle.h"

unsigned int WINAPI ThreadMain(void*);
void SendMsg(const char* msg, DWORD len);

//for test
int clntCnt = 0;
SOCKET clntSocks[256];

int main(int argc, char* argv[])
{
	ServerManager servManager;
	HANDLE hComPort;

	if (argc != 2) {
		printf("Usage: %s <Port>\n", argv[0]);
		exit(1);
	}

	servManager.InitServer(2, 2);
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	servManager.CreateThread(hComPort, ThreadMain);

	SOCKET_INFO servSockInfo;
	servSockInfo.SocketSetup(atoi(argv[1]));

	SOCKET servSock = servSockInfo.getSocket();
	SOCKADDR_IN servAdr = servSockInfo.getSocketAdr();
	if (bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) != 0) {
		ErrorHandling("bind() error!!");
	}
	if (listen(servSock, 10) != 0) {
		ErrorHandling("listen() error!!");
	}

	LPSOCKET_INFO sockInfo;
	LPIO_INFO ioInfo;
	DWORD recvBytes, flags = 0;
	while (1)
	{
		SOCKET clntSock;
		SOCKADDR_IN clntAdr;
		int adrSz = sizeof(clntAdr);

		clntSock = accept(servSock, (SOCKADDR*)&clntAdr, &adrSz);
		if (clntSock == INVALID_SOCKET) {
			//ErrorHandling("accept() error!");
			printf("%d\n", WSAGetLastError());
			continue;
		}
		clntSocks[clntCnt++] = clntSock;
		sockInfo = new SOCKET_INFO(clntSock, clntAdr);
		CreateIoCompletionPort((HANDLE)clntSock, hComPort, (ULONG_PTR)sockInfo, 0);
		ioInfo = new IO_INFO(1024);

		WSARecv(sockInfo->getSocket(), &(ioInfo->getWsaBuf()),
			1, &recvBytes, &flags, &(ioInfo->getOverlapped()), NULL);
	}

	return 0;
}

unsigned int WINAPI ThreadMain(void* pComPort)
{
	HANDLE hComPort = (HANDLE)pComPort;
	SOCKET sock;
	DWORD bytesTrans;
	LPSOCKET_INFO sockInfo;
	LPIO_INFO ioInfo;
	DWORD flags = 0;
	int i;

	while (1)
	{
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
			(PULONG_PTR)&sockInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		sock = sockInfo->getSocket();

		puts("message received!");
		if (bytesTrans == 0)    // EOF Àü¼Û ½Ã
		{
			for (i = 0; i<clntCnt; i++)   // remove disconnected client
			{
				if (sock == clntSocks[i])
				{
					while (i++<clntCnt - 1)
						clntSocks[i] = clntSocks[i + 1];
					break;
				}
			}
			clntCnt--;

			closesocket(sock);
			free(sockInfo); free(ioInfo);
			continue;
		}


		WSABUF wsaBuf = ioInfo->getWsaBuf();
		SendMsg(wsaBuf.buf, bytesTrans);

		memset(&(ioInfo->getOverlapped()), 0, sizeof(OVERLAPPED));
		WSARecv(sock, &(wsaBuf),
			1, NULL, &flags, &(ioInfo->getOverlapped()), NULL);
	}

	return 0;
}


void SendMsg(const char * msg, DWORD len)   // send to all
{
	int i;
	for (i = 0; i<clntCnt; i++)
		send(clntSocks[i], msg, len, 0);
}