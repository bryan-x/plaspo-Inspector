#include "stdafx.h"
#include "NSCanManager.h"
#include "NSCan.h"
#include "NSCanDump.h"
#include "NSChiefManager.h"
#include "NSPcsManager.h"
#include "NSLog.h"
#include "OutputWnd.h"
#ifdef __DISCONNECT_MODULE
#include "NSMcManager.h"
#endif



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CNSCanObject::CNSCanObject()
{
	InitializeCriticalSection(&_cs);
	_idx = 0;
	_latest = 0;
}

CNSCanObject::~CNSCanObject()
{
	DeleteCriticalSection(&_cs);
}

void CNSCanObject::SetMessage(CanMessage* m)
{
	EnterCriticalSection(&_cs);
	memcpy(&_m[_idx], m, sizeof(CanMessage));
	//TRACE(_T("Message%d Slot%d Set\n"), m->id, _idx);
	LeaveCriticalSection(&_cs);
	_latest = _idx;
	_idx = (++_idx) % 2;
}

CanMessage* CNSCanObject::GetMessage()
{
	CanMessage* m = NULL;
	EnterCriticalSection(&_cs);
	m = &_m[_latest];
	//TRACE(_T("--->Message%d Slot%d Get\n"), m->id, _latest);
	LeaveCriticalSection(&_cs);
	return m;
}

CNSCanManager::CNSCanManager(CNSChiefManager* chief)
{
	_pChief = chief;
	_pcs1 = NULL;
	_pcs2 = NULL;
	_can = NULL;
	_dump = NULL;
	_log = NULL;

	_bLog = 1;
	_logFilter = 0xff;

	_msgCnt[0] = 0;
	_msgCnt[1] = 0;
	_msgCnt[2] = 0;

	InitializeCriticalSection(&_cs);
	InitializeCriticalSection(&_cs_pcs1);
	InitializeCriticalSection(&_cs_pcs2);
	InitializeCriticalSection(&_cs_mc);
}

CNSCanManager::~CNSCanManager()
{
	DeleteCriticalSection(&_cs);
	DeleteCriticalSection(&_cs_pcs1);
	DeleteCriticalSection(&_cs_pcs2);
	DeleteCriticalSection(&_cs_mc);
}

// 0 : OK
ns_uint32 CNSCanManager::Create(CNSPcsManager* pcs1, CNSPcsManager* pcs2)
{
	ns_uint32 ret = 0;

	_pcs1 = pcs1;
	_pcs2 = pcs2;

	_can = CreateCanEngine();
	if (_can) {
		ret = _can->Create(this);
		if (NS_OK != ret) {
			Log(ERROR_IPC, _T("Init CAN Engine Objcet Fail!!"));
			return 1;
		}
	} else {
		Log(ERROR_MEMORY, _T("Create CAN Engine Object Fail!!"));
		return 1;
	}

	_dump = CreateCanDump();
	if (_dump) {
		ret = _dump->Create();
		if (NS_OK != ret) {
			Log(ERROR_IPC, _T("Init Dump Objcet Fail!!"));
		}
	} else {
		Log(ERROR_MEMORY, _T("Create CAN Message Dump Object Fail!!"));
	}

	_log = CreateLogObject();
	if (NULL == _log) {
		Log(ERROR_MEMORY, _T("Create Log Object Fail!!"));
	}

	//ret = map_init();

	return ret;
}

CNSCanEngine *CNSCanManager::CreateCanEngine()
{
	CNSCanEngine *o = NULL;
	try {
		o = new CNSCanEngine();
	} catch (...) {
		return NULL;
	}

	return o;
}

CNSCanDump *CNSCanManager::CreateCanDump()
{
	CNSCanDump *o = NULL;
	try {
		o = new CNSCanDump();
	} catch (...) {
		return NULL;
	}

	return o;
}

CNSCanObject* CNSCanManager::CreateCanObject()
{
	CNSCanObject* ms = NULL;
	try {
		ms = new CNSCanObject();
	} catch (...) {
		return NULL;
	}
	return ms;
}

ns_uint32 CNSCanManager::Destroy()
{
	ns_uint32 ret = 0;

	if (NULL != _can) {
		_can->Destroy();
		delete _can;
		_can = NULL;
	}

	if (NULL != _dump) {
		_dump->Destroy();
		delete _dump;
		_dump = NULL;
	}

	if (NULL != _log) {
		delete _log;
		_log = NULL;
	}
	
	map_clear();

	return ret;
}

ns_uint32 CNSCanManager::Start()
{
	ns_uint32 ret = 0;
	ns_uint8 status = 0;
	if( _can ) {
		status = _can->GetStatus();
		if( status != CAN_STATE_START ) {
			map_init();
			ResetMsgCnt(MI_DPM);
			ResetMsgCnt(MI_PCS1);
			ResetMsgCnt(MI_PCS2);
			_can->Start();
		} else {
			ret = 1;
		}
	}
	return  ret;
}

ns_uint32 CNSCanManager::Stop()
{
	ns_uint32 ret = 0;
	ns_uint8 status = 0;
	if (_can) {
		status = _can->GetStatus();
		if( status != CAN_STATE_STOP ) {
			_can->Stop();
			map_clear();
			_dump->Stop();
		}
	}
	return ret;
}

ns_uint32 CNSCanManager::map_init()
{
	ns_uint32 ret = 0, i = 0;
	CanMessage m = {0};
	CNSCanObject* o = NULL;

	for( i = 0; i < CID_MAX; i++ ) {
		if( i >= CID_PCS_1 && i <= CID_PCS_12_FAULT ) {
			o = CreateCanObject();
			if( o ) {
				m.id = i;
				o->SetMessage( &m );
				_pcs1Map[i] = o;
			}
		}

		if( i >= CID_PCS_21 && i <= CID_PCS_32_FAULT ) {
			o = CreateCanObject();
			if( o ) {
				m.id = i;
				o->SetMessage( &m );
				_pcs2Map[i] = o;
			}		
		}		

		if( i >= CID_DPM_61 && i <= CID_EMS ) {
			o = CreateCanObject();
			if( o ) {
				m.id = i;
				o->SetMessage( &m );
				_mcMap[i] = o;
			}			
		}
	}
	
	return ret;
}

// 0:OK
ns_uint32 CNSCanManager::map_write(CanMessage *m)
{
	ns_uint32 ret = 0;
	CNSCanObject* ms = NULL;
	std::map<ns_uint32, CNSCanObject*>::iterator it;

#ifdef __CAN_DUMP
	if (_dump->Dump(m) != NS_OK) {
		Log(ERROR_EXCEPTION, _T("Message Dump Fail!!"));
	}
#endif

	if (CID_PCS1_TRACE == m->id) {
		_pcs1->TraceDump(m->data);
		IncreaseMsgCnt(MI_PCS1);
		return 0;
	}

	if (CID_PCS2_TRACE == m->id) {
		_pcs2->TraceDump(m->data);
		IncreaseMsgCnt(MI_PCS2);
		return 0;
	}

	if ((CID_PCS_1 <= m->id) && (CID_PCS_12_FAULT >= m->id)) {
		EnterCriticalSection(&_cs_pcs1);
		it = _pcs1Map.find(m->id);
		if( it != _pcs1Map.end() && it->second ) {
			((CNSCanObject *)it->second)->SetMessage(m);
		}
		LeaveCriticalSection(&_cs_pcs1);
		IncreaseMsgCnt(MI_PCS1);
		return 0;
	}
	
	if ((CID_PCS_21 <= m->id) && (CID_PCS_32_FAULT >= m->id)) {
		EnterCriticalSection(&_cs_pcs2);
		it = _pcs2Map.find(m->id);
		if( it != _pcs2Map.end() && it->second ) {
			((CNSCanObject *)it->second)->SetMessage(m);
		}
		LeaveCriticalSection(&_cs_pcs2);
		IncreaseMsgCnt(MI_PCS2);
		return 0;
	}
	
	if ((CID_DPM_61 <= m->id) && (CID_EMS >= m->id)) {
		EnterCriticalSection(&_cs_mc);
		it = _mcMap.find(m->id);
		if( it != _mcMap.end() && it->second ) {
			((CNSCanObject *)it->second)->SetMessage(m);
		}
		LeaveCriticalSection(&_cs_mc);
		IncreaseMsgCnt(MI_DPM);
		return 0;
	}
	
	Log(SERVICE_ETC, _T("Undefined ID"));
	return 1;
}

ns_uint32 CNSCanManager::map_read_pcs1(ns_uint32 id, CanMessage* out)
{
	ns_uint32 ret = 1;
	CNSCanObject *o = NULL;
	std::map<ns_uint32, CNSCanObject*>::iterator it;
	EnterCriticalSection(&_cs_pcs1);
	it = _pcs1Map.find(id);
	if( it != _pcs1Map.end() && it->second ) {
		o = (CNSCanObject *)it->second;
	}
	LeaveCriticalSection(&_cs_pcs1);
	
	if( o ) {
		memcpy(out, o->GetMessage(), sizeof(CanMessage));
		ret = 0;
	}
	return ret;
}

ns_uint32 CNSCanManager::map_read_pcs2(ns_uint32 id, CanMessage* out)
{
	ns_uint32 ret = 1;
	CNSCanObject *o = NULL;
	std::map<ns_uint32, CNSCanObject*>::iterator it;
	EnterCriticalSection(&_cs_pcs2);
	it = _pcs2Map.find(id);
	if( it != _pcs2Map.end() && it->second ) {
		o = (CNSCanObject *)it->second;
	}
	LeaveCriticalSection(&_cs_pcs2);
	
	if( o ) {
		memcpy(out, o->GetMessage(), sizeof(CanMessage));
		ret = 0;
	}
	return ret;
}

ns_uint32 CNSCanManager::map_read_mc(ns_uint32 id, CanMessage* out)
{
	ns_uint32 ret = 1;
	CNSCanObject *o = NULL;
	std::map<ns_uint32, CNSCanObject*>::iterator it;
	EnterCriticalSection(&_cs_mc);
	it = _mcMap.find(id);
	if( it != _mcMap.end() && it->second ) {
		o = (CNSCanObject *)it->second;
	}
	LeaveCriticalSection(&_cs_mc);
	if( o ) {
		memcpy(out, o->GetMessage(), sizeof(CanMessage));
		ret = 0;
	}
	return ret;
}

void CNSCanManager::map_clear()
{
	std::map<ns_uint32, CNSCanObject*>::iterator it;
	for (it = _pcs1Map.begin(); it != _pcs1Map.end(); it++) {
		if (it->second) {
			delete (it->second);
		}
	}
	_pcs1Map.clear();

	for (it = _pcs2Map.begin(); it != _pcs2Map.end(); it++) {
		if (it->second) {
			delete (it->second);
		}
	}
	_pcs2Map.clear();

	for (it = _mcMap.begin(); it != _mcMap.end(); it++) {
		if (it->second) {
			delete (it->second);
		}
	}
	_mcMap.clear();
}

CNSLog* CNSCanManager::CreateLogObject()
{
	CNSLog *o = NULL;
	try {
		o = new CNSLog(_T("CAN"));
	}
	catch (...) {
		return NULL;
	}
	return o;
}

ns_uint32 CNSCanManager::Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...)
{
	HWND hWnd = NULL;
	TCHAR buf[1024] = { 0, };
	CString strLog = _T("");
	va_list va;

	time_t t;
	struct tm c;

	ASSERT(lpszLog);

	t = time(NULL);
	localtime_s(&c, &t);

	va_start(va, lpszLog);
	_vsntprintf_s(buf, _countof(buf), lpszLog, va);
	va_end(va);

	strLog.Format(_T("[%02d:%02d:%02d] %s\r\n"), c.tm_hour, c.tm_min, c.tm_sec, buf);

	_log->WriteLog(strLog);

	if (_pChief) {
		hWnd = _pChief->GetCanLogBoxHandle();
		if (hWnd && _bLog && (_logFilter & filterFlag))
			SendMessage(hWnd, WM_LOGBOX_UPDATE, NULL, (LPARAM)&strLog);
	}
	return 0;
}

void CNSCanManager::FilterLog(ns_uint8 bShow, ns_uint8 flag)
{
	_bLog = bShow;
	_logFilter = flag;
}

void CNSCanManager::GetMsgCount(ns_uint16* arr)
{
	ns_uint8 i = 0;
	for (i = 0; i < 3; i++)
		arr[i] = GetMsgCntByIndex(i);
}

void CNSCanManager::IncreaseMsgCnt(ns_uint8 module)
{
	EnterCriticalSection(&_cs);
	_msgCnt[module]++;
	LeaveCriticalSection(&_cs);
}

void CNSCanManager::ResetMsgCnt(ns_uint8 module)
{
	EnterCriticalSection(&_cs);
	_msgCnt[module] = 0;
	LeaveCriticalSection(&_cs);
}

ns_uint16 CNSCanManager::GetMsgCntByIndex(ns_uint8 module)
{
	ns_uint16 cnt = 0;
	EnterCriticalSection(&_cs);
	cnt = _msgCnt[module];
	_msgCnt[module] = 0;
	LeaveCriticalSection(&_cs);
	return cnt;
}


#ifdef __DISCONNECT_MODULE
void CNSCanManager::ClearPcsList(ns_uint8 instanceID)
{
	HWND hWnd = NULL;
	if (_pChief) {
		hWnd = _pChief->GetPcsListHandle();
		if (hWnd) {
			if (instanceID == IID_PCS1) {
				SendMessage(hWnd, WM_PCS1_LIST_CLEAR, PCS1, 0);
			}
			else if (instanceID == IID_PCS2) {
				SendMessage(hWnd, WM_PCS1_LIST_CLEAR, PCS2, 0);
			}
		}
	}
}

void CNSCanManager::ClearFaultList()
{
}

void CNSCanManager::ClearDpmList()
{
	HWND hWnd = NULL;
	if (_pChief) {
		hWnd = _pChief->GetDpmListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_DPMLIST_CLEAR, 0, 0);
	}
}

void CNSCanManager::ClearCsList()
{
	HWND hWnd = NULL;
	if (_pChief) {
		hWnd = _pChief->GetCsListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_CSLIST_CLEAR, 0, 0);
	}
}

#endif


#if 0
/**
* @reamarks
*	CAN 리스트 컨트롤 고정 문자열 초기화 함수
* @author 김동현
* @date 2016/06/09
* @return	0:성공, 1:실패
*/
ns_uint32 CNSCanManager::InitListCtrl()
{
	ns_uint32 ret = 0;
	CString strID = _T("");
	HFONT hFont;
	ns_uint id = 0;

	LISTCTRLHEADER CANListHeader[] =
	{
		LISTCTRLHEADER(_T("ID"),			13,	LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Time Stamp"),	27, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("DLC"),			10, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Data"),			45, LVCFMT_CENTER),
	};

	if (NULL == _list) {
		TRACE(_T("CAN InitListCtrl() - Failed \n"));
		return 1;
	}

	hFont = CreateFont(
		20,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		0,
		ANSI_CHARSET,
		OUT_STRING_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS,
		_T("맑은 고딕"));

	_list->SendMessage(WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	_list->InitListCtrlHeader(sizeof(CANListHeader) / sizeof(LISTCTRLHEADER), CANListHeader);

	return ret;
}

/**
* @reamarks
*	CAN ListCtrl 항목별 표시 함수
* @author 김동현
* @date 2016/06/17
* @param CanMessage* msg	CAN 메시지
* @return	0:성공, 1:실패
*/
ns_uint32 CNSCanManager::ShowData(CanMessage* m)
{
	ns_uint32 ret = 0;
	ns_uint16 idx = 0;
	CanMessage* m2 = NULL;

	if (m == NULL || _list == NULL) {
		TRACE(_T("CAN ShowData() - Failed() \n"));
		return 1;
	}
	return ret;
}
#endif

