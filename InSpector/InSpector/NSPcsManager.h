/**
* @file NSPcsManager.h
* @date 2016/07/11
* @author 김동현
* @brief
*	PCS 매니저 클래스
* @details
*	CAN 데이터를 분류하여 UI로 표현
*	Fault 발생시 트레이스 덤프 수행
*/

#pragma once

#include "NSPcsListCtrl.h"
#include "NSFaultListCtrl.h"
#include "CanMessage.h"
#include "NSPcsTraceDump.h"

#define IID_PCS1	1
#define IID_PCS2	2

#define TRACE_TIMEOUT	10000

#define PCS_STATUS_RUN			0x01
#define PCS_STATUS_GATING		0x02
#define PCS_STATUS_READY		0x04
#define PCS_STATUS_CHARGE		0x08
#define PCS_STATUS_SWFAULT		0x10
#define PCS_STATUS_HWFAULT		0x20
#define PCS_STATUS_WARNING		0x40
#define PCS_STATUS_DISCHARGE	0x80


class CNSChiefManager;
class CNSLog;


/**
* @brief
*	PCS 관련 메시지 처리 및 UI 표시
* @date 2016/07/11
* @author 김동현
*/
class CNSPcsManager
{
public:
	CNSPcsManager(ns_uint8 instanceID, CNSChiefManager* chief);
	~CNSPcsManager();

	ns_uint32 Create();
	ns_uint32 Destroy();
	ns_uint32 Start();
	ns_uint32 Stop();
	ns_uint32 TraceDump(ns_uint8 *data);
	void FilterLog(ns_uint8 bShow ,ns_uint8 flag);
#ifdef __DISCONNECT_MODULE
	void ClearList(ns_uint8 instanceID);
#endif
	
private:
	static unsigned int __stdcall PcsThreadProc(void *param);

	ns_uint32 Receive();
	ns_uint32 Parse(const CanMessage& m);

	ns_uint32 SaveFault(const CString& strType, const CString& strFault);
	ns_uint32 ParseFault(const CanMessage& m);
	ns_uint CheckFaultBit(ns_uint typeInfo, ns_uint16 faultVal, CString& strType, CString& strFault);
	CString GetFaultDesc(ns_uint subInfo, ns_uint bit);

	static unsigned int __stdcall TraceTimeoutThread(void *param);
	ns_uint32 TraceTimeoutStart();
	ns_uint32 TraceTimeoutEnd();
	ns_uint32 TraceTimeout();
	ns_uint32 TraceTimeoutStop();
	float GetTraceVaule(ns_uint8* data);
	
	CString CheckStatus(ns_uint16 value);
	void ShowData(ns_uint8 param, ns_uint8 header, ns_uint16 value);
	void ShowFault(ns_uint8 id, ns_uint8 header, ns_uint8 param, ns_uint16 Code1, ns_uint16 Code2, ns_uint16 Code3, ns_uint16 Code4);

	CNSLog* CreateLogObject();
	ns_uint32 Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...);

private:
#if 0
	enum TRACE_PARAMETER {
		TRACE_STATUS = 0,
		TRACE_POINT,
		TRACE_COUNT,
		TRACE_DATA3,
		TRACE_DATA2,
		TRACE_DATA1,
		TRACE_DATA0,
		TRACE_DATA_TYPE,
		TRACE_SIZE,
	};
#endif

	CRITICAL_SECTION _cs;
	ns_uint8 _instanceID;					/**< 객체 식별자 */
	CNSChiefManager* _pChief;
	
	HANDLE _hThread;						/**< PCS 매니저 쓰레드 핸들 */
	HANDLE _hStopEVent;						/**< 쓰레드 종료 이벤트 */
	ns_bool	_bEndThread;					/**< 쓰레드 종료 플래그 */

	HANDLE _hTimeOutThread;					/**< 트레이스 타임아웃 쓰레드 핸들 */
	HANDLE _hTimeOutEvent;					/**< 트레이스 타임아웃 이벤트 핸들 */
	ns_bool _bTimeout;						/**< 트레이스 타임아웃 상태 플래그 */
	ns_bool _bTraceError;					/**< 트레이스 데이터 에러 상태 플래그 */
	ns_uint8 _traceNum;						/**< 트레이스 순차검사 번호 */
	ns_uint8 _traceIdx;						/**< 트레이스 순차검사 카운트 */
	ns_float _traceData[TRACE_BUF_SIZE];	/**< 트레이스 수집 데이터 */
	CNSPcsTraceDump _dump;					/**< 트레이스 덤프 객체 */

	ns_uint16 _pcsData[LIST_SIZE];			/**< 데이터 버퍼 */
	ns_uint16 _pcsFault[FAULT_SIZE];		/**< Fault 데이터 버퍼 */

	CNSLog*	_log;							/**< 로그 객체 포인터 */
	ns_uint8 _bLog;							/**< 로그 on/off 플래그 */
	ns_uint8 _logFilter;					/**< 로그 타입 필터*/
};