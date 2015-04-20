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
#include <string>
class CUtils
{
public:
	const static std::wstring CUtils::utf8_to_wstring (const std::string& str);
	const static std::string CUtils::wstring_to_utf8 (const std::wstring& str);
	const static std::string CString_to_utf8(const CString &str);
	const static CString utf8_to_CString(const char* str);
	static std::wstring calculateSize(unsigned long long size);
	static std::wstring calculateBitSize(unsigned long long size);
};

