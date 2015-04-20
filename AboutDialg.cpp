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
#include "p3t.h"
#include "AboutDialg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CAboutDialg, CDialogEx)

CAboutDialg::CAboutDialg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAboutDialg::IDD, pParent)
{

}

CAboutDialg::~CAboutDialg()
{
}

void CAboutDialg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAILADDRESS, m_SysLinkMailAddress);
	DDX_Control(pDX, IDC_CHARTCONTROLLINK, m_SysLinkChartControlLink);
}

BEGIN_MESSAGE_MAP(CAboutDialg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDialg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_SysLinkMailAddress.SizeToContent();
	m_SysLinkChartControlLink.SizeToContent();

	return TRUE;
}
