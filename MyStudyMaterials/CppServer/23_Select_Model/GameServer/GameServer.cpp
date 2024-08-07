﻿#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void HandleError(const char* cause)
{
	int32 errCode = ::WSAGetLastError();
	cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
	int32 sendBytes = 0;
};

int main()
{
	// 윈속 사용 준비
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// listening socket 생성
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;
	// 논블럭 소켓으로 변경
	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;
	// 서버 주소 입력
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);
	// listening socket에 주소 바인딩
	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;

	// Select 모델 = (select 함수가 핵심이 되는)
	// 소켓 함수 호출이 성공할 시점을 미리 알 수 있다!
	// 문제 상황)
	// 수신버퍼에 데이터가 없는데, read 한다거나!
	// 송신버퍼가 꽉 찼는데, write 한다거나!
	// - 블로킹 소켓 : 조건이 만족되지 않아서 블로킹되는 상황 예방
	// - 논블로킹 소켓 : 조건이 만족되지 않아서 불필요하게 반복 체크하는 상황을 예방

	// socket set
	// 1) 읽기[ 2 ] 쓰기[ ] 예외(OOB)[ ] 관찰 대상 등록
	// OutOfBand는 send() 마지막 인자 MSG_OOB로 보내는 특별한 데이터
	// 받는 쪽에서도 recv OOB 세팅을 해야 읽을 수 있음
	// 2) select(readSet, writeSet, exceptSet); -> 관찰 시작
	// 3) 적어도 하나의 소켓이 준비되면 리턴 -> 낙오자는 알아서 제거됨
	// 4) 남은 소켓 체크해서 진행

	// fd_set set;
	// FD_ZERO : 비운다
	// ex) FD_ZERO(set);
	// FD_SET : 소켓 s를 넣는다
	// ex) FD_SET(s, &set);
	// FD_CLR : 소켓 s를 제거
	// ex) FD_CLR(s, &set);
	// FD_ISSET : 소켓 s가 set에 들어있으면 0이 아닌 값을 리턴한다

	// FD_SETSIZE는 64라서 하나의 fd_set에는 소켓을 최대 64개 밖에 등록하지 못한다.

	
	// 사용할 소켓을 컨테이너에 담기(풀링)
	vector<Session> sessions;
	sessions.reserve(100);
	// fd_set 객체 생성
	fd_set reads;
	fd_set writes;

	while (true)
	{
		// fd_set 초기화(매 select 호출 후 초기화 & 재등록 필요)
		FD_ZERO(&reads);
		FD_ZERO(&writes);

		// ListenSocket을 fd_set에 등록
		FD_SET(listenSocket, &reads);

		// 클라이언트용 소켓을 fd_set에 등록
		for (Session& s : sessions)
		{//에코 서버이므로 받은 데이터의 양이 더 적거나 같다면 받을 차례
			if (s.recvBytes <= s.sendBytes)
				FD_SET(s.socket, &reads);
			else//아니면 보낼 차례
				FD_SET(s.socket, &writes);
		}

		/* [ select 함수 옵션 ]
			첫번째 - 리눅스와 양식을 맞춰주기 위한 옵션으로 거의 사용안함
			두번째 - 수신 확인을 위한 fd_set
			세번째 - 송신 확인을 위한 fd_set
			네번째 - 예외(OOB) 확인을 위한 fd_set (거의 사용 안함)
			다섯째 - timeout 인자 설정 가능
		
		*/
		int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
		if (retVal == SOCKET_ERROR)
			break;

		// Listener 소켓 체크
		if (FD_ISSET(listenSocket, &reads))//accept가 성공할 수 있는 상태이면
		{
			SOCKADDR_IN clientAddr;
			int32 addrLen = sizeof(clientAddr);
			SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET)
			{
				cout << "Client Connected" << endl;
				sessions.push_back(Session{ clientSocket });
			}
		}

		// 나머지 소켓 체크
		for (Session& s : sessions)
		{
			// Read
			if (FD_ISSET(s.socket, &reads))
			{
				int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
				if (recvLen <= 0)
				{
					// TODO : sessions 제거
					continue;
				}

				s.recvBytes = recvLen;
			}

			// Write
			if (FD_ISSET(s.socket, &writes))
			{
				// 블로킹 모드 send -> 모든 데이터 다 보냄
				// 논블로킹 모드 send -> 일부만 보낼 수가 있음 (상대방 수신 버퍼 상황에 따라)
				int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
				if (sendLen == SOCKET_ERROR)
				{
					// TODO : sessions 제거
					continue;
				}
				// send가 100% 되었는지 확인한 뒤 bufferCursor를 리셋
				s.sendBytes += sendLen;
				if (s.recvBytes == s.sendBytes)
				{
					s.recvBytes = 0;
					s.sendBytes = 0;
				}
			}
		}
	}

	
	// 윈속 종료
	::WSACleanup();
}