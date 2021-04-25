#pragma once

#include "CanMessage.h"
#include <map>


#define NUM_OF_IDS	31

typedef enum {
	MI_DPM =0,
	MI_PCS1,
	MI_PCS2
} MODULE_INDEX;


typedef enum {
	CID_NONE = 0,

	CID_PCS_1 = 1,
	CID_PCS_2_VOL,
	CID_PCS_3,
	CID_PCS_4_TEMP,
	CID_PCS_5_ENERGY,
	CID_PCS_6_BAT,
	CID_PCS_7_BMS,
	CID_PCS_8,
	CID_PCS_9_M,
	CID_PCS_10_M,
	CID_PCS_11_FAULT,
	CID_PCS_12_FAULT,

	CID_PCS_21 = 21,
	CID_PCS_22_VOL,
	CID_PCS_23,
	CID_PCS_24_TEMP,
	CID_PCS_25_ENERGY,
	CID_PCS_26_BAT,
	CID_PCS_27_BMS,
	CID_PCS_28,
	CID_PCS_29_M,
	CID_PCS_30_M,
	CID_PCS_31_FAULT,
	CID_PCS_32_FAULT,

	CID_PCS1_TRACE = 51,
	CID_PCS2_TRACE,

	CID_DPM_61 = 61,
	CID_COOL_62,
	CID_COOL_63,

	CID_PCS_CONTROL = 64,
	CID_EMS = 65,
	CID_MAX
} CAN_ID;


class CNSCanEngine;
class CNSCanDump;
class CNSChiefManager;
class CNSPcsManager;
class CNSLog;
class CNSCanObject;

class CNSCanObject
{
public:
	CNSCanObject();
	~CNSCanObject();
	void SetMessage(CanMessage* m);
	CanMessage* GetMessage();

private:
	CRITICAL_SECTION _cs;
	CanMessage _m[2];
	ns_uint8 _idx;
	ns_uint8 _latest;
};


class CNSCanManager 
{
public:
	CNSCanManager(CNSChiefManager* chief);
	~CNSCanManager();

	ns_uint32 Create(CNSPcsManager* pcs1, CNSPcsManager* pcs2);
	ns_uint32 Destroy();
	ns_uint32 Start();  
	ns_uint32 Stop();

	ns_uint32 map_write(CanMessage *m);
	ns_uint32 map_read_pcs1(ns_uint32 id, CanMessage* out);
	ns_uint32 map_read_pcs2(ns_uint32 id, CanMessage* out);
	ns_uint32 map_read_mc(ns_uint32 id, CanMessage* out);

	ns_uint32 Log(ns_uint8 filterFlag, LPCTSTR lpszLog, ...);
	void FilterLog(ns_uint8 bShow, ns_uint8 flag);

	void GetMsgCount(ns_uint16* arr);

#ifdef __DISCONNECT_MODULE
	void ClearPcsList(ns_uint8 instanceID);
	void ClearFaultList();
	void ClearDpmList();
	void ClearCsList();
#endif

private:
	CNSCanDump *CreateCanDump();
	CNSCanEngine *CreateCanEngine();
	CNSLog* CreateLogObject();
	CNSCanObject* CreateCanObject();
	ns_uint32 map_init();
	void map_clear();

	void IncreaseMsgCnt(ns_uint8 module);
	void ResetMsgCnt(ns_uint8 module);
	ns_uint16 GetMsgCntByIndex(ns_uint8 module);

	//ns_uint32 InitListCtrl();
	//ns_uint32 ShowData(CanMessage* msg);
	
private:
#if 0
	// Header
	enum HEADER
	{
		ID = 0,
		TIMESTAMP,
		RTR,
		DLC,
		DATA,
	};
#endif
	CRITICAL_SECTION _cs;
	CRITICAL_SECTION _cs_pcs1;
	CRITICAL_SECTION _cs_pcs2;
	CRITICAL_SECTION _cs_mc;

	CNSCanEngine* _can;		/**< CAN 엔진 포인터 */
	CNSCanDump*	_dump;		/**< CAN 덤프 포인터 */

	CNSChiefManager* _pChief;
	CNSPcsManager* _pcs1;	/**< PCS1 매니저 포인터 */
	CNSPcsManager* _pcs2;	/**< PCS2 매니저 포인터 */

	std::map<ns_uint32, CNSCanObject*> _pcs1Map;
	std::map<ns_uint32, CNSCanObject*> _pcs2Map;
	std::map<ns_uint32, CNSCanObject*> _mcMap;

	CNSLog* _log;			/**< 로그 객체 포인터 */
	ns_uint8 _bLog;			/**< 로그 on/off 플래그 */
	ns_uint8 _logFilter;	/**< 로그 타입 필터 */

	ns_uint16 _msgCnt[3];	/**< 0:mc, 1:pcs1, 2:pcs2 */
};