
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

//#pragma comment(linker,"/ignore:4089")	// all references to "xxx.dll" discarded by /OPT:REF

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif


#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions




#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


#include <afxsock.h>            // MFC socket extensions

//#include ".\XListCtrl\XTrace.h"

///////////////////////////////////////////////////////////////////////////////
// uncomment the following line to enable TRACE statements within the 
// XListCtrl library
#define ENABLE_XLISTCTRL_TRACE

#ifdef ENABLE_XLISTCTRL_TRACE
	#define XLISTCTRL_TRACE TRACE
#else
	#define XLISTCTRL_TRACE __noop
#endif

/////////////////////////////////////////////////////////////////////////////////

typedef unsigned char ns_uint8;
typedef unsigned short ns_uint16;
typedef unsigned long ns_uint32;
typedef unsigned int ns_uint;
typedef unsigned long long ns_uint64;

typedef float ns_float;
typedef double ns_double;

typedef signed char ns_int8;
typedef signed short ns_int16;
typedef signed long ns_int32;
typedef signed int ns_int;
typedef long long ns_int64;

typedef unsigned short ns_wchar;
typedef unsigned char ns_bool;

const ns_uint8 NS_TRUE = 1;
const ns_uint8 NS_FALSE = 0;

#define NS_OK			0
#define NS_ERR			1
#define NS_ASSERT		2
#define NS_NULL			3

/////////////////////////////////////////////////////////////////////////////////
//#define _CAN_SIM_
#define THREAD_TIMEOUT_WAIT		1050
#define _UI_UPDATE_INTERVAL		500		// ms

//#define CAN_ENGINE_MESSAGE_TRACE
//#define __PARSE_FAULT
#define __DISCONNECT_MODULE


/////////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "./can/canlib32.lib") 


/////////////////////////////////////////////////////////////////////////////////

#define MAKEUINT16(a,b)			(ns_uint16)(((a & 0xff) << 8) | (b & 0xff))
#define MAKEUINT32(a,b)			(ns_uint32)(((a & 0xffff) << 16) | (b & 0xffff))

/////////////////////////////////////////////////////////////////////////////////

char * string2char(CString str);


////////////////////////////////////////////////////////////////////////////////////////

enum CUSTOM_MESSAGE {
	MSGID_LOG = 100,
	MSGID_PCS_UPDATE,
	MSGID_FAULT_UPDATE,
	MSGID_DPM_UPDATE,
	MSGID_CS_UPDATE,
	//MSGID_CAN_UPDATE,
#ifdef __DISCONNECT_MODULE
	MSGID_PCS1_CLEAR,
	MSGID_PCS2_CLEAR,
	MSGID_FAULT_CLEAR,
	MSGID_DPM_CLEAR,
	MSGID_CS_CLEAR,
#endif
};


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


