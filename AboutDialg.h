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
#include "afxlinkctrl.h"

class CAboutDialg : public CDialogEx
{
	DECLARE_DYNAMIC(CAboutDialg)

public:
	CAboutDialg(CWnd* pParent = NULL);  
	virtual ~CAboutDialg();

	enum { IDD = IDD_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CMFCLinkCtrl m_SysLinkMailAddress;
	CMFCLinkCtrl m_SysLinkChartControlLink;
};
