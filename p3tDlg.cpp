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
#include "afxdialogex.h"
#include <sstream>
#include <algorithm>
#include <list>

#include "p3t.h"
#include "p3tDlg.h"
#include "SenderMonitorDlg.h"
#include "ReceiverMonitorDlg.h"
#include "ErrorDialog.h"
#include "AboutDialg.h"
#include "Common.h"
#include "Utils.h"
#include "Receiver.h"
#include "Sender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int iBlockSize[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16434, 32768, 65536 };
wchar_t* strBlockSize[] = { _T("1b"), _T("2b"), _T("4b"), _T("8b"), _T("16b"), _T("32b"), _T("64b"), _T("128b"), _T("256b"), _T("512b"), _T("1k"), _T("2k"), _T("4k"), _T("8k"), _T("16k"), _T("32k"), _T("64k")};

Cp3tDlg::Cp3tDlg(CWnd* pParent)
	: CDialogEx(Cp3tDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cp3tDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROTO_SENDER, m_ProtoSender);
	DDX_Control(pDX, IDC_PROTO_RECEIVER, m_ProtoReceiver);
	DDX_Control(pDX, IDC_PORT, m_PortSender);
	DDX_Control(pDX, IDC_PORT2, m_PortReceiver);
	DDX_Control(pDX, IDC_BLOCKSIZE, m_BlockSize);
	DDX_Control(pDX, IDC_BACKETSPERBLOCK, m_PacketsPerBlock);
	DDX_Control(pDX, IDC_BLOCKDELAY, m_BlockDelay);
	DDX_Control(pDX, IDC_RECEIVERHOST, m_HostSender);
}

BEGIN_MESSAGE_MAP(Cp3tDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RUNSENDER, &Cp3tDlg::OnBnClickedRunSender)
	ON_BN_CLICKED(IDC_RUNRECEIVER, &Cp3tDlg::OnBnClickedRunReceiver)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(NM_CLICK, IDC_CREATESENDERLINK, &Cp3tDlg::OnNMClickCreatesenderlink)
	ON_NOTIFY(NM_CLICK, IDC_CREATERECEIVERLINK, &Cp3tDlg::OnNMClickCreatereceiverlink)
END_MESSAGE_MAP()


// Cp3tDlg-Meldungshandler

BOOL Cp3tDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	HMENU pSysMenu = ::GetSystemMenu(GetSafeHwnd(), FALSE);
	if (pSysMenu)
	{
		::InsertMenu(pSysMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ABOUT, _T("About"));
		::InsertMenu(pSysMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
	}
	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	m_brush.CreateSolidBrush(RGB(255, 255, 255)); // Weißen Hintergrund erzeugen
	m_ProtoSender.AddString(strTCP);
	m_ProtoSender.AddString(strUDP);
	m_ProtoReceiver.AddString(strTCP);
	m_ProtoReceiver.AddString(strUDP);

	m_ProtoSender.SetCurSel(0);
	m_ProtoReceiver.SetCurSel(0);
	m_HostSender.SetWindowText(_T("localhost"));
	
	wchar_t c[10];
	_itow_s(DEFAULTPORT, c, 10);
	m_PortSender.SetWindowText(c);
	m_PortReceiver.SetWindowText(c);

	_itow_s(DEFAULTBLOCKDELAY, c, 10);
	m_BlockDelay.SetWindowText(c);

	_itow_s(DEFAULTPACKETSPERBLOCK, c, 10);
	m_PacketsPerBlock.SetWindowText(c);

	for (int i=0; i<sizeof(strBlockSize) / sizeof(strBlockSize[0]); i++)
		m_BlockSize.AddString(strBlockSize[i]);
	m_BlockSize.SetCurSel(16);

	return TRUE; 
}

void Cp3tDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR Cp3tDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cp3tDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID==ID_MENU_ABOUT)
	{
		CAboutDialg aboutDialog;
		aboutDialog.DoModal();
	}
	CDialogEx::OnSysCommand(nID, lParam);
}

void Cp3tDlg::CreateNewSender()
{
	CString strServer;
	CString strPort;
	CString strPacketsPerBlock;
	CString strBlockDelay;
	int Port, BlockSize, PacketsPerBlock, BlockDelay;
	ProtocolType Protocol;

	CSenderMonitorDlg *senderMonitor = new CSenderMonitorDlg();
	if (senderMonitor != NULL)
	{
		BOOL result = senderMonitor->Create(IDD_SENDERMONITOR, this); // nicht modales Fenster erzeugen
		if (!result)   //Create failed.
		{
			AfxMessageBox(_T("Error creating Dialog"));
			delete senderMonitor;
			return;
		}

		Protocol = (ProtocolType)m_ProtoSender.GetCurSel();
		m_HostSender.GetWindowText(strServer);
		m_PortSender.GetWindowText(strPort);
		m_PacketsPerBlock.GetWindowText(strPacketsPerBlock);
		m_BlockDelay.GetWindowText(strBlockDelay);

		Port=_wtoi(strPort);
		if (Port==0) Port=DEFAULTPORT;

		PacketsPerBlock=_wtoi(strPacketsPerBlock);
		if (PacketsPerBlock==0) PacketsPerBlock=DEFAULTPACKETSPERBLOCK;

		BlockDelay=_wtoi(strBlockDelay);
		if (BlockDelay==0) BlockDelay=DEFAULTBLOCKDELAY;

		BlockSize = iBlockSize[m_BlockSize.GetCurSel()];

		senderMonitor->ShowWindow(SW_SHOW);

		if (strServer=="")
			strServer = "localhost";

		senderMonitor->RunTest(strServer, Port, BlockSize, PacketsPerBlock, BlockDelay, Protocol);		
	}
}

void Cp3tDlg::CreateNewReceiver()
{
	CString strPort;
	int Port;
	ProtocolType Protocol;
	
	CReceiverMonitorDlg *receiverMonitor = new CReceiverMonitorDlg();
	if (receiverMonitor != NULL)
	{
		BOOL result = receiverMonitor->Create(IDD_RECEIVERMONITOR, this); // nicht modales Fenster erzeugen
		if (!result)   //Create failed.
		{
			AfxMessageBox(_T("Error creating Dialog"));
			delete receiverMonitor;
			return;
		}

		m_PortReceiver.GetWindowText(strPort);
		Protocol = (ProtocolType)m_ProtoReceiver.GetCurSel();
		Port=_wtoi(strPort);
		if (Port==0)
			Port=DEFAULTPORT;

		receiverMonitor->RunTest(Port, Protocol);
		receiverMonitor->ShowWindow(SW_SHOW);
	}
}

void Cp3tDlg::OnBnClickedRunSender()
{
	CreateNewSender();
}

void Cp3tDlg::OnBnClickedRunReceiver()
{
	CreateNewReceiver();
}

void Cp3tDlg::OnNMClickCreatesenderlink(NMHDR *pNMHDR, LRESULT *pResult)
{
	CreateNewSender();
	*pResult = 0;
}


void Cp3tDlg::OnNMClickCreatereceiverlink(NMHDR *pNMHDR, LRESULT *pResult)
{
	CreateNewReceiver();
	*pResult = 0;
}
