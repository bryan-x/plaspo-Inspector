#pragma once
#include ".\XListCtrl\XListCtrl.h"

#define WM_PCSLIST_UPDATE (WM_USER + MSGID_PCS_UPDATE)

#ifdef __DISCONNECT_MODULE
#define WM_PCS1_LIST_CLEAR (WM_USER + MSGID_PCS1_CLEAR)
#define WM_PCS2_LIST_CLEAR (WM_USER + MSGID_PCS2_CLEAR)
#endif

enum PCS_LIST_HEADER {
	ID = 0,
	PARAMETER,
	PCS1,
	PCS2,
};

static enum PCS_LIST_PARAMETER {
	STATUS = 0,
	ACTIVE_POWER,
	REACTIVE_POWER,
	FREQUENCY,
	DC_LINK_VOLTAGE,
	VOLTAGE_RS,
	VOLTAGE_ST,
	VOLTAGE_TR,
	CURRENT_R,
	CURRENT_S,
	CURRENT_T,
	POWER_FACTOR,
	HS_TEMPERATURE_M1,
	HS_TEMPERATURE_M2,
	R_TEMPERATURE_M1,
	R_TEMPERATURE_M2,
	TODAY_CHARGE_ENERGY,
	TODAY_DISCHARGE_ENERGY,
	TOTAL_CHARGE_ENERGY,
	TOTAL_DISCHARGE_ENERGY,
	BAT_POWER,
	BAT_VOLTAGE,
	BAT_CURRENT,
	LIST_SIZE,		// UI ����Ʈ ��Ʈ�� ũ��

	BMS_STATUS = LIST_SIZE,
	BMS_SOC,
	BMS_RELAY,
	BMS_VOLTAGE,
	BMS_CURRENT,
	EXT_BMS_SIZE,		// BMS ������ ������ ũ��
};

class CNSPcsListCtrl : public CXListCtrl
{
public:
	CNSPcsListCtrl();
	~CNSPcsListCtrl();

	void Init();

private:
	afx_msg LRESULT OnListCtrlUpdate(WPARAM wParam, LPARAM lParam);
#ifdef __DISCONNECT_MODULE
	afx_msg LRESULT OnListCtrlClear(WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()

private:
	HFONT _hFont;
};