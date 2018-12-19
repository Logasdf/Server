#include <cstdio>
#include <cstdlib>
#include <process.h>
#include "ServerManager.h"
#include "ErrorHandle.h"

#define LOG(s) printf("[Log]: %s\n", s);

unsigned __stdcall ThreadMain(void* pVoid) 
{
	
	return 0;
}

ServerManager::ServerManager() 
{ 
	hCompPort = NULL; 
	servSock = NULL;
}

ServerManager::~ServerManager() 
{ 
	if (hCompPort != NULL)
		CloseHandle(hCompPort);
	if (servSock != NULL)
		closesocket(servSock);

	WSACleanup();
}

void ServerManager::Start(int port)
{
	InitSocket(port);
	InitCompletionPort();
	CreateThreadPool();
	AcceptClient();
}

void ServerManager::Stop() 
{
	ShutdownThreads();
	CloseHandle(hCompPort);
	closesocket(servSock);

	hCompPort = NULL;
	servSock = NULL;
}

void ServerManager::InitSocket(int port, int prime, int sub)
{
	if (WSAStartup(MAKEWORD(prime, sub), &wsaData) != 0) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	servSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (servSock == INVALID_SOCKET) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(port);

	if (bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling(WSAGetLastError());
		closesocket(servSock);
		servSock = NULL;
		return;
	}

	if (listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
		ErrorHandling(WSAGetLastError());
		closesocket(servSock);
		servSock = NULL;
		return;
	}

	LOG("Server Socket Initiation Success!!");
}

void ServerManager::InitCompletionPort(int maxNumberOfThreads)
{
	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxNumberOfThreads);
	if (hCompPort == NULL) {
		ErrorHandling(WSAGetLastError());
		return;
	}

	LOG("IOCP Kernel Object Created!!");
}

void ServerManager::AcceptClient()
{
	SOCKET clntSock;
	SOCKADDR_IN clntAdr;
	int clntAdrSz = sizeof(SOCKADDR_IN);

	LOG("Accept Process Start");

	while (true)
	{
		__try
		{
			clntSock = accept(servSock, (SOCKADDR*)&clntAdr, &clntAdrSz);
			if (clntSock == INVALID_SOCKET)
			{
				int errCode = WSAGetLastError();
				if (errCode == WSAEINTR)
				{
					return;
				}
				ErrorHandling(errCode);
				__leave;
			}

			LOG("Client (%s::%d) connected\n", inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

			int zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError());
				continue;
			}
			zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError());
				continue;
			}

			// socket context 생성

			// IOCP와 clnt socket 연결

			// 초기 Recv 요청
		}
		__finally
		{
			if (AbnormalTermination())
			{
				// CloseClient
			}
		}
	}
}

void ServerManager::CloseClient()
{

}

void ServerManager::CreateThreadPool(int numOfThreads)
{

}

void ServerManager::ShutdownThreads()
{

}


