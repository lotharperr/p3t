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
#include <string>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <locale>
#include <codecvt>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Utils.h"

const std::wstring CUtils::utf8_to_wstring (const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

const std::string CUtils::wstring_to_utf8 (const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

const std::string CUtils::CString_to_utf8(const CString &str)
{
	return wstring_to_utf8(std::wstring(str));
}

const CString CUtils::utf8_to_CString(const char* str)
{
	return utf8_to_wstring(str).c_str();
}

#define DIM(x) (sizeof(x)/sizeof(*(x)))

static const char     *sizes[]   = { "EB", "PB", "TB", "GB", "MB", "kB", "B" };
static const char     *bitSizes[]   = { "Eb", "Pb", "Tb", "Gb", "Mb", "kb", "b" };
static const unsigned long long  exbibytes = 1024ULL * 1024ULL * 1024ULL *
                                   1024ULL * 1024ULL * 1024ULL;

std::wstring CUtils::calculateSize(unsigned long long size)
{   
	std::wstringstream ss;
	std::wstring result = _T("0");
	ss << std::fixed << std::setprecision(3);

    unsigned long long  multiplier = exbibytes;
    int i;

    for (i = 0; i < DIM(sizes); i++, multiplier /= 1024)
    {   
        if (size < multiplier)
            continue;

        if (size % multiplier == 0)
			ss << size / multiplier << " " << sizes[i];
        else
			ss << (float) size/ multiplier << " " << sizes[i];	
		result = ss.str();
        return result;
    }
    return result;
}

std::wstring CUtils::calculateBitSize(unsigned long long size)
{   
	size = size*8;
	std::wstringstream ss;
	std::wstring result = _T("0");
	ss << std::fixed << std::setprecision(3);

    unsigned long long  multiplier = exbibytes;
    int i;

    for (i = 0; i < DIM(sizes); i++, multiplier /= 1024)
    {   
        if (size < multiplier)
            continue;

        if (size % multiplier == 0)
			ss << size / multiplier << " " << bitSizes[i];
        else
			ss << (float) size/ multiplier << " " << bitSizes[i];	
		result = ss.str();
        return result;
    }
    return result;
}