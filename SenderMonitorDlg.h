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
#include "Sender.h"
#include "afxwin.h"
#include <string>
#include <mutex>
#include "afxlinkctrl.h"
#include "p3t.h"

class CSenderMonitorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSenderMonitorDlg)

private:
	CString m_Param_Server;
	int m_Param_Port;
	int m_Param_PacketSize;
	int m_Param_PacketsPerBlock;
	int m_Param_BlockDelay;
	ProtocolType m_Param_Protocol;

	CBrush m_brush;
	class CSender *m_Sender;
	std::thread *m_SenderMonitorThread;
	std::atomic<bool> m_StopFlag;
	std::atomic<unsigned short> m_RefreshCount;
	std::mutex m_StatusStringLock;
	std::wstring m_StatusString;
	std::wstring m_LastStatusString;
	std::atomic<unsigned long long> m_SenderTotalPackets;
	std::atomic<unsigned long long> m_SenderTotalBytes;
	std::atomic<unsigned long long> m_SenderCurrentPacketsPerSecond;
	std::atomic<unsigned long long> m_SenderCurrentBytesPerSecond;
	std::atomic<bool> m_NewValue;

	bool m_PauseStatus;
	void MonitorSenderThreadProc();
	void UpdateSenderValues();
	void StartTest();
	void StopTest();

public:
	CSenderMonitorDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CSenderMonitorDlg();

	void RunTest(CString Server, int Port, int PacketSize, int PacketsPerBlock, int BlockDelay, ProtocolType Protocol);

// Dialogfelddaten
	enum { IDD = IDD_SENDERMONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_PacketCount;
	CEdit m_BytesSent;
	CEdit m_PacketsPerSecond;
	CEdit m_BytesPerSecond;
	CEdit m_SenderStatus;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCancelDialog();
	afx_msg void OnBnClickedPause();
	CButton m_btnPause;
	CEdit m_Receiver;
	CEdit m_Settings;
};
