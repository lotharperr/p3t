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
#include "afxwin.h"
#include "afxcmn.h"
#include <atomic>
#include <thread>

#include <map>
#include <vector>
#include <tuple>
#include <list>

#include "SenderMonitorDlg.h"
#include "Deps\ChartCtrl\ChartDef.h"
#include "Deps\ChartCtrl\ChartContainer.h"

// Cp3tDlg-Dialogfeld
class Cp3tDlg : public CDialogEx
{
// Konstruktion
public:
	Cp3tDlg(CWnd* pParent = NULL);	// Standardkonstruktor

// Dialogfelddaten
	enum { IDD = IDD_P3T_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brush;
	void CreateNewSender();
	void CreateNewReceiver();
public:
	CComboBox m_ProtoSender;
	CComboBox m_ProtoReceiver;
	CEdit m_PortSender;
	CEdit m_PortReceiver;
	afx_msg void OnBnClickedRunSender();
	afx_msg void OnBnClickedRunReceiver();
	CComboBox m_BlockSize;
	CEdit m_PacketsPerBlock;
	CEdit m_BlockDelay;
//	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnNMClickCreatesenderlink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickCreatereceiverlink(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_HostSender;
};
