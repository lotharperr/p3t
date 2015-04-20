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
#include "Sender.h"
#include "Utils.h"
#include <memory>
#include <sstream>

CSender::CSender(CString Server, int Port, int PacketSize, int PacketsPerBlock, int BlockDelay, ProtocolType protocol)
	: m_Protocol(protocol),
	m_TAG("Sender"),
	m_Server(Server),
	m_Port(Port),
	m_PacketSize(PacketSize),
	m_PacketsPerBlock(PacketsPerBlock),
	m_BlockDelay(BlockDelay),
	m_StopFlag(false),
	m_PauseFlag(false),
	m_ErrorFlag(false),
	m_PacketsSent(0),
	m_BytesSent(0),
	m_StatusString(_T("Idle")),
	m_SenderThread(NULL)
{
	switch (m_Protocol)
	{
	case ProtocolType::TCP:
		m_SenderThread = new std::thread(&CSender::SenderThreadProc_TCP, this);
		break;
	case ProtocolType::UDP:
		m_SenderThread = new std::thread(&CSender::SenderThreadProc_UDP, this);
		break;
	}
}

CSender::~CSender()
{
	StopTest();
}

void CSender::StopTest()
{
	m_StopFlag=true;
	if (m_SenderThread!=NULL)
	{
		if (m_SenderThread->joinable())
			m_SenderThread->join();
		delete(m_SenderThread);
		m_SenderThread=NULL;
	}
}

std::wstring CSender::getStatusString()
{
	std::wstring result;
	m_StatusStringLock.lock();
	result = m_StatusString;
	m_StatusStringLock.unlock();
	return result;
}

void CSender::setStatus(std::wstring StatusString)
{
	m_StatusStringLock.lock();
	m_StatusString = StatusString;
	m_StatusStringLock.unlock();
}

void CSender::setErrorStatus(std::wstring StatusString, int ErrorID)
{
	m_ErrorFlag=true;
	std::wstringstream ss;
	if (ErrorID>0)
		ss << StatusString << _T(": ") << CErrorDialog::GetSystemErrorMessage(ErrorID); else
		ss << StatusString;
	setStatus(ss.str());
}

#pragma optimize( "", off ) // Optimierung muss für atomic_ullong - Typen abgeschaltet werden, Compiler-Fehler in VS2012
void CSender::SenderThreadProc_TCP()
{
	const size_t portBufSize = 13;
	char portBuf[portBufSize];

	memset(portBuf,0,portBufSize);
	_itoa_s(m_Port, portBuf, portBufSize, 10);

	m_Socket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;
	setStatus(_T("Connecting"));
	iResult = WSAStartup(MAKEWORD(2,2), &m_WSA);
	if (iResult != 0) {
		setErrorStatus(_T("WSAStartup failed"), iResult);
		return;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(CUtils::CString_to_utf8(m_Server).c_str(), portBuf, &hints, &result);
	if ( iResult != 0 ) {
		WSACleanup();
		setErrorStatus(_T("getaddrinfo failed"), iResult);
		return;
	}

	std::wstring lastErrorMessage=_T("");
	int lastErrorCode=ERROR_SUCCESS;
	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		m_Socket = socket(ptr->ai_family, ptr->ai_socktype, 
			ptr->ai_protocol);
		if (m_Socket == INVALID_SOCKET) 
		{
			iResult = WSAGetLastError();
			WSACleanup();
			lastErrorMessage=_T("socket failed");
			lastErrorCode=iResult;
			continue;
		}

		// Connect to server.
		iResult = connect( m_Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			iResult = WSAGetLastError();
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			lastErrorMessage=_T("connect failed");
			lastErrorCode=iResult;
			continue;
		}
		setStatus(_T("Connected"));
		break;
	}

	freeaddrinfo(result);

	if (m_Socket == INVALID_SOCKET) 
	{
		setErrorStatus(lastErrorMessage, lastErrorCode);
		WSACleanup();
		return;
	}

	unsigned long long messageID=0;
	const int maxPacketSize = 65535;
	char packetBuffer[maxPacketSize];

	if (packetBuffer!=NULL)
	{
		memset(packetBuffer, 42, maxPacketSize); // mit * initialisieren
		while(!m_StopFlag.load())
		{
			if (!m_PauseFlag.load())
			{
				iResult = send( m_Socket, packetBuffer, m_PacketSize, 0 );
				if (iResult == SOCKET_ERROR) {
					iResult = WSAGetLastError();
					closesocket(m_Socket);
					WSACleanup();
					setErrorStatus(_T("send failed"), iResult);
					return;
				} else
				{
					m_PacketsSent++;
					m_BytesSent+=iResult;

					messageID++;
					if (m_BlockDelay>0)
						if (messageID % m_PacketsPerBlock == 0)
							Sleep(m_BlockDelay);
				}
			} else
			{
				Sleep(200);
			}
		}
		iResult = shutdown(m_Socket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			iResult = WSAGetLastError();
			closesocket(m_Socket);
			WSACleanup();
			setErrorStatus(_T("shutdown failed"), iResult);
			return;
		}
		setStatus(_T("Disconnected"));
		closesocket(m_Socket);
		WSACleanup();
	}
}
#pragma optimize( "", on )

#pragma optimize( "", off )
void CSender::SenderThreadProc_UDP()
{
	try
	{
		int iResult=0;
		struct sockaddr_in si_other;
		int slen=sizeof(si_other);

		char* packetBuffer;

		CString message;
		WSADATA wsa;

		setStatus(_T("Initializing"));

		//Initialise winsock
		if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
		{
			setErrorStatus(_T("WSAStartup failed"), WSAGetLastError());
			return;
		}

		//create socket
		if ( (m_Socket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			setErrorStatus(_T("Create socket failed"), WSAGetLastError());
			return;
		}

		//setup address structure
		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(m_Port);
		si_other.sin_addr.S_un.S_addr = inet_addr(CUtils::CString_to_utf8(m_Server).c_str());

		//start communication
		unsigned long long messageID=0;

		packetBuffer = (char*) malloc(m_PacketSize);
		memset(packetBuffer, 42, m_PacketSize); // mit * initialisieren
		if (packetBuffer!=NULL)
		{
			setStatus(_T("Sending"));

			while(!m_StopFlag.load())
			{
				if (!m_PauseFlag.load())
				{
					memcpy_s(packetBuffer, m_PacketSize, &messageID, sizeof(messageID)); // Message-ID schreiben
					iResult=sendto(m_Socket, packetBuffer, m_PacketSize, 0 , (struct sockaddr *) &si_other, slen);
					if (iResult == SOCKET_ERROR)
					{
						setErrorStatus(_T("Sendto failed"), WSAGetLastError());
						m_StopFlag=true;
					} else
					{
						m_PacketsSent++;
						m_BytesSent+=iResult;

						messageID++;
						if (m_BlockDelay>0)
							if (messageID % m_PacketsPerBlock == 0)
								Sleep(m_BlockDelay);
					}
				} else
				{
					Sleep(200);
				}
			}
			free(packetBuffer);
		} else
		{
			setErrorStatus(_T("Can't allocate message buffer"), -1);
		}

		closesocket(m_Socket);
		WSACleanup();
	}
	catch(std::exception &e)
	{
		setErrorStatus(CUtils::utf8_to_wstring(e.what()), -1);
	}
}
#pragma optimize( "", on )
