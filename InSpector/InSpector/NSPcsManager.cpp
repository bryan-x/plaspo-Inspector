#include "stdafx.h"
#include "NSPcsManager.h"
#include "NSChiefManager.h"
#include "NSPcsFaultLog.h"
#include "NSCanManager.h"
#include "OutputWnd.h"
#include "NSLog.h"


/**
* @reamarks
*	PCS 매니저 클래스 생성자
* @author 김동현
* @date 2016/07/11
* @param ns_uint instanceID		객체 식별자
*/
CNSPcsManager::CNSPcsManager(ns_uint8 instanceID, CNSChiefManager* chief)
	:_dump(instanceID)
{
	_instanceID = instanceID;

	_pChief = chief;
	_log = NULL;
	_hThread = INVALID_HANDLE_VALUE;
	_hStopEVent = INVALID_HANDLE_VALUE;
	_bEndThread = 0;

	_hTimeOutThread = INVALID_HANDLE_VALUE;
	_hTimeOutEvent = INVALID_HANDLE_VALUE;
	_bTimeout = 0;
	_bTraceError = 0;
	_traceNum = 0xff;
	_traceIdx = 0xff;
	ZeroMemory(_traceData, sizeof(_traceData));

	ZeroMemory(_pcsData, sizeof(_pcsData));
	_pcsData[STATUS] = 0x00ff;

	ZeroMemory(_pcsFault, sizeof(_pcsFault));

	_bLog = 1;
	_logFilter = 0xff;

	InitializeCriticalSection(&_cs);
}

/**
* @reamarks
*	PCS 매니저 클래스 소멸자
* @author 김동현
* @date 2016/07/11
*/
CNSPcsManager::~CNSPcsManager()
{
	DeleteCriticalSection(&_cs);
}

/**
* @reamarks
*	PCS 매니저 생성 함수
* @author 김동현
* @date 2016/07/11
* @param CNSCanManager*	pCanMgr	CAN 매니저 객체 포인터
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSPcsManager::Create()
{
	ns_uint32 ret = 0;
	CString strName = _T("");

	strName.Format(_T("NSPcs%dEvent"), _instanceID);

	_hStopEVent = CreateEvent(NULL, FALSE, FALSE, strName);
	if (INVALID_HANDLE_VALUE == _hStopEVent) {
		Log(ERROR_IPC, _T("Create Thread Stop Event Fail!!"));
		return 1;
	}

	_log = CreateLogObject();
	if (NULL == _log)
		Log(ERROR_MEMORY, _T("Create Log Object Fail!!"));

	return ret;
}

/**
* @reamarks
*	PCS 매니저 소멸 함수
* @author 김동현
* @date 2016/07/11
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSPcsManager::Destroy()
{
	ns_uint32 ret = 0;

	Stop();

	if (INVALID_HANDLE_VALUE != _hStopEVent) {
		CloseHandle(_hStopEVent);
		_hStopEVent = INVALID_HANDLE_VALUE;
	}

	if (NULL != _log) {
		delete _log;
		_log = NULL;
	}

	return ret;
}

/**
* @reamarks
*	PCS 매니저 (재)시작 인터페이스 함수
* @author 김동현
* @date 2016/07/11
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSPcsManager::Start()
{
	if (INVALID_HANDLE_VALUE == _hThread) {
		_hThread = (HANDLE)_beginthreadex(NULL, 0, PcsThreadProc, this, 0, NULL);
		if (INVALID_HANDLE_VALUE == _hThread) {
			Log(ERROR_IPC, _T("Start Fail!!"));
			return 1;
		}
		Log(SERVICE_ETC, _T("Start"));
	}

#if 1
	int header = 0;
	for(int i = STATUS; i<= BAT_CURRENT; i++) {
		header = (_instanceID == IID_PCS1)? PCS1 : PCS2;
		ShowData(i, header, _pcsData[i]);

	}
#endif

	return 0;
}

/**
* @reamarks
*	PCS 매니저 정지 인터페이스 함수
* @author 김동현
* @date 2016/07/11
* @return	0: 성공, 1: 실패
*/
ns_uint32 CNSPcsManager::Stop()
{
	ns_uint32 ret = 0;
	DWORD dwExitCode = 0;
	// 쓰레드 종료
	if (INVALID_HANDLE_VALUE != _hThread) {
		_bEndThread = 1;
		SetEvent(_hStopEVent);

		WaitForSingleObject(_hThread, THREAD_TIMEOUT_WAIT);
		GetExitCodeThread(_hThread, &dwExitCode);
		if (STILL_ACTIVE == dwExitCode) {
			TerminateThread(_hThread, 0);
			TRACE(_T("PCS%d Thread -  terminated...\n"), _instanceID);
		}

		CloseHandle(_hThread);
		_hThread = INVALID_HANDLE_VALUE;
		Log(SERVICE_ETC, _T("Stop"));
	}
	return 0;
}

/**
* @reamarks
*	PCS 매니저 쓰레드 함수
* @author 김동현
* @date 2016/07/11
*/
unsigned int __stdcall CNSPcsManager::PcsThreadProc(void* param)
{
	ns_uint32 ret = 0;
	CNSPcsManager* o = (CNSPcsManager*)param;
	ret = o->Receive();
	return ret;
}

/**
* @reamarks
*	CAN 데이터맵 참조 함수
* @author 김동현
* @date 2016/07/11
* @reteurn	0: 성공
* @see
*	식별자에 따라 데이터맵 참조 범위를 달리함
*/
ns_uint32 CNSPcsManager::Receive()
{
	ns_uint32 ret = 0;
	CNSCanManager* pCan = NULL;
	CanMessage m = {0};
	DWORD dwResult = 0;
	int i;

	pCan = _pChief->GetCanManager();
	_bEndThread = 0;

	while (!_bEndThread) {
		dwResult = WaitForSingleObject(_hStopEVent, _UI_UPDATE_INTERVAL);
		if (WAIT_OBJECT_0 == dwResult) {
			_bEndThread = 1;
		}
		else if (WAIT_TIMEOUT == dwResult && pCan) {
			if (IID_PCS1 == _instanceID) {
				for (i = CID_PCS_1; i <= CID_PCS_12_FAULT; i++) {
					if(pCan->map_read_pcs1(i, &m) == NS_OK)
						Parse(m);
				}
			}
			else if (IID_PCS2 == _instanceID) {
				for (i = CID_PCS_21; i <= CID_PCS_32_FAULT; i++) {
					if(pCan->map_read_pcs2(i, &m) == NS_OK)
						Parse(m);
				}
			}
			memset(&m, 0, sizeof(CanMessage)); 
		}
	}
	return ret;
}

ns_uint32 CNSPcsManager::Parse(const CanMessage& m)
{
	ns_uint32 ret = 0;
	ns_uint16 temp16 = 0;

	switch (m.id) {
	case CID_PCS_1:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[STATUS] != temp16) {
			_pcsData[STATUS] = temp16;
			ShowData(STATUS, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[ACTIVE_POWER] != temp16) {
			_pcsData[ACTIVE_POWER] = temp16;
			ShowData(ACTIVE_POWER, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[REACTIVE_POWER] != temp16) {
			_pcsData[REACTIVE_POWER] = temp16;
			ShowData(REACTIVE_POWER, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[FREQUENCY] != temp16) {
			_pcsData[FREQUENCY] = temp16;
			ShowData(FREQUENCY, PCS1, temp16);
		}
		break;

	case CID_PCS_2_VOL:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[DC_LINK_VOLTAGE] != temp16) {
			_pcsData[DC_LINK_VOLTAGE] = temp16;
			ShowData(DC_LINK_VOLTAGE, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[VOLTAGE_RS] != temp16) {
			_pcsData[VOLTAGE_RS] = temp16;
			ShowData(VOLTAGE_RS, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[VOLTAGE_ST] != temp16) {
			_pcsData[VOLTAGE_ST] = temp16;
			ShowData(VOLTAGE_ST, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[VOLTAGE_TR] != temp16) {
			_pcsData[VOLTAGE_TR] = temp16;
			ShowData(VOLTAGE_TR, PCS1, temp16);
		}
		break;

	case CID_PCS_3:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[CURRENT_R] != temp16) {
			_pcsData[CURRENT_R] = temp16;
			ShowData(CURRENT_R, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[CURRENT_S] != temp16) {
			_pcsData[CURRENT_S] = temp16;
			ShowData(CURRENT_S, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[CURRENT_T] != temp16) {
			_pcsData[CURRENT_T] = temp16;
			ShowData(CURRENT_T, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[POWER_FACTOR] != temp16) {
			_pcsData[POWER_FACTOR] = temp16;
			ShowData(POWER_FACTOR, PCS1, temp16);
		}
		break;

	case CID_PCS_4_TEMP:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[HS_TEMPERATURE_M1] != temp16) {
			_pcsData[HS_TEMPERATURE_M1] = temp16;
			ShowData(HS_TEMPERATURE_M1, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[HS_TEMPERATURE_M2] != temp16) {
			_pcsData[HS_TEMPERATURE_M2] = temp16;
			ShowData(HS_TEMPERATURE_M2, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[R_TEMPERATURE_M1] != temp16) {
			_pcsData[R_TEMPERATURE_M1] = temp16;
			ShowData(R_TEMPERATURE_M1, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[R_TEMPERATURE_M2] != temp16) {
			_pcsData[R_TEMPERATURE_M2] = temp16;
			ShowData(R_TEMPERATURE_M2, PCS1, temp16);
		}
		break;

	case CID_PCS_5_ENERGY:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[TODAY_CHARGE_ENERGY] != temp16) {
			_pcsData[TODAY_CHARGE_ENERGY] = temp16;
			ShowData(TODAY_CHARGE_ENERGY, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[TODAY_DISCHARGE_ENERGY] != temp16) {
			_pcsData[TODAY_DISCHARGE_ENERGY] = temp16;
			ShowData(TODAY_DISCHARGE_ENERGY, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[TOTAL_CHARGE_ENERGY] != temp16) {
			_pcsData[TOTAL_CHARGE_ENERGY] = temp16;
			ShowData(TOTAL_CHARGE_ENERGY, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[TOTAL_DISCHARGE_ENERGY] != temp16) {
			_pcsData[TOTAL_DISCHARGE_ENERGY] = temp16;
			ShowData(TOTAL_DISCHARGE_ENERGY, PCS1, temp16);
		}
		break;

	case CID_PCS_6_BAT:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[BAT_POWER] != temp16) {
			_pcsData[BAT_POWER] = temp16;
			ShowData(BAT_POWER, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[BAT_VOLTAGE] != temp16) {
			_pcsData[BAT_VOLTAGE] = temp16;
			ShowData(BAT_VOLTAGE, PCS1, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[BAT_CURRENT] != temp16) {
			_pcsData[BAT_CURRENT] = temp16;
			ShowData(BAT_CURRENT, PCS1, temp16);
		}
		break;

	case CID_PCS_7_BMS:
		break;

	case CID_PCS_8:
		break;

	case CID_PCS_9_M:
		break;

	case CID_PCS_10_M:
		break;

	case CID_PCS_11_FAULT:
	case CID_PCS_12_FAULT:
#ifdef __PARSE_FAULT
		ParseFault(m);
#endif
		break;

	case CID_PCS_21:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[STATUS] != temp16) {
			_pcsData[STATUS] = temp16;
			ShowData(STATUS, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[ACTIVE_POWER] != temp16) {
			_pcsData[ACTIVE_POWER] = temp16;
			ShowData(ACTIVE_POWER, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[REACTIVE_POWER] != temp16) {
			_pcsData[REACTIVE_POWER] = temp16;
			ShowData(REACTIVE_POWER, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[FREQUENCY] != temp16) {
			_pcsData[FREQUENCY] = temp16;
			ShowData(FREQUENCY, PCS2, temp16);
		}
		break;

	case CID_PCS_22_VOL:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[DC_LINK_VOLTAGE] != temp16) {
			_pcsData[DC_LINK_VOLTAGE] = temp16;
			ShowData(DC_LINK_VOLTAGE, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[VOLTAGE_RS] != temp16) {
			_pcsData[VOLTAGE_RS] = temp16;
			ShowData(VOLTAGE_RS, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[VOLTAGE_ST] != temp16) {
			_pcsData[VOLTAGE_ST] = temp16;
			ShowData(VOLTAGE_ST, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[VOLTAGE_TR] != temp16) {
			_pcsData[VOLTAGE_TR] = temp16;
			ShowData(VOLTAGE_TR, PCS2, temp16);
		}
		break;

	case CID_PCS_23:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[CURRENT_R] != temp16) {
			_pcsData[CURRENT_R] = temp16;
			ShowData(CURRENT_R, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[CURRENT_S] != temp16) {
			_pcsData[CURRENT_S] = temp16;
			ShowData(CURRENT_S, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[CURRENT_T] != temp16) {
			_pcsData[CURRENT_T] = temp16;
			ShowData(CURRENT_T, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[POWER_FACTOR] != temp16) {
			_pcsData[POWER_FACTOR] = temp16;
			ShowData(POWER_FACTOR, PCS2, temp16);
		}
		break;

	case CID_PCS_24_TEMP:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[HS_TEMPERATURE_M1] != temp16) {
			_pcsData[HS_TEMPERATURE_M1] = temp16;
			ShowData(HS_TEMPERATURE_M1, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[HS_TEMPERATURE_M2] != temp16) {
			_pcsData[HS_TEMPERATURE_M2] = temp16;
			ShowData(HS_TEMPERATURE_M2, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[R_TEMPERATURE_M1] != temp16) {
			_pcsData[R_TEMPERATURE_M1] = temp16;
			ShowData(R_TEMPERATURE_M1, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[R_TEMPERATURE_M2] != temp16) {
			_pcsData[R_TEMPERATURE_M2] = temp16;
			ShowData(R_TEMPERATURE_M2, PCS2, temp16);
		}
		break;

	case CID_PCS_25_ENERGY:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[TODAY_CHARGE_ENERGY] != temp16) {
			_pcsData[TODAY_CHARGE_ENERGY] = temp16;
			ShowData(TODAY_CHARGE_ENERGY, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[TODAY_DISCHARGE_ENERGY] != temp16) {
			_pcsData[TODAY_DISCHARGE_ENERGY] = temp16;
			ShowData(TODAY_DISCHARGE_ENERGY, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[TOTAL_CHARGE_ENERGY] != temp16) {
			_pcsData[TOTAL_CHARGE_ENERGY] = temp16;
			ShowData(TOTAL_CHARGE_ENERGY, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsData[TOTAL_DISCHARGE_ENERGY] != temp16) {
			_pcsData[TOTAL_DISCHARGE_ENERGY] = temp16;
			ShowData(TOTAL_DISCHARGE_ENERGY, PCS2, temp16);
		}
		break;

	case CID_PCS_26_BAT:
		temp16 = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsData[BAT_POWER] != temp16) {
			_pcsData[BAT_POWER] = temp16;
			ShowData(BAT_POWER, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsData[BAT_VOLTAGE] != temp16) {
			_pcsData[BAT_VOLTAGE] = temp16;
			ShowData(BAT_VOLTAGE, PCS2, temp16);
		}

		temp16 = MAKEUINT16(m.data[4], m.data[5]);
		if (_pcsData[BAT_CURRENT] != temp16) {
			_pcsData[BAT_CURRENT] = temp16;
			ShowData(BAT_CURRENT, PCS2, temp16);
		}
		break;

	case CID_PCS_27_BMS:
		break;

	case CID_PCS_28:
		break;

	case CID_PCS_29_M:
		break;

	case CID_PCS_30_M:
		break;

	case CID_PCS_31_FAULT:
	case CID_PCS_32_FAULT:
#ifdef __PARSE_FAULT
		ParseFault(m);
#endif
		break;

	case CID_PCS1_TRACE:
		break;

	case CID_PCS2_TRACE:
		break;

		//case CID_PCS_CONTROL: 
		//	break;

	default:
		break;
	}

	return ret;
}

/**
* @reamarks
*	Fault 데이터 파싱 함수
* @author 김동현
* @date 2016/07/22
* @param long id	CAN ID
* @param ns_uint8* data	CAN 데이터
* @return	0:성공
*/
ns_uint32 CNSPcsManager::ParseFault(const CanMessage& m)
{
	ns_uint32 ret = 0;
	CString strType = _T("");
	CString strFault = _T("");

	ns_uint16 faultVal = 0;
	ns_bool bUpdate = 0;

	switch (m.id) {
	case CID_PCS_11_FAULT:
		faultVal = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsFault[FAULT1_M1] != faultVal) {
			_pcsFault[FAULT1_M1] = faultVal;
			if (CheckFaultBit(FAULT1_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsFault[FAULT2_M1] != faultVal) {
			_pcsFault[FAULT2_M1] = faultVal;
			if (CheckFaultBit(FAULT2_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsFault[FAULT4_M1] != faultVal) {
			_pcsFault[FAULT4_M1] = faultVal;
			if (CheckFaultBit(FAULT4_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		if (bUpdate)
			ShowFault((ns_uint8)m.id, FAULT_PCS1, FAULT_M1, _pcsFault[FAULT1_M1], _pcsFault[FAULT2_M1], 0x00, _pcsFault[FAULT4_M1]);

		break;

	case CID_PCS_12_FAULT:
		faultVal = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsFault[FAULT1_M2] != faultVal) {
			_pcsFault[FAULT1_M2] = faultVal;
			if (CheckFaultBit(FAULT1_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsFault[FAULT2_M2] != faultVal) {
			_pcsFault[FAULT2_M2] = faultVal;
			if (CheckFaultBit(FAULT2_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsFault[FAULT4_M2] != faultVal) {
			_pcsFault[FAULT4_M2] = faultVal;
			if (CheckFaultBit(FAULT4_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		if (bUpdate)
			ShowFault((ns_uint8)m.id, FAULT_PCS1, FAULT_M2, _pcsFault[FAULT1_M2], _pcsFault[FAULT2_M2], 0x00, _pcsFault[FAULT4_M2]);

		break;

	case CID_PCS_31_FAULT:
		faultVal = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsFault[FAULT1_M1] != faultVal) {
			_pcsFault[FAULT1_M1] = faultVal;
			if (CheckFaultBit(FAULT1_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsFault[FAULT2_M1] != faultVal) {
			_pcsFault[FAULT2_M1] = faultVal;
			if (CheckFaultBit(FAULT2_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsFault[FAULT4_M1] != faultVal) {
			_pcsFault[FAULT4_M1] = faultVal;
			if (CheckFaultBit(FAULT4_M1, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		if (bUpdate)
			ShowFault((ns_uint8)m.id, FAULT_PCS2, FAULT_M1, _pcsFault[FAULT1_M1], _pcsFault[FAULT2_M1], 0x00, _pcsFault[FAULT4_M1]);

		break;

	case CID_PCS_32_FAULT:
		faultVal = MAKEUINT16(m.data[0], m.data[1]);
		if (_pcsFault[FAULT1_M2] != faultVal) {
			_pcsFault[FAULT1_M2] = faultVal;
			if (CheckFaultBit(FAULT1_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[2], m.data[3]);
		if (_pcsFault[FAULT2_M2] != faultVal) {
			_pcsFault[FAULT2_M2] = faultVal;
			if (CheckFaultBit(FAULT2_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		faultVal = MAKEUINT16(m.data[6], m.data[7]);
		if (_pcsFault[FAULT4_M2] != faultVal) {
			_pcsFault[FAULT4_M2] = faultVal;
			if (CheckFaultBit(FAULT4_M2, faultVal, strType, strFault) > 0) {
				SaveFault(strType, strFault);
				Log(SERVICE_FAULT, _T("%s Code:0x%02x"), strType, faultVal);
				bUpdate = 1;
			}
		}

		if (bUpdate)
			ShowFault((ns_uint8)m.id, FAULT_PCS2, FAULT_M2, _pcsFault[FAULT1_M2], _pcsFault[FAULT2_M2], 0x00, _pcsFault[FAULT4_M2]);

		break;
	}

	return ret;
}

/**
* @reamarks
*	Fault 데이터 파싱 함수
* @author 김동현
* @date 2016/07/22
* @param ns_uint typeInfo		Fault 타입
* @param ns_uint16 faultVal		Fault 데이터
* @param CString& strType		Fault 타입 문자열을 받을 참조 포인터
* @param CString& strFault		Fault 파일에 씌여질 문자열을 받을 참조 포인터
* @return	Fault 비트 개수
*/
ns_uint CNSPcsManager::CheckFaultBit(ns_uint typeInfo, ns_uint16 faultVal, CString& strType, CString& strFault)
{
	CString strTemp, strDesc = _T("");
	ns_uint i, count = 0;

	if (faultVal > 0) {
		switch (typeInfo) {
		case FAULT1_M1: strType = _T("Fault1 M1"); break;
		case FAULT2_M1: strType = _T("Fault2 M1"); break;
			//case FAULT3_M1: strType = _T("Fault3 M1"); break;
		case FAULT4_M1: strType = _T("Fault4 M1"); break;

		case FAULT1_M2: strType = _T("Fault1 M2"); break;
		case FAULT2_M2: strType = _T("Fault2 M2"); break;
			//case FAULT3_M2: strType = _T("Fault3 M2"); break;
		case FAULT4_M2: strType = _T("Fault4 M2"); break;
		}

		strFault = (_T("Error Num.  Error Message\r\n"));
		strFault += _T("--------------------------------------------------------------------------------\r\n");

		for (i = 0; i < 16; i++) {
			if (faultVal & (0x1 << i))
				strDesc = GetFaultDesc(typeInfo, i);

			if (strDesc.IsEmpty())
				continue;

			strTemp.Format(_T("b%d: %s / %s\r\n"), i, strType, strDesc);
			strFault += strTemp;
			count++;

			strDesc.Empty();
		}

		strFault += _T("--------------------------------------------------------------------------------\r\n");

		strTemp.Format(_T("이상 %d 개의 오류가 발생했습니다.\r\n"), count);
		strFault += strTemp;

		strTemp.Format(_T("Value = %d ( 0x%08x )\r\n"), faultVal, faultVal);

		strFault += strTemp;
	}

	return count;
}

/**
* @reamarks
*	Fault 로그 파일 생성
* @author 김동현
* @date 2016/08/17
* @param const CString& strType		Fault 타입
* @param const CString& strFault	Fault 내용
* @return	성공:0, 실패:1
*/
ns_uint32 CNSPcsManager::SaveFault(const CString& strType, const CString& strFault)
{
	CNSPcsFaultLog _fault(_instanceID);

	if (_fault.WriteLog(strType, strFault) == 0) {
		Log(ERROR_EXCEPTION, _T("Fault Log Fail!!"));
		return 1;
	}

	return 0;
}

/**
* @reamarks
*	Fault에 해당하는 내용을 반환
* @author 김동현
* @date 2016/07/22
* @param ns_uint typeInfo	Fault 타입
* @param ns_uint bit		비트 번호
* @return	Fault 내용 문자열
*/
CString CNSPcsManager::GetFaultDesc(ns_uint typeInfo, ns_uint bit)
{
	CString strDesc = _T("");

	switch (typeInfo) {
	case FAULT1_M1:
		switch (bit) {
		case 0: strDesc = _T("DC-link over voltage"); break;
		case 1: strDesc = _T("Battery under voltage"); break;
		case 2: strDesc = _T("Battery over voltage"); break;
		case 3: strDesc = _T("AC current offset error"); break;
		case 4: strDesc = _T("AC R over current"); break;
		case 5: strDesc = _T("AC S over current"); break;
		case 6: strDesc = _T("AC T over current"); break;
		case 7: strDesc = _T("DC over current"); break;
		case 8: strDesc = _T("AC current unbalance"); break;
		case 9: strDesc = _T("AC source sequence error"); break;
		case 10: strDesc = _T("AC source under voltage"); break;
		case 11: strDesc = _T("AC source over voltage"); break;
		case 12: strDesc = _T("AC source low frequency"); break;
		case 13: strDesc = _T("AC source high frequency"); break;
		case 14: strDesc = _T("DC charge fault 1"); break;
		case 15: strDesc = _T("AC charge fault 1"); break;
		}
		break;

	case FAULT2_M1:
		switch (bit) {
		case 0: strDesc = _T("DCCB 1 fault"); break;
		case 1: strDesc = _T("ACB 1 fault"); break;
		case 2: strDesc = _T("Circuit breaker 2 fault (AC filter capacitor over current)"); break;
		case 3: strDesc = _T("AC overload"); break;
		case 4: strDesc = _T("DCL over temperature (thermal SW)"); break;
		case 5: strDesc = _T("ACL over temperature (NTC)"); break;
		case 6: strDesc = _T("ACL over temperature (thermal SW)"); break;
		case 7: strDesc = _T("Heat-sink over temperature (NTC)"); break;
		case 8: strDesc = _T("Heat-sink over temperature (thermal SW)"); break;
		case 9: strDesc = _T("FAN 1 fault"); break;
		case 10: strDesc = _T("FAN 2 fault"); break;
		case 11: strDesc = _T("Door open"); break;
		case 12: strDesc = _T("Panel E-stop"); break;
		case 13: strDesc = _T("Water flow fault (flow switch off)"); break;
		case 14: strDesc = _T("Module unbalance (M1-M2 AC/DC/Power unbalnce)"); break;
		case 15: strDesc = _T("BMS fault"); break;
		}
		break;

		//case FAULT3_M1:
		//	break;

	case FAULT4_M1:
		switch (bit) {
		case 0: strDesc = _T("Battery over voltage (H/W)"); break;
		case 1: strDesc = _T("DC link over voltage (H/W)"); break;
		case 2: strDesc = _T("AC over current (H/W)"); break;
		case 3: strDesc = _T("DC over current (H/W)"); break;
		case 4: strDesc = _T("IGBT fault R TOP"); break;
		case 6: strDesc = _T("IGBT fault S TOP"); break;
		case 8: strDesc = _T("IGBT fault T TOP"); break;
		case 11: strDesc = _T("IGBT fault R BOTTOM"); break;
		case 13: strDesc = _T("IGBT fault S BOTTOM"); break;
		case 15: strDesc = _T("IGBT fault T BOTTOM"); break;
		}
		break;

	case FAULT1_M2:
		switch (bit) {
		case 0: strDesc = _T("DC-link over voltage"); break;
		case 1: strDesc = _T("Battery under voltage"); break;
		case 2: strDesc = _T("Battery over voltage"); break;
		case 3: strDesc = _T("AC current offset error"); break;
		case 4: strDesc = _T("AC R over current"); break;
		case 5: strDesc = _T("AC S over current"); break;
		case 6: strDesc = _T("AC T over current"); break;
		case 7: strDesc = _T("DC over current"); break;
		case 8: strDesc = _T("AC current unbalance"); break;
		case 14: strDesc = _T("DC charge fault 2"); break;
		case 15: strDesc = _T("AC charge fault 2"); break;
		}
		break;

	case FAULT2_M2:
		switch (bit) {
		case 0: strDesc = _T("DCCB 2 fault"); break;
		case 1: strDesc = _T("ACB 2 fault"); break;
		case 2: strDesc = _T("Circuit breaker 5 fault (AC filter capacitor over current)"); break;
		case 3: strDesc = _T("AC overload"); break;
		case 4: strDesc = _T("DCL 2 over temperature (thermal SW)"); break;
		case 5: strDesc = _T("ACL 2 over temperature (NTC)"); break;
		case 6: strDesc = _T("ACL 2 over temperature (thermal SW)"); break;
		case 7: strDesc = _T("Heat-sink 2 over temperature (NTC)"); break;
		case 8: strDesc = _T("Heat-sink 2 over temperature (thermal SW)"); break;
		case 9: strDesc = _T("FAN 3 fault"); break;
		case 10: strDesc = _T("FAN 4 fault"); break;
		}
		break;

		//case FAULT3_M2:
		//	break;

	case FAULT4_M2:
		switch (bit) {
		case 0: strDesc = _T("Battery over voltage (H/W)"); break;
		case 1: strDesc = _T("DC link over voltage (H/W)"); break;
		case 2: strDesc = _T("AC over current (H/W)"); break;
		case 3: strDesc = _T("DC over current (H/W)"); break;
		case 4: strDesc = _T("IGBT fault R TOP"); break;
		case 6: strDesc = _T("IGBT fault S TOP"); break;
		case 8: strDesc = _T("IGBT fault T TOP"); break;
		case 11: strDesc = _T("IGBT fault R BOTTOM"); break;
		case 13: strDesc = _T("IGBT fault S BOTTOM"); break;
		case 15: strDesc = _T("IGBT fault T BOTTOM"); break;
		}
		break;
	}

	return strDesc;
}

/**
* @reamarks
*	Trace 데이터를 수집하여 덤프 파일 생성
* @author 김동현
* @date 2016/08/17
* @param ns_uint8* data		CAN 메시지 데이터
* @return	성공:0, 실패:1
*/
ns_uint32 CNSPcsManager::TraceDump(ns_uint8 *data)
{
	ns_uint8 num = 0, idx = 0, status = 0, point = 0;

	num = data[0] & 0x0f;
	status = (data[0] >> 4) & 0x0f;
	idx = data[2];
	point = data[1];

#ifdef PCS_DUMP_MESSAGE_TRACE
	TRACE(_T("TraceDump pcs:%d status:%d no:%2d index:%3d point:%d %02x %02x %02x %02x \n"),
		_instanceID, status, num, idx, point, data[3], data[4], data[5], data[6]);
	//TRACE(_T(" ---> %f\n"), _traceData[_traceIdx]);
#endif

	if (_bTraceError)
		return 1;

	if (_bTimeout) {
		Log(SERVICE_TRACEDUMP, _T("Trace Dump Timeout."));
		if (_dump.Save() != 0)
			Log(ERROR_EXCEPTION, _T("Trace Dump Fail!!"));
		TraceTimeoutEnd();
		_bTraceError = 1;
		return 2;
	}

	if (TRACE_READY == status) {
		Log(SERVICE_TRACEDUMP, _T("Trace Dump Ready."));
		TraceTimeoutStart();
		_traceNum = 0;
		_traceIdx = 0;
		_bTraceError = 0;
		ZeroMemory(_traceData, sizeof(_traceData));
	}
	else if ((_traceNum == num) && (_traceIdx == idx)) {
		if (TRACE_START == status)
			Log(SERVICE_TRACEDUMP, _T("Trace Dump Start."));

		_traceData[_traceIdx] = GetTraceVaule(data);
		_traceData[256] = point;

		if (_traceIdx == 255) {
			if (status == TRACE_SENDING) {
				_dump.Add(_traceNum, _traceData);
			}
			else if ((_traceNum == 9) && (status == TRACE_END)) {
				Log(SERVICE_TRACEDUMP, _T("Trace Dump Finish."));
				_dump.Add(_traceNum, _traceData);
				if (_dump.Save() != 0)
					Log(ERROR_EXCEPTION, _T("Trace Dump Fail!!"));
				TraceTimeoutEnd();
			}

			_traceIdx = 0;
			_traceNum++;
		}
		else
			_traceIdx++;
	}
	else {
		Log(ERROR_EXCEPTION, _T("Trace Dump Stop : num:%d, idx:%d"), num, idx);
		if (_dump.Save() != 0)
			Log(ERROR_EXCEPTION, _T("Trace Dump Fail!!"));
		TraceTimeoutEnd();
		_bTraceError = 1;
	}

	return 0;
}

// 0:OK
ns_uint32 CNSPcsManager::TraceTimeoutStart()
{
	ns_uint32 ret = 0;
	unsigned int dummy = 0;

	_hTimeOutEvent = CreateEvent(NULL, TRUE, FALSE, _T("NSPcsMgrTTOEvt"));
	if (INVALID_HANDLE_VALUE == _hTimeOutEvent) {
		Log(ERROR_IPC, _T("Create Timeout Event Fail!!"));
		return 1;
	}

	_hTimeOutThread = (HANDLE)_beginthreadex(NULL, 0, TraceTimeoutThread, this, 0, &dummy);
	if (INVALID_HANDLE_VALUE == _hTimeOutThread) {
		Log(ERROR_IPC, _T("Begin Timeout Thread Fail!!"));
		return 1;
	}

	_bTimeout = 0;

	return ret;
}

ns_uint32 CNSPcsManager::TraceTimeoutEnd()
{
	ns_uint32 ret = 0;
	DWORD dwExitcode = 0;

	if (INVALID_HANDLE_VALUE != _hTimeOutEvent)
		TraceTimeoutStop();

	if (INVALID_HANDLE_VALUE != _hTimeOutThread) {
		dwExitcode = WaitForSingleObject(_hTimeOutThread, 10);
		if (STILL_ACTIVE == dwExitcode)
			TerminateThread(_hTimeOutThread, 0);

		CloseHandle(_hTimeOutThread);
		_hTimeOutThread = INVALID_HANDLE_VALUE;
	}

	_bTimeout = 0;

	return ret;
}

// 0:OK 1:timeout
ns_uint32 CNSPcsManager::TraceTimeout()
{
	DWORD dwStatus = 0;
	DWORD ms = TRACE_TIMEOUT;			// 10 sec

	dwStatus = WaitForSingleObject(_hTimeOutEvent, ms);
	if (dwStatus == WAIT_OBJECT_0) {
		_bTimeout = 0;
	}
	else if (dwStatus == WAIT_TIMEOUT) {
		_bTimeout = 1;
	}
	//TRACE(_T("CNSPcsManager::TraceTimeout timeout:%d\n"), _bTimeout);
	CloseHandle(_hTimeOutEvent);
	_hTimeOutEvent = INVALID_HANDLE_VALUE;

	return 0;
}

unsigned int __stdcall CNSPcsManager::TraceTimeoutThread(void *param)
{
	ns_uint32 ret = 0;
	CNSPcsManager *mgr = (CNSPcsManager *)param;
	ret = mgr->TraceTimeout();

	return ret;
}

ns_uint32 CNSPcsManager::TraceTimeoutStop()
{
	ns_uint32 ret = 0;
	SetEvent(_hTimeOutEvent);

	return ret;
}

/**
* @reamarks
*	Trace 데이터에서 실제 값을 계산
* @author 김동현
* @date 2016/08/17
* @param ns_uint8* data		CAN 메시지 데이터
* @return	실제 값
*/
float CNSPcsManager::GetTraceVaule(ns_uint8* data)
{
	ns_uint8 type = 0, ratio = 0;
	float val = 0;
	short hi = 0;
	short low = 0;

	type = data[7] & 0x0f;
	ratio = (data[7] >> 4) & 0x0f;

	if ((1 == type) || ((2 == type) && (1 == ratio))) {
		val += data[3] << 24;
		val += data[4] << 16;
		val += data[5] << 8;
		val += data[6];
	}
	else if ((2 == type) && (0 == ratio)) {
		hi = (data[3] << 8) | data[4];
		low = (data[5] << 8) | data[6];
		val = (float)(hi + (low * 0.0001));
	}
	else if ((2 == type) && (2 == ratio)) {
		val += data[3] << 24;
		val += data[4] << 16;
		val += data[5] << 8;
		val += data[6];
		val = (float)(val * 0.00000001);
	}
	else {
		Log(ERROR_EXCEPTION, _T("Unexpected Trace Vaule Type."));
	}

	return val;
}

/**
* @reamarks
*	PCS ListCtrl 항목별 위치와 단위를 결정하여 리스트에 표시하는 함수
* @author 김동현
* @date 2016/06/13
* @param ns_uint param		리스트 컨트롤 아이템 항목
* @param ns_uint header		리스트 컨트롤 아이템 헤더
* @param ns_uint16 value	리스트 컨트롤 아이템 데이터
* @return	0:성공, 1:실패
*/
void CNSPcsManager::ShowData(ns_uint8 param, ns_uint8 header, ns_uint16 value)
{
	HWND hWnd = NULL;
	CString strValue = _T("");

	switch (param) {
	case STATUS: strValue = CheckStatus(value); break;
	case ACTIVE_POWER: strValue.Format(_T("%.1f kW"), (ns_int16)value * 0.1f); break;
	case REACTIVE_POWER: strValue.Format(_T("%.1f kVar"), (ns_int16)value * 0.1f); break;
	case FREQUENCY: strValue.Format(_T("%.1f Hz"), value * 0.1f); break;
	case DC_LINK_VOLTAGE: strValue.Format(_T("%.1f V"), value * 0.1f); break;
	case VOLTAGE_RS: strValue.Format(_T("%.1f V"), value * 0.1f); break;
	case VOLTAGE_ST: strValue.Format(_T("%.1f V"), value * 0.1f); break;
	case VOLTAGE_TR: strValue.Format(_T("%.1f V"), value * 0.1f); break;
	case CURRENT_R: strValue.Format(_T("%.1f A"), value * 0.1f); break;
	case CURRENT_S: strValue.Format(_T("%.1f A"), value * 0.1f); break;
	case CURRENT_T: strValue.Format(_T("%.1f A"), value * 0.1f); break;
	case POWER_FACTOR: strValue.Format(_T("%.3f"), (ns_int16)value * 0.001f); break;
	case HS_TEMPERATURE_M1: strValue.Format(_T("%.1f ℃"), (ns_int16)value * 0.1f); break;
	case HS_TEMPERATURE_M2: strValue.Format(_T("%.1f ℃"), (ns_int16)value * 0.1f); break;
	case R_TEMPERATURE_M1: strValue.Format(_T("%.1f ℃"), (ns_int16)value * 0.1f); break;
	case R_TEMPERATURE_M2: strValue.Format(_T("%.1f ℃"), (ns_int16)value * 0.1f); break;
	case TODAY_CHARGE_ENERGY: strValue.Format(_T("%d kWh"), value); break;
	case TODAY_DISCHARGE_ENERGY: strValue.Format(_T("%d kWh"), value); break;
	case TOTAL_CHARGE_ENERGY: strValue.Format(_T("%.1f MWh"), value * 0.1f); break;
	case TOTAL_DISCHARGE_ENERGY: strValue.Format(_T("%.1f MWh"), value * 0.1f); break;
	case BAT_POWER: strValue.Format(_T("%.1f kW"), (ns_int16)value * 0.1f); break;
	case BAT_VOLTAGE: strValue.Format(_T("%.1f V"), value * 0.1f); break;
	case BAT_CURRENT: strValue.Format(_T("%.1f A"), (ns_int16)value * 0.1f); break;
	}

	if (_pChief) {
		hWnd = _pChief->GetPcsListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_PCSLIST_UPDATE, (header << 8) + param, (LPARAM)&strValue);
	}
}

/**
* @reamarks
*	PCS ListCtrl의 Status 구분 함수
* @author 김동현
* @date 2016/06/13
* @param ns_uint16 value		리스트 컨트롤 아이템 데이터
* @return	PCS 상태 문자열
* @deatils
*	STOP 상태에서는 모든 비트에 관계없이 STOP 상태로 표시 \n
*	STOP이 아닌 상태에서는 FAULT 비트가 최우선 순위 \n
*/
CString CNSPcsManager::CheckStatus(ns_uint16 value)
{
	CString strStatus = _T("");
	ns_uint8 hb = 0, status = 0;

	status = value & 0xff;
	hb = (value >> 8) & 0xff;

#if 1
	if(status & PCS_STATUS_READY)	strStatus = _T("READY");
#endif

	if (status & PCS_STATUS_RUN) {
		if (status & PCS_STATUS_CHARGE) _T("RUN(CHARGE)");
		else if (status & PCS_STATUS_DISCHARGE) _T("RUN(DISCHARGE)");
	}
	else
		strStatus = _T("STOP");

	if (status & PCS_STATUS_WARNING)	strStatus = _T("WARNING");
	if (status & PCS_STATUS_SWFAULT)	strStatus = _T("S/W FAULT");
	if (status & PCS_STATUS_HWFAULT)	strStatus = _T("H/W FAULT");

	return strStatus;
}


/**
* @reamarks
*	Fault 리스트 컨트롤에 Fault 정보를 표시
* @author 김동현
* @date 2016/06/17
* @param long id			CAN ID
* @param ns_uint header		리스트 컨트롤 헤더
* @param ns_uint16 Code1	에러코드1
* @param ns_uint16 Code2	에러코드2
* @param ns_uint16 Code2	에러코드3
* @param ns_uint16 Code4	에러코드4
* @return	0:성공, 1:실패
*/
void CNSPcsManager::ShowFault(ns_uint8 id, ns_uint8 header, ns_uint8 param, ns_uint16 Code1, ns_uint16 Code2, ns_uint16 Code3, ns_uint16 Code4)
{
	HWND hWnd = NULL;
	CString strFault = _T("");
	strFault.Format(_T("%d - %04x %04x %04x %04x"), id, Code1, Code2, Code3, Code4);

	if (_pChief) {
		hWnd = _pChief->GetFaultListHandle();
		if (hWnd)
			SendMessage(hWnd, WM_FAULTLIST_UPDATE, (header << 8) | param, (LPARAM)&strFault);
	}
}

CNSLog* CNSPcsManager::CreateLogObject()
{
	CNSLog *o = NULL;
	try {
		o = new CNSLog(_T("PCS"), _instanceID);
	}
	catch (...) {
		return NULL;
	}
	return o;
}

/**
* @reamarks
*	UI 로그 및 파일 로그
* @author 김동현
* @date 2016/08/17
* @param ns_uint8 filterFlag	로그 타입
* @param LPCTSTR lpszLog, ...	로그 문자열 가변인자
* @return	성공:0, 실패:1
*/
ns_uint32 CNSPcsManager::Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...)
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
		hWnd = _pChief->GetPcsLogBoxHandle(_instanceID);
		if (hWnd && _bLog && (_logFilter & filterFlag))
			SendMessage(hWnd, WM_LOGBOX_UPDATE, NULL, (LPARAM)&strLog);
	}

	return 0;
}

void CNSPcsManager::FilterLog(ns_uint8 bShow, ns_uint8 flag)
{
	_bLog = bShow;
	_logFilter = flag;
}

#ifdef __DISCONNECT_MODULE
void CNSPcsManager::ClearList(ns_uint8 instanceID)
{
	HWND hWnd = _pChief->GetPcsListHandle();
	int param, header = (instanceID == IID_PCS1) ? PCS1 : PCS2;

	for (param = STATUS; param <= BAT_CURRENT; param++) {
		SendMessage(hWnd, WM_PCSLIST_UPDATE, (header << 8) + param, (LPARAM)&_T(""));
	}
}
#endif