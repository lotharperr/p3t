
// stdafx.h: Includedatei f�r Standardsystem-Includedateien
// oder h�ufig verwendete, projektspezifische Includedateien,
// die nur selten ge�ndert werden.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Selten verwendete Teile der Windows-Header ausschlie�en
#endif

#define _VARIADIC_MAX 10

#ifdef _DEBUG
#pragma comment (lib, "Deps\\ChartCtrl\\ChartCtrlLibD2012.lib")
#else
#pragma comment (lib, "Deps\\ChartCtrl\\ChartCtrlLib2012.lib")
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // einige CString-Konstruktoren sind explizit

// Deaktiviert das Ausblenden einiger h�ufiger und oft ignorierter Warnungen durch MFC
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC-Kern- und -Standardkomponenten
#include <afxext.h>         // MFC-Erweiterungen


#include <afxdisp.h>        // MFC-Automatisierungsklassen



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC-Unterst�tzung f�r allgemeine Steuerelemente von Internet Explorer 4
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC-Unterst�tzung f�r allgemeine Windows-Steuerelemente
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC-Unterst�tzung f�r Men�b�nder und Steuerleisten


#include <afxsock.h>            // MFC-Socket-Erweiterungen

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


