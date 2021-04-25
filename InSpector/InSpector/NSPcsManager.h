/**
* @file NSPcsManager.h
* @date 2016/07/11
* @author �赿��
* @brief
*	PCS �Ŵ��� Ŭ����
* @details
*	CAN �����͸� �з��Ͽ� UI�� ǥ��
*	Fault �߻��� Ʈ���̽� ���� ����
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
*	PCS ���� �޽��� ó�� �� UI ǥ��
* @date 2016/07/11
* @author �赿��
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
	ns_uint8 _instanceID;					/**< ��ü �ĺ��� */
	CNSChiefManager* _pChief;
	
	HANDLE _hThread;						/**< PCS �Ŵ��� ������ �ڵ� */
	HANDLE _hStopEVent;						/**< ������ ���� �̺�Ʈ */
	ns_bool	_bEndThread;					/**< ������ ���� �÷��� */

	HANDLE _hTimeOutThread;					/**< Ʈ���̽� Ÿ�Ӿƿ� ������ �ڵ� */
	HANDLE _hTimeOutEvent;					/**< Ʈ���̽� Ÿ�Ӿƿ� �̺�Ʈ �ڵ� */
	ns_bool _bTimeout;						/**< Ʈ���̽� Ÿ�Ӿƿ� ���� �÷��� */
	ns_bool _bTraceError;					/**< Ʈ���̽� ������ ���� ���� �÷��� */
	ns_uint8 _traceNum;						/**< Ʈ���̽� �����˻� ��ȣ */
	ns_uint8 _traceIdx;						/**< Ʈ���̽� �����˻� ī��Ʈ */
	ns_float _traceData[TRACE_BUF_SIZE];	/**< Ʈ���̽� ���� ������ */
	CNSPcsTraceDump _dump;					/**< Ʈ���̽� ���� ��ü */

	ns_uint16 _pcsData[LIST_SIZE];			/**< ������ ���� */
	ns_uint16 _pcsFault[FAULT_SIZE];		/**< Fault ������ ���� */

	CNSLog*	_log;							/**< �α� ��ü ������ */
	ns_uint8 _bLog;							/**< �α� on/off �÷��� */
	ns_uint8 _logFilter;					/**< �α� Ÿ�� ����*/
};