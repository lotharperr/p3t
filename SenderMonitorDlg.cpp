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
#include <list>
#include <sstream>
#include "Utils.h"
#include "SenderMonitorDlg.h"
#include "p3tDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CSenderMonitorDlg, CDialogEx)

CSenderMonitorDlg::CSenderMonitorDlg(CWnd* pParent /*=NULL*/)
	: m_Param_Server(_T("")),
	m_Param_Port(0),
	m_Param_PacketSize(0),
	m_Param_PacketsPerBlock(0),
	m_Param_BlockDelay(0),
	m_Param_Protocol(ProtocolType::TCP),
	m_PauseStatus(false),
	m_StopFlag(false), 
	m_RefreshCount(DEFAULTREFRESHRATE),
	m_NewValue(true),
	m_StatusString(_T("Idle")),
	m_LastStatusString(_T("")),
	m_Sender(NULL),
	m_SenderMonitorThread(NULL),
	CDialogEx(CSenderMonitorDlg::IDD, pParent)
{

}

CSenderMonitorDlg::~CSenderMonitorDlg()
{
	StopTest();
}

BOOL CSenderMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_brush.CreateSolidBrush(RGB(255, 255, 255)); // Weißen Hintergrun erzeugen
	return TRUE; 
}

void CSenderMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PACKETSSENT, m_PacketCount);
	DDX_Control(pDX, IDC_BYTESSENT, m_BytesSent);
	DDX_Control(pDX, IDC_PACKETSSENTPERSECOND, m_PacketsPerSecond);
	DDX_Control(pDX, IDC_BYTESSENTPERSECOND, m_BytesPerSecond);
	DDX_Control(pDX, IDC_SENDERSTATUS, m_SenderStatus);
	DDX_Control(pDX, IDC_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_RECEIVER, m_Receiver);
	DDX_Control(pDX, IDC_SETTINGS, m_Settings);
}

BEGIN_MESSAGE_MAP(CSenderMonitorDlg, CDialogEx)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CSenderMonitorDlg::OnCancelDialog)
	ON_BN_CLICKED(IDC_PAUSE, &CSenderMonitorDlg::OnBnClickedPause)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


void CSenderMonitorDlg::MonitorSenderThreadProc()
{
	time_t lastIntervalSecondValue = time(0);
	unsigned long long LastPacketCount=0;
	unsigned long long LastByteCount=0;
	unsigned long long IntervalPacketCount=0;
	unsigned long long IntervalByteCount=0;
	unsigned long long SecondPacketCount=0;
	unsigned long long SecondByteCount=0;
	unsigned short refreshCount = m_RefreshCount.load();

	while(!m_StopFlag.load())
	{
		int interval = 1000 / refreshCount;
		Sleep(interval);
		m_StatusStringLock.lock();
		m_StatusString = m_Sender->getStatusString();
		m_StatusStringLock.unlock();

		m_SenderTotalPackets = m_Sender->getPacketsSent();
		m_SenderTotalBytes = m_Sender->getBytesSent(); 

		// Differenz-Summen berechnen (Anzahl, Bytes)
		IntervalPacketCount = m_SenderTotalPackets.load() - LastPacketCount;
		IntervalByteCount = m_SenderTotalBytes.load() - LastByteCount;

		time_t now = time(0);
		if (lastIntervalSecondValue != now)
		{
			m_SenderCurrentPacketsPerSecond = SecondPacketCount; // Neue Sekunde angebrochen -> Werte speichern und zurücksetzen
			m_SenderCurrentBytesPerSecond = SecondByteCount;
			SecondPacketCount = 0; 
			SecondByteCount = 0;
			m_NewValue = true;
			lastIntervalSecondValue = now;
		}

		SecondPacketCount += IntervalPacketCount;
		SecondByteCount += IntervalByteCount;

		LastPacketCount = m_SenderTotalPackets.load();
		LastByteCount = m_SenderTotalBytes.load();
	}
	
	m_SenderCurrentPacketsPerSecond = 0;
	m_SenderCurrentBytesPerSecond = 0;
}

void CSenderMonitorDlg::UpdateSenderValues()
{
	if (!m_Sender->getPauseStatus())
	{
		m_StatusStringLock.lock();
		if (m_StatusString!=m_LastStatusString)
		{
			m_SenderStatus.SetWindowText(m_StatusString.c_str());
			m_LastStatusString = m_StatusString;
		}
		m_StatusStringLock.unlock();
	} else
	{
		m_LastStatusString=_T("");
		m_SenderStatus.SetWindowTextW(_T("Pause"));
	}

	if (m_Sender->getErrorStatus())
	{
		StopTest();
		m_btnPause.SetWindowText(_T("Retry"));
		m_SenderStatus.SetWindowText(m_StatusString.c_str());
	}

	std::wstringstream ss;
	if (m_Param_Protocol==TCP)
		ss << strTCP; else
		ss << strUDP; 

	ss << ", Packet size: " << CUtils::calculateSize(m_Param_PacketSize);
	if (m_Param_PacketsPerBlock>1)
		ss << ", Packets per Block: " << m_Param_PacketsPerBlock;
	if (m_Param_BlockDelay>0)
		ss << ", Block delay: " << m_Param_BlockDelay << "ms";

	//m_Param_PacketSize, m_Param_PacketsPerBlock, m_Param_BlockDelay, m_Param_Protocol
	m_Settings.SetWindowTextW(ss.str().c_str());

	m_PacketCount.SetWindowText(std::to_wstring(m_SenderTotalPackets.load()).c_str());
	m_BytesSent.SetWindowText(CUtils::calculateSize(m_SenderTotalBytes.load()).c_str());

	std::wstringstream ssBytes;
	ssBytes << CUtils::calculateSize(m_SenderCurrentBytesPerSecond.load()) << "/s (" << CUtils::calculateBitSize(m_SenderCurrentBytesPerSecond.load()) << "/s)";
	m_PacketsPerSecond.SetWindowText(std::to_wstring(m_SenderCurrentPacketsPerSecond.load()).c_str());
	m_BytesPerSecond.SetWindowText(ssBytes.str().c_str());
}

void CSenderMonitorDlg::StartTest()
{
	m_StopFlag=false;
	m_PauseStatus=false;

	m_Sender = new CSender(m_Param_Server, m_Param_Port, m_Param_PacketSize, m_Param_PacketsPerBlock, m_Param_BlockDelay, m_Param_Protocol);
	m_SenderMonitorThread = new std::thread(&CSenderMonitorDlg::MonitorSenderThreadProc, this);
	UpdateSenderValues();
	SetTimer (1, SMOOTHTIME, NULL);
	m_btnPause.SetWindowText(_T("Pause"));
}

void CSenderMonitorDlg::RunTest(CString Server, int Port, int PacketSize, int PacketsPerBlock, int BlockDelay, ProtocolType Protocol)
{
	std::wstringstream ss;
	std::wstring strIP(Server);
	ss << strIP << ":" << Port;
	std::wstring strServer = ss.str();
	m_Receiver.SetWindowText(ss.str().c_str());
	m_Param_Server = Server;
	m_Param_Port = Port;
	m_Param_PacketSize = PacketSize;
	m_Param_PacketsPerBlock = PacketsPerBlock;
	m_Param_BlockDelay = BlockDelay;
	m_Param_Protocol = Protocol;
	StartTest();
}

void CSenderMonitorDlg::StopTest()
{
	m_StopFlag = true;
	KillTimer(1);

	if (m_Sender!=NULL)
		m_Sender->StopTest();

	if (m_SenderMonitorThread!=NULL)
		if (m_SenderMonitorThread->joinable())
			m_SenderMonitorThread->join();
	
	if (m_Sender!=NULL)
	{
		delete(m_Sender);
		m_Sender=NULL;
	}

	if (m_SenderMonitorThread!=NULL)
	{
		delete(m_SenderMonitorThread);
		m_SenderMonitorThread=NULL;
	}
}

void CSenderMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1: 
			UpdateSenderValues();
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CSenderMonitorDlg::OnCancelDialog()
{
	StopTest();
	CDialogEx::OnCancel();
	delete this;
}

void CSenderMonitorDlg::OnBnClickedPause()
{
	m_PauseStatus=!m_PauseStatus;
	if (m_Sender==NULL)
	{
		StartTest();
	} else
	if (m_PauseStatus)
	{
		m_btnPause.SetWindowText(_T("Resume"));
		m_Sender->Pause();
	} else
	{
		m_btnPause.SetWindowText(_T("Pause"));
		m_Sender->Resume();
	}
}
