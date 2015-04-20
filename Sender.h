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

#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include "Common.h"

class CSender
{
private:
	CSender(const CSender&);
    CSender& operator=(const CSender&);
    SOCKET m_Socket;
    WSADATA m_WSA;
	ProtocolType m_Protocol;
	std::atomic<bool> m_StopFlag;
	std::atomic<bool> m_PauseFlag;
	std::atomic<bool> m_ErrorFlag;
	std::mutex m_StatusStringLock;
	std::wstring m_StatusString;
	void setStatus(std::wstring StatusString);
	void setErrorStatus(std::wstring StatusString, int ErrorID);
	CString m_Server;
	const int m_Port;
	const int m_PacketSize;
	const int m_BlockDelay;
	const int m_PacketsPerBlock;
	CString m_TAG;
	std::thread *m_SenderThread;
	void SenderThreadProc_TCP();
	void SenderThreadProc_UDP();
	std::atomic<unsigned long long> m_PacketsSent;
	std::atomic<unsigned long long> m_BytesSent;
public:
	CSender(CString Server, int Port, int PacketSize, int PacketsPerBlock, int BlockDelay, ProtocolType protocol);
	~CSender();
	void StopTest();
	unsigned long long getPacketsSent() {return m_PacketsSent.load();};
	unsigned long long getBytesSent() {return m_BytesSent.load();};
	std::wstring getStatusString();
	void Pause() {m_PauseFlag=true;};
	void Resume() {m_PauseFlag=false;};
	bool getErrorStatus() {return m_ErrorFlag.load();};
	bool getPauseStatus() {return m_PauseFlag.load();};
};

