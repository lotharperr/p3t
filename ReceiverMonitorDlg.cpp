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
#include <algorithm>
#include "Utils.h"
#include "p3t.h"
#include "ReceiverMonitorDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CReceiverMonitorDlg, CDialogEx)

CReceiverMonitorDlg::CReceiverMonitorDlg(CWnd* pParent /*=NULL*/)
:	m_Param_Port(0),
	m_Param_Protocol(ProtocolType::TCP),
	m_StopFlag(false), 
	m_RefreshCount(DEFAULTREFRESHRATE),
	m_SmoothCount(DEFAULTSMOOTHCOUNT),
	m_NewValue(true),
	m_StatusString(_T("Idle")),
	m_LastStatusString(_T("")),
	m_ReceiverMonitorThread(NULL),
	m_Chart_CurrentStart(0),
	m_Chart_CurrentPos(0),
	m_Chart_CurrentValue(20000),
	CDialogEx(CReceiverMonitorDlg::IDD, pParent)
{

}

CReceiverMonitorDlg::~CReceiverMonitorDlg()
{
	StopTest();
}

void CReceiverMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PACKETSRECEIVED, m_PacketCount);
	DDX_Control(pDX, IDC_BYTESRECEIVED, m_BytesReceived);
	DDX_Control(pDX, IDC_PACKETSRECEIVEDPERSECOND, m_PacketsPerSecond);
	DDX_Control(pDX, IDC_BYTESRECEIVEDPERSECOND, m_BytesPerSecond);
	DDX_Control(pDX, IDC_Y_MAX, m_YMax);
	DDX_Control(pDX, IDC_CHART, m_chartContainer);
}


BEGIN_MESSAGE_MAP(CReceiverMonitorDlg, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CReceiverMonitorDlg::OnCancelDialog)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CReceiverMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_brush.CreateSolidBrush(RGB(255, 255, 255)); // Weißen Hintergrund erzeugen

// Init Chart
	m_YMax.SetWindowText(rxmaxDescription_default);
	m_chartContainer.SetContainerName(_T("rxmax"));
	m_ChartID = m_chartContainer.AddChart(true, true, _T(""), _T("y"), 1, Gdiplus::DashStyle::DashStyleSolid, 1.0f, 0.0f, Gdiplus::Color::Red /*Gdiplus::Color::Yellow*/, m_ChartData, true);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CReceiverMonitorDlg::MonitorReceiverThreadProc()
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

		m_ReceiverTotalPackets = m_Receiver->getPacketsReceived();
		m_ReceiverTotalBytes = m_Receiver->getBytesReceived(); 

		// Differenz-Summen berechnen (Anzahl, Bytes)
		IntervalPacketCount = m_ReceiverTotalPackets.load() - LastPacketCount;
		IntervalByteCount = m_ReceiverTotalBytes.load() - LastByteCount;

		time_t now = time(0);
		if (lastIntervalSecondValue != now)
		{
			m_ReceiverPacketsLastSecond = SecondPacketCount; // Neue Sekunde angebrochen -> Werte speichern und zurücksetzen
			m_ReceiverBytesLastSecond = SecondByteCount;
			SecondPacketCount = 0; 
			SecondByteCount = 0;
			m_NewValue = true;
			lastIntervalSecondValue = now;
		}

		SecondPacketCount += IntervalPacketCount;
		SecondByteCount += IntervalByteCount;

		m_ReceiverCurrentPacketsPerSecond = SecondPacketCount;
		m_ReceiverCurrentBytesPerSecond = SecondByteCount;
		
		LastPacketCount = m_ReceiverTotalPackets.load();
		LastByteCount = m_ReceiverTotalBytes.load();
		Sleep(interval);
	}
}

void CReceiverMonitorDlg::UpdateReceiverValues()
{
	m_PacketCount.SetWindowText(std::to_wstring(m_ReceiverTotalPackets.load()).c_str());
	m_BytesReceived.SetWindowText(CUtils::calculateSize(m_ReceiverTotalBytes.load()).c_str());

	if (m_NewValue.load())
	{
		std::wstringstream ssBytes;
		ssBytes << CUtils::calculateSize(m_ReceiverBytesLastSecond.load()) << "/s (" << CUtils::calculateBitSize(m_ReceiverBytesLastSecond.load()) << "/s)";
		m_PacketsPerSecond.SetWindowText(std::to_wstring(m_ReceiverPacketsLastSecond.load()).c_str());
		m_BytesPerSecond.SetWindowText(ssBytes.str().c_str());
	}

	AddChartValue(m_ReceiverCurrentBytesPerSecond.load());
}

void CReceiverMonitorDlg::StartTest()
{
	m_StopFlag=false;
	m_Receiver = new CReceiver(m_Param_Port, m_Param_Protocol);
	m_ReceiverMonitorThread = new std::thread(&CReceiverMonitorDlg::MonitorReceiverThreadProc, this);
	UpdateReceiverValues();
	SetTimer (1, 1000 / m_RefreshCount.load(), NULL);
}

void CReceiverMonitorDlg::RunTest(int Port, ProtocolType Protocol)
{
	m_Param_Protocol = Protocol;
	m_Param_Port = Port;
	StartTest();
}

void CReceiverMonitorDlg::StopTest()
{
	m_StopFlag = true;
	KillTimer(1);

	if (m_Receiver!=NULL)
		m_Receiver->StopTest();

	if (m_ReceiverMonitorThread!=NULL)
		if (m_ReceiverMonitorThread->joinable())
			m_ReceiverMonitorThread->join();
	
	if (m_Receiver!=NULL)
	{
		delete(m_Receiver);
		m_Receiver=NULL;
	}

	if (m_ReceiverMonitorThread!=NULL)
	{
		delete(m_ReceiverMonitorThread);
		m_ReceiverMonitorThread=NULL;
	}
}

void CReceiverMonitorDlg::AddChartValue(unsigned long long value)
{
	unsigned long long maxValue=0;
	const unsigned int pointCount=100;
	bool newValue = m_NewValue.load();

	// Aktuellen Sekundenwert aktualisieren oder neuen Wert hinzufügen
	
	if (newValue) // gleiche Sekunde
	{
		m_NewValue=false;
		while (m_ChartData.size()<pointCount+m_Chart_CurrentStart)
		{
			PointD p(m_Chart_CurrentPos++, (double)value); 
			m_ChartData.push_back(p);
		}
	} else
	{
		int size = m_ChartData.size();
		m_ChartData[size-1].Y = (double)value;
	}

	int testSize = m_ChartData.size();
	if (testSize>pointCount) 
		testSize=pointCount;

	maxValue=0;
	
	for (unsigned int c=m_Chart_CurrentStart; c<m_ChartData.size(); c++)
	{
		if (m_ChartData.at(c).Y>maxValue)
			maxValue=(unsigned long long)m_ChartData.at(c).Y;
	}

	std::wstringstream maxYValue;
	maxYValue << rxmaxDescription << CUtils::calculateSize(maxValue) << "/s (" << CUtils::calculateBitSize(maxValue) << "/s)";

	m_YMax.SetWindowText(maxYValue.str().c_str());
	m_chartContainer.UpdateExtX(m_Chart_CurrentStart, m_Chart_CurrentStart+pointCount);
	m_chartContainer.ZoomMoveContainerX(m_Chart_CurrentStart, m_Chart_CurrentStart+pointCount, true);
	m_chartContainer.ZoomMoveContainerY(0, maxValue*1.1, true, true);
	m_chartContainer.ReplaceChartData(m_ChartID, m_ChartData, true); 
	
	if (newValue)
		m_Chart_CurrentStart++;
	m_chartContainer.RedrawWindow();
}

void CReceiverMonitorDlg::OnCancelDialog()
{
	StopTest();
	CDialogEx::OnCancel();
	delete this;
}

void CReceiverMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case 1: 
		{
			KillTimer(1);
			UpdateReceiverValues();
			SetTimer (1, 1000 / m_RefreshCount.load(), NULL);
		}
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
