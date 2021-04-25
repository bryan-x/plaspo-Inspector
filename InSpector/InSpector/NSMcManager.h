/**
* @file NSMcManager.h
* @date 2016/06/16
* @author �赿��
* @brief
*	Master Controller �Ŵ��� Ŭ����
* @details
*	CAN �����͸� �з��Ͽ� UI�� ǥ����.
*/

#pragma once
#include "CanMessage.h"
#include "NSDpmListCtrl.h"
#include "NSCsListCtrl.h"

class CNSChiefManager;
class CNSLog;


/**
* @brief
*	BMS, DPM, Cooling System ��� �����͸� ���� ����
* @date 2016/06/16
* @author �赿��
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

	HANDLE		_hThread;		/**< ������ �ڵ� */
	HANDLE		_hStopEvent;	/**< ������ ���� �̺�Ʈ �ڵ� */
	ns_uint8	_bEndThread;	/**< ������ ���� �÷��� */

	ns_uint16 _dpmData[DPM_LIST_SIZE];		/**< DPM ������ ���� */
	ns_uint16 _csData[CS_LIST_SIZE];		/**< CS ������ ���� */

	CNSLog* _log;				/**< �α� ��ü ������ */
	ns_uint8 _bLog;				/**< �α� on/off �÷��� */
	ns_uint8 _logFilter;		/**< �α� Ÿ�� ����*/
};