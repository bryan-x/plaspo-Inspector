#include "stdafx.h"
#include "NSMcManager.h"
#include "NSChiefManager.h"
#include "NSCanManager.h"
#include "OutputWnd.h"
#include "NSLog.h"


/**
* @reamarks
*	MC 매니저 클래스 생성자
* @author 김동현
* @date 2016/06/20
*/
CNSMcManager::CNSMcManager(CNSChiefManager* chief)
{
	_pChief = chief;

	_hThread = INVALID_HANDLE_VALUE;
	_hStopEvent = INVALID_HANDLE_VALUE;

	_bEndThread = 0;

	_log = NULL;

	ZeroMemory(_dpmData, sizeof(_dpmData));
	ZeroMemory(_csData, sizeof(_csData));
	_csData[CS_STATUS] = 0xffff;

	_bLog = 1;
	_logFilter = 0xff;
}

/**
* @reamarks
*	MC 매니저 클래스 소멸자
* @author 김동현
* @date 2016/06/20
*/
CNSMcManager::~CNSMcManager()
{
}

/**
* @reamarks
*	MC 매니저 생성 함수
* @author 김동현
* @date 2016/06/20
* @param CNSCanManager*	pCanMgr	CAN 매니저 객체 포인터
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSMcManager::Create()
{
	ns_uint32 ret = 0;

	_hStopEvent = CreateEvent(NULL, TRUE, FALSE, _T("NSMcEvent"));
	if (INVALID_HANDLE_VALUE == _hStopEvent) {
		Log(ERROR_IPC, _T("Create Thread Stop Event Fail!!"));
		return 1;
	}

	_log = CreateLogObject(_T("MC"));
	if (NULL == _log)
		Log(ERROR_MEMORY, _T("Create Log Object Fail!!"));

	return ret;
}

/**
* @reamarks
*	MC 매니저 소멸 함수
* @author 김동현
* @date 2016/06/20
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSMcManager::Destroy()
{
	ns_uint32 ret = 0;

	Stop();

	if (INVALID_HANDLE_VALUE != _hStopEvent) {
		CloseHandle(_hStopEvent);
		_hStopEvent = INVALID_HANDLE_VALUE;
	}

	if (_log) {
		delete _log;
		_log = NULL;
	}

	return ret;
}

/**
* @reamarks
*	MC 매니저 (재)시작 인터페이스 함수
* @author 김동현
* @date 2016/06/23
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSMcManager::Start()
{
	if (INVALID_HANDLE_VALUE == _hThread) {
		_hThread = (HANDLE)_beginthreadex(NULL, 0, MCThreadProc, this, 0, NULL);
		if (INVALID_HANDLE_VALUE == _hThread) {
			Log(ERROR_IPC, _T("Start Fail!!"));
			return 1;
		}
		Log(SERVICE_ETC, _T("Start"));
	}

#if 1
	ShowDpmData(DPM_POWER, _dpmData[DPM_POWER]);
	ShowDpmData(DPM_VOLTAGE, _dpmData[DPM_VOLTAGE]);
	ShowDpmData(DPM_CURRENT, _dpmData[DPM_CURRENT]);

	ShowCsData(CS_STATUS, _csData[CS_STATUS]);
	ShowCsData(CS_PRESSURE1, _csData[CS_PRESSURE1]);
	ShowCsData(CS_PRESSURE2, _csData[CS_PRESSURE2]);
	ShowCsData(CS_TEMPERATURE1, _csData[CS_TEMPERATURE1]);
	ShowCsData(CS_TEMPERATURE2, _csData[CS_TEMPERATURE2]);
	ShowCsData(CS_TEMPERATURE3, _csData[CS_TEMPERATURE3]);
	ShowCsData(CS_FLOWMETER1, _csData[CS_FLOWMETER1]);
	ShowCsData(CS_FLOWMETER2, _csData[CS_FLOWMETER2]);
#endif

	return 0;
}

/**
* @reamarks
*	MC 매니저 정지 인터페이스 함수
* @author 김동현
* @date 2016/06/23
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSMcManager::Stop()
{
	ns_uint32 ret = 0;
	DWORD dwExitCode = 0;

	if (INVALID_HANDLE_VALUE != _hThread) {
		_bEndThread = 1;
		SetEvent(_hStopEvent);
		WaitForSingleObject(_hThread, THREAD_TIMEOUT_WAIT);
		GetExitCodeThread(_hThread, &dwExitCode);

		if (STILL_ACTIVE == dwExitCode){ 
			TerminateThread(_hThread, 0);
			TRACE(_T("MC Thread -  terminated...\n"));
		}

		CloseHandle(_hThread);
		_hThread = INVALID_HANDLE_VALUE;
		ResetEvent(_hStopEvent);
		Log(SERVICE_ETC, _T("Stop"));
	}
	return 0;
}

/**
* @reamkars
*	MC 매니저 쓰레드 함수
* @author 김동현
* @date 2016/06/15
*/
unsigned int __stdcall CNSMcManager::MCThreadProc(void* param)
{
	ns_uint32 ret = 0;
	CNSMcManager* o = (CNSMcManager*)param;
	ret = o->Receive();

	return ret;
}

/**
* @reamarks
*	CAN 데이터 수신 함수
* @author 김동현
* @date 2016/06/16
* @reteurn	0: 성공
*/
ns_uint32 CNSMcManager::Receive()
{
	ns_uint32 ret = 0;
	CNSCanManager* _pCan = NULL;
	CanMessage m = {0};
	DWORD dwResult = 0;
	int i;

	_pCan = _pChief->GetCanManager();
	_bEndThread = 0;

	while (!_bEndThread) {
		dwResult = WaitForSingleObject(_hStopEvent, _UI_UPDATE_INTERVAL);
		if (WAIT_OBJECT_0 == dwResult) {
			_bEndThread = 1;
		}
		else if (WAIT_TIMEOUT == dwResult && _pCan) {
			for (i = CID_DPM_61; i <= CID_EMS; i++) {
				if(_pCan->map_read_mc(i, &m) == NS_OK)
					Parse(m);
			}
		}
	}

	return ret;
}

ns_uint32 CNSMcManager::Parse(const CanMessage& m)
{
	ns_uint32 ret = 0;
	ns_uint16 temp16 = 0;
	
	switch (m.id) {
	case CID_DPM_61:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_dpmData[DPM_POWER] != temp16) {
			_dpmData[DPM_POWER] = temp16;
			ShowDpmData(DPM_POWER, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_dpmData[DPM_VOLTAGE] != temp16) {
			_dpmData[DPM_VOLTAGE] = temp16;
			ShowDpmData(DPM_VOLTAGE, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_dpmData[DPM_CURRENT] != temp16) {
			_dpmData[DPM_CURRENT] = temp16;
			ShowDpmData(DPM_CURRENT, temp16);
		}
		break;

	case CID_COOL_62:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_csData[CS_STATUS] != temp16) {
			_csData[CS_STATUS] = temp16;
			ShowCsData(CS_STATUS, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_csData[CS_PRESSURE1] != temp16) {
			_csData[CS_PRESSURE1] = temp16;
			ShowCsData(CS_PRESSURE1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_csData[CS_PRESSURE2] != temp16) {
			_csData[CS_PRESSURE2] = temp16;
			ShowCsData(CS_PRESSURE2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_csData[CS_TEMPERATURE1] != temp16) {
			_csData[CS_TEMPERATURE1] = temp16;
			ShowCsData(CS_TEMPERATURE1, temp16);
		}
		break;

	case CID_COOL_63:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_csData[CS_TEMPERATURE2] != temp16) {
			_csData[CS_TEMPERATURE2] = temp16;
			ShowCsData(CS_TEMPERATURE2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_csData[CS_TEMPERATURE3] != temp16) {
			_csData[CS_TEMPERATURE3] = temp16;
			ShowCsData(CS_TEMPERATURE3, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_csData[CS_FLOWMETER1] != temp16) {
			_csData[CS_FLOWMETER1] = temp16;
			ShowCsData(CS_FLOWMETER1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_csData[CS_FLOWMETER2] != temp16) {
			_csData[CS_FLOWMETER2] = temp16;
			ShowCsData(CS_FLOWMETER2, temp16);
		}
		break;

	case CID_PCS_CONTROL:
		break;

	case CID_EMS:
		break;

	default:
		break;
	}
	return ret;
}

/**
* @reamarks
*	DPM 리스트 컨트롤 항목별 위치와 단위를 결정하여 표시하는 함수
* @author 김동현
* @date 2016/06/14
* @param int parameter		리스트 컨트롤 아이템 항목
* @param ns_uint16 value	리스트 컨트롤 아이템 데이터
* @return	0:성공, 1:실패
*/
void CNSMcManager::ShowDpmData(ns_uint param, ns_uint16 value)
{
	HWND hWnd = NULL;
	CString strVaule = _T("");
	switch (param) {
	case DPM_POWER:	strVaule.Format(_T("%.1f kW"), value * 0.1f); break;
	case DPM_VOLTAGE: strVaule.Format(_T("%.1f V"), value * 0.1f); break;
	case DPM_CURRENT: strVaule.Format(_T("%.1f A"), value * 0.1f); break;
	}

	if (_pChief) {
		hWnd = _pChief->GetDpmListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_DPMLIST_UPDATE, (DPM_VALUE << 8) + param, (LPARAM)&strVaule);
	}
}

/**
* @reamarks
*	Coolint System 리스트 컨트롤 항목별 위치와 단위를 결정하여 표시하는 함수
* @author 김동현
* @date 2016/06/14
* @param int parameter		리스트 컨트롤 아이템 항목
* @param ns_uint16 value	리스트 컨트롤 아이템 데이터
* @return	0:성공, 1:실패
*/
void CNSMcManager::ShowCsData(ns_uint param, ns_uint16 value)
{
	HWND hWnd = NULL;
	CString strVaule = _T("");

	switch (param) {
	case CS_STATUS:
		strVaule = CheckCsStatus(value);
		break;
	case CS_PRESSURE1:
	case CS_PRESSURE2:
		strVaule.Format(_T("%.2f bar"), value * 0.01f);
		break;
	case CS_TEMPERATURE1:
	case CS_TEMPERATURE2:
	case CS_TEMPERATURE3:
		//strVaule.Format(_T("%.1f ℃"), (ns_int16)value * 0.1f);
		strVaule.Format(_T("%.1f C"), (ns_int16)value * 0.1f);
		break;
	case CS_FLOWMETER1:
	case CS_FLOWMETER2:
		strVaule.Format(_T("%.1f m^3/h"), value * 0.1f);
		break;
	}

	if (_pChief) {
		hWnd = _pChief->GetCsListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_CSLIST_UPDATE, (CS_VALUE << 8) + param, (LPARAM)&strVaule);
	}
}

/**
* @reamarks
*	Cooling System 상태 구분 함수
* @author 김동현
* @date 2016/06/14
* @param ns_uint16 value	리스트 컨트롤 아이템 데이터
*/
CString CNSMcManager::CheckCsStatus(ns_uint16 value)
{
	CString strStatus = _T("");

	if (value > 0x07)
		strStatus = _T("INVALID");
	else {
		if ((value & 0x01) == 0)
			strStatus = _T("STOP");
		else if (value & 0x02)
			strStatus = _T("FAULT");
		else if (value & 0x04)
			strStatus = _T("RUN*");
		else
			strStatus = _T("RUN");
	}

	return strStatus;
}

CNSLog* CNSMcManager::CreateLogObject(LPCTSTR module)
{
	CNSLog *o = NULL;
	try {
		o = new CNSLog(module);
	}
	catch (...) {
		return NULL;
	}
	return o;
}

ns_uint32 CNSMcManager::Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...)
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
		hWnd = _pChief->GetMcLogBoxHandle();
		if (hWnd && _bLog && (_logFilter & filterFlag))
			SendMessage(hWnd, WM_LOGBOX_UPDATE, NULL, (LPARAM)&strLog);
	}
	
	return 0;
}

void CNSMcManager::FilterLog(ns_uint8 bShow, ns_uint8 flag)
{
	_bLog = bShow;
	_logFilter = flag;
}