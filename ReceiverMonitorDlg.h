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
#include <map>
#include <vector>
#include <mutex>
#include "Receiver.h"
#include "afxwin.h"
#include "Deps\ChartCtrl\ChartDef.h"
#include "Deps\ChartCtrl\ChartContainer.h"

class CReceiverMonitorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CReceiverMonitorDlg)

private:
	CBrush m_brush;
	CChartContainer m_chartContainer;
	V_CHARTDATAD m_ChartData;
	int m_ChartID;
	int m_Param_Port;
	int m_LastChartSecond;
	ProtocolType m_Param_Protocol;

	class CReceiver *m_Receiver;
	std::thread *m_ReceiverMonitorThread;
	std::atomic<bool> m_StopFlag;
	std::atomic<unsigned short> m_RefreshCount;
	std::atomic<unsigned int> m_SmoothCount;
	std::mutex m_StatusStringLock;
	std::wstring m_StatusString;
	std::wstring m_LastStatusString;

	std::atomic<unsigned long long> m_ReceiverTotalPackets;
	std::atomic<unsigned long long> m_ReceiverTotalBytes;
	std::atomic<unsigned long long> m_ReceiverCurrentPacketsPerSecond;
	std::atomic<unsigned long long> m_ReceiverCurrentBytesPerSecond;
	std::atomic<unsigned long long> m_ReceiverPacketsLastSecond;
	std::atomic<unsigned long long> m_ReceiverBytesLastSecond;

	std::atomic<bool> m_NewValue;
	void MonitorReceiverThreadProc();
	void UpdateReceiverValues();
	void AddChartValue(unsigned long long value);

	void StartTest();
	void StopTest();

	int m_Chart_CurrentStart;
	int m_Chart_CurrentPos;
	int m_Chart_CurrentValue;

public:
	CReceiverMonitorDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CReceiverMonitorDlg();

	void RunTest(int Port, ProtocolType Protocol);

// Dialogfelddaten
	enum { IDD = IDD_RECEIVERMONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_PacketCount;
	CEdit m_BytesReceived;
	CEdit m_PacketsPerSecond;
	CEdit m_BytesPerSecond;
	CStatic m_YMax;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCancelDialog();
};
