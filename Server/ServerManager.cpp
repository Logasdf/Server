#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <process.h>
#include "ServerManager.h"
#include "ErrorHandle.h"
#include "def.h"

ServerManager::ServerManager() 
{ 
	hCompPort = NULL; 
	servSock = INVALID_SOCKET;
	hMutexObj = CreateMutex(NULL, FALSE, NULL);
}

ServerManager::~ServerManager() 
{ 
	if (hCompPort != NULL)
		CloseHandle(hCompPort);
	if (servSock != INVALID_SOCKET)
		closesocket(servSock);
	if (hMutexObj != NULL)
		CloseHandle(hMutexObj);

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
	CloseHandle(hMutexObj);

	hCompPort = NULL;
	servSock = INVALID_SOCKET;
	hMutexObj = NULL;
}

unsigned __stdcall ServerManager::ThreadMain(void * pVoid)
{
	ServerManager* self = (ServerManager*)pVoid;
	SocketInfo* lpSocketInfo;
	IOInfo* lpIOInfo;
	DWORD dwBytesTransferred = 0;

	while (true)
	{
		bool rtn = GetQueuedCompletionStatus(self->hCompPort, &dwBytesTransferred,
			reinterpret_cast<ULONG_PTR*>(&lpSocketInfo), reinterpret_cast<LPOVERLAPPED*>(&lpIOInfo), INFINITE);
		if (rtn)
		{
			if (((DWORD)lpSocketInfo) == KILL_THREAD) break;

			if (lpIOInfo == NULL) {
				ErrorHandling("Getting IO Information Failed...", WSAGetLastError(), false);
				continue;
			}
		}
		else
		{
			if (lpIOInfo == NULL) {
				ErrorHandling("Getting IO Information Failed...", WSAGetLastError(), false);
			}
			else
			{
				ErrorHandling("Client Connection Close, Socket will close", false);
				self->CloseClient(lpSocketInfo);
			}
			continue;
		}

		try
		{
			if (dwBytesTransferred == 0)
			{
				LOG("Client %d Connection Closed...");
				throw "[Cause]: dwBytesTransferr == 0";
			}

			if (lpIOInfo == lpSocketInfo->recvBuf)
			{
				if (!(self->HandleRecvEvent(lpSocketInfo, dwBytesTransferred)))
				{
					throw "[Cause]: RecvEvent Handling Error!!";
				}
			}
			else if (lpIOInfo == lpSocketInfo->sendBuf)
			{
				if (!(self->HandleSendEvent(lpSocketInfo, dwBytesTransferred)))
				{
					throw "[Cause]: SendEvent Handling Error!!";
				}
			}
			else 
			{
				throw "[Cause]: UnknownEvent Exception...";
			}
		}
		catch (const char* msg)
		{
			ErrorHandling(msg, WSAGetLastError(), false);
			self->CloseClient(lpSocketInfo);
		}
	}

	return 0;
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
	SocketInfo* lpSocketInfo;
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
				ErrorHandling(errCode, false);
				__leave;
			}

			printf("Client (%s::%d) connected\n", inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

			int zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_RCVBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}
			zero = 0;
			if (setsockopt(clntSock, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(int)) == SOCKET_ERROR)
			{
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}

			// socket context 생성(Completion Key로 넘김)
			lpSocketInfo = SocketInfo::AllocateSocketInfo(clntSock);
			if (lpSocketInfo == NULL)
			{
				ErrorHandling("Socket Info Object Allocation Failed...", false);
				continue;
			}

			// IOCP와 clnt socket 연결
			if (CreateIoCompletionPort((HANDLE)clntSock, hCompPort, (ULONG_PTR)lpSocketInfo, 0)
				== NULL) {
				ErrorHandling(WSAGetLastError(), false);
				continue;
			}

			// 초기 Recv 요청
			DWORD dwFlags = 0;
			DWORD dwRecvBytes = 0;
			ZeroMemory(&lpSocketInfo->recvBuf->overlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(lpSocketInfo->recvBuf->buffer, MAX_SIZE);
			if (WSARecv(lpSocketInfo->socket, &(lpSocketInfo->recvBuf->wsaBuf), 1,
				&dwRecvBytes, &dwFlags, &(lpSocketInfo->recvBuf->overlapped), NULL)
				== SOCKET_ERROR)
			{
				int errCode = WSAGetLastError();
				if (errCode != WSA_IO_PENDING) {
					ErrorHandling("Init Recv Operation Error!!", errCode, false);
					continue;
				}
			}
		}
		__finally
		{
			if (AbnormalTermination())
			{
				// CloseClient
				CloseClient(lpSocketInfo);
			}
		}
	}
}

void ServerManager::CloseClient(SocketInfo* lpSocketInfo, bool graceful)
{
	WaitForSingleObject(hMutexObj, INFINITE);
	if (lpSocketInfo != NULL && lpSocketInfo->socket != INVALID_SOCKET)
	{
		if (!graceful)
		{
			LINGER LingerStruct;
			LingerStruct.l_onoff = 1;
			LingerStruct.l_linger = 0;
			assert(SOCKET_ERROR 
				!= setsockopt(lpSocketInfo->socket, SOL_SOCKET, SO_LINGER, 
				(char*)&LingerStruct, sizeof(LingerStruct)));
		}
		closesocket(lpSocketInfo->socket);
		SocketInfo::DeallocateSocketInfo(lpSocketInfo);
		lpSocketInfo = NULL;
	}
	ReleaseMutex(hMutexObj);
}

void ServerManager::CreateThreadPool(int numOfThreads)
{
	if (numOfThreads == 0)
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		threadPoolSize = sysinfo.dwNumberOfProcessors * 2 + 2;
	}
	else
	{
		threadPoolSize = numOfThreads;
	}

	for (int i = 0; i < threadPoolSize; ++i)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = BEGINTHREADEX(NULL, 0, ServerManager::ThreadMain, this, 0, &dwThreadId);
		CloseHandle(hThread);
	}
}

void ServerManager::ShutdownThreads()
{
	for (int i = 0; i < threadPoolSize; ++i)
	{
		PostQueuedCompletionStatus(hCompPort, 0, KILL_THREAD, NULL);
	}
}

bool ServerManager::SendPacket(SocketInfo * lpSocketInfo)
{
	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&lpSocketInfo->sendBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSASend(lpSocketInfo->socket, &(lpSocketInfo->sendBuf->wsaBuf), 1,
		&dwSendBytes, dwFlags, &(lpSocketInfo->sendBuf->overlapped), NULL);

	if (rtn == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("WSASend Failed...", errCode, false);
			return false;
		}
	}

	return true;
}

bool ServerManager::RecvPacket(SocketInfo * lpSocketInfo)
{
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;

	ZeroMemory(&lpSocketInfo->recvBuf->overlapped, sizeof(WSAOVERLAPPED));
	int rtn = WSARecv(lpSocketInfo->socket, &(lpSocketInfo->recvBuf->wsaBuf), 1,
		&dwRecvBytes, &dwFlags, &(lpSocketInfo->recvBuf->overlapped), NULL);

	if (rtn == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			ErrorHandling("WSARecv Failed...", errCode, false);
			return false;
		}
	}

	return true;
}

bool ServerManager::HandleSendEvent(SocketInfo * lpSocketInfo, DWORD dwBytesTransferred)
{

	return true;
}

bool ServerManager::HandleRecvEvent(SocketInfo * lpSocketInfo, DWORD dwBytesTransferred)
{
	//ZeroMemory(lpSocketInfo->sendBuf->buffer, MAX_SIZE);
	//lpSocketInfo->sendBuf->lpPacket->ClearBuffer();
	//CopyMemory(lpSocketInfo->sendBuf->buffer,
	//	lpSocketInfo->recvBuf->buffer, dwBytesTransferred);
	//lpSocketInfo->sendBuf->wsaBuf.len = dwBytesTransferred;
	//lpSocketInfo->recvBuf->lpPacket->ClearBuffer();
	//ZeroMemory(lpSocketInfo->recvBuf->buffer, MAX_SIZE);

	if (!SendPacket(lpSocketInfo))
		return false;

	if (!RecvPacket(lpSocketInfo))
		return false;

	return true;
}


