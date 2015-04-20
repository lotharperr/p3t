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
#include <sstream>
#include "ErrorDialog.h"

std::wstring CErrorDialog::GetSystemErrorMessage(int ErrorID)
{
	LPTSTR errorText = NULL;
	std::wstring result;

	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM
		|FORMAT_MESSAGE_ALLOCATE_BUFFER
		|FORMAT_MESSAGE_IGNORE_INSERTS,  
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		ErrorID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if ( errorText!=NULL )
	{
		std::wstringstream ss;
		ss << ErrorID << _T(" ") << errorText;
		LocalFree(errorText);
		errorText = NULL;
		result = ss.str();
	} else
		result = _T("Unknown Error");
	return result;
}

void CErrorDialog::ShowWSAErrorMessage(CString Component, CString Message, int ErrorID)
{
	std::wstringstream ss;
	ss << GetSystemErrorMessage(ErrorID);
	MessageBox(0, ss.str().c_str(), Component, MB_OK | MB_ICONERROR);
}

void CErrorDialog::ShowInfoMessage(CString Component, CString Message)
{
	MessageBox(0, Message, Component, MB_OK | MB_ICONINFORMATION);
}

void CErrorDialog::ShowErrorMessage(CString Component, CString Message)
{
	MessageBox(0, Message, Component, MB_OK | MB_ICONERROR);
}
