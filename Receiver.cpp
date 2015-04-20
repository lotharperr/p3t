/***************************************************************************
*  Copyright 2015 Lothar Perr
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
***************************************************************************/

#include "stdafx.h"
#include "ErrorDialog.h"
#include "Receiver.h"
#include <sstream>
#include <fstream>

#define BUFLEN 32768  //Max length of buffer

CReceiver::CReceiver(int Port, ProtocolType protocol)
	: m_Protocol(protocol),
	m_TAG("Receiver"),
	m_Port(Port),
	m_TCPConnected(false),
	m_StopFlag(false),
	m_PacketsReceived(0),
	m_BytesReceived(0),
	m_UDPSocket(INVALID_SOCKET),
	m_TCPListenSocket(INVALID_SOCKET),
	m_TCPClientSocket(INVALID_SOCKET)
{
	switch (m_Protocol)
	{
	case ProtocolType::TCP:
		m_ReceiverThread = new std::thread(&CReceiver::ReceiverThreadProc_TCP, this);
		break;
	case ProtocolType::UDP:
		m_ReceiverThread = new std::thread(&CReceiver::ReceiverThreadProc_UDP, this);
		break;
	}
}

CReceiver::~CReceiver()
{
	StopTest();
}

#pragma optimize( "", off ) // Optimierung muss für atomic_ullong - Typen abgeschaltet werden, Compiler-Fehler in VS2012
void CReceiver::ReceiverThreadProc_TCP()
{
    int iResult;
	int recv_len;
	const size_t portBufSize = 13;

	struct addrinfo *addressInfo = NULL;
	struct addrinfo hints;

	char portBuf[portBufSize];
	char recvbuf[BUFLEN];
	int recvbuflen = BUFLEN;
	memset(portBuf,0,portBufSize);
	_itoa_s(m_Port, portBuf, portBufSize, 10);

	iResult = WSAStartup(MAKEWORD(2,2), &m_WSA);
	if (iResult != 0) {
		CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("WSAStartup failed."), iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	while(!m_StopFlag.load()) 
	{
		m_TCPConnected = false;
		iResult = getaddrinfo(NULL, portBuf, &hints, &addressInfo);
		if ( iResult != 0 ) {
			WSACleanup();
			CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("getaddrinfo failed."), iResult);
			return;
		}

		m_TCPListenSocket = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
		if (m_TCPListenSocket == INVALID_SOCKET) {
			iResult = WSAGetLastError();
			freeaddrinfo(addressInfo);
			WSACleanup();
			CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("socket failed."), iResult);
			return;
		}

		int iOptVal = 0;
		int iOptLen = sizeof (int);
		iResult = setsockopt(m_TCPListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &iOptVal, iOptLen);
		if (iResult == SOCKET_ERROR) {
			iResult = WSAGetLastError();
			freeaddrinfo(addressInfo);
			closesocket(m_TCPListenSocket);
			WSACleanup();
			CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("setsockopt failed."), iResult);
			return;
		}

		iResult = bind( m_TCPListenSocket, addressInfo->ai_addr, (int)addressInfo->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			iResult = WSAGetLastError();
			freeaddrinfo(addressInfo);
			closesocket(m_TCPListenSocket);
			WSACleanup();
			CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("bind failed."), iResult);
			return;
		}

		freeaddrinfo(addressInfo);

		iResult = listen(m_TCPListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			iResult = WSAGetLastError();
			closesocket(m_TCPListenSocket);
			WSACleanup();
			CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("listen failed."), iResult);
			return;
		}

		m_TCPClientSocket = accept(m_TCPListenSocket, NULL, NULL);
		if (m_TCPClientSocket == INVALID_SOCKET) {
			iResult = WSAGetLastError();
			closesocket(m_TCPListenSocket);
			WSACleanup();
			if (iResult!=WSAEINTR)
				CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("accept failed."), iResult);
			return;
		}

		closesocket(m_TCPListenSocket);
		m_TCPConnected = true;

		// Receive until the peer shuts down the connection
		while(!m_StopFlag.load())
		{
			recv_len = recv(m_TCPClientSocket, recvbuf, recvbuflen, 0);
			if (recv_len > 0) 
			{
				m_PacketsReceived++;
				m_BytesReceived+=recv_len;
			}
			else if (recv_len == 0)
			{
				break;
			}
			else  {
				iResult = WSAGetLastError();
				closesocket(m_TCPClientSocket);
				WSACleanup();
				if (iResult!=WSAEINTR)
					CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("recv failed."), iResult);
				return;
			}
		}

		// shutdown the connection since we're done
		shutdown(m_TCPClientSocket, SD_RECEIVE);
		closesocket(m_TCPClientSocket);

	}
    WSACleanup();
}
#pragma optimize( "", on ) // Optimierung muss für atomic_ullong - Typen abgeschaltet werden, Compiler-Fehler in VS2012

#pragma optimize( "", off ) // Optimierung muss für atomic_ullong - Typen abgeschaltet werden, Compiler-Fehler in VS2012
void CReceiver::ReceiverThreadProc_UDP()
{
	struct sockaddr_in server, si_other;
	int slen , recv_len;
	char buf[BUFLEN];

	slen = sizeof(si_other) ;

	if (WSAStartup(MAKEWORD(2,2),&m_WSA) != 0)
	{
		CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("WSAStartup failed."), WSAGetLastError());
		return;
	}

	if((m_UDPSocket = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
	{
		CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("Could not create socket."), WSAGetLastError());
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( m_Port );

	if( bind(m_UDPSocket ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("Bind socket failed."), WSAGetLastError());
		return;
	}

	while(!m_StopFlag.load())
	{
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(m_UDPSocket, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			int errorCode = WSAGetLastError();
			switch (errorCode)
			{
			case WSAEINTR: break;
			default:
				CErrorDialog::ShowWSAErrorMessage(m_TAG, _T("recvfrom from socket failed."), errorCode);
				m_StopFlag=true;
				break;
			}
		} else
		{
			m_PacketsReceived++;
			m_BytesReceived+=recv_len;
		}
	}
	closesocket(m_UDPSocket);
	WSACleanup();
}
#pragma optimize( "", on ) // Optimierung muss für atomic_ullong - Typen abgeschaltet werden, Compiler-Fehler in VS2012

void CReceiver::ShutdownUDPTest()
{
	shutdown(m_UDPSocket, SD_BOTH);	
	closesocket(m_UDPSocket);
}

void CReceiver::ShutdownTCPTest()
{
	if (m_TCPConnected)
	{
		shutdown(m_TCPClientSocket, SD_BOTH);	
		closesocket(m_TCPClientSocket);
	} else
	{
		shutdown(m_TCPListenSocket, SD_BOTH);	
		closesocket(m_TCPListenSocket);
	}
}

void CReceiver::StopTest()
{
	m_StopFlag=true;	
	switch(m_Protocol)
	{
		case ProtocolType::TCP: ShutdownTCPTest(); break;
		case ProtocolType::UDP: ShutdownUDPTest(); break;
	}

	if (m_ReceiverThread!=NULL)
	{
		if (m_ReceiverThread->joinable())
			m_ReceiverThread->join();
		delete (m_ReceiverThread);
		m_ReceiverThread=NULL;
	}
}

