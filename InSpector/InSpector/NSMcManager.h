/**
* @file NSMcManager.h
* @date 2016/06/16
* @author 김동현
* @brief
*	Master Controller 매니저 클래스
* @details
*	CAN 데이터를 분류하여 UI로 표현함.
*/

#pragma once
#include "CanMessage.h"
#include "NSDpmListCtrl.h"
#include "NSCsListCtrl.h"

class CNSChiefManager;
class CNSLog;


/**
* @brief
*	BMS, DPM, Cooling System 모듈 데이터를 통합 관리
* @date 2016/06/16
* @author 김동현
*/
class CNSMcManager
{
public:
	CNSMcManager(CNSChiefManager* chief);
	~CNSMcManager();

	ns_uint32 Create();
	ns_uint32 Destroy();
	ns_uint32 Start();
	ns_uint32 Stop();

	void FilterLog(ns_uint8 bShow, ns_uint8 flag);

private:
	static unsigned int __stdcall MCThreadProc(void *param);

	ns_uint32 Receive();
	ns_uint32 Parse(const CanMessage& m);
	void ShowDpmData(ns_uint param, ns_uint16 value);
	void ShowCsData(ns_uint param, ns_uint16 value);
	CString CheckCsStatus(ns_uint16 value);

	CNSLog* CreateLogObject(LPCTSTR module);
	ns_uint32 Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...);

private:
	CNSChiefManager* _pChief;

	HANDLE		_hThread;		/**< 쓰레드 핸들 */
	HANDLE		_hStopEvent;	/**< 쓰레드 종료 이벤트 핸들 */
	ns_uint8	_bEndThread;	/**< 쓰레드 종료 플래그 */

	ns_uint16 _dpmData[DPM_LIST_SIZE];		/**< DPM 데이터 버퍼 */
	ns_uint16 _csData[CS_LIST_SIZE];		/**< CS 데이터 버퍼 */

	CNSLog* _log;				/**< 로그 객체 포인터 */
	ns_uint8 _bLog;				/**< 로그 on/off 플래그 */
	ns_uint8 _logFilter;		/**< 로그 타입 필터*/
};