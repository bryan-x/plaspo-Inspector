#pragma once
#include ".\XListCtrl\XListCtrl.h"

#define WM_DPMLIST_UPDATE (WM_USER + MSGID_DPM_UPDATE)

#ifdef __DISCONNECT_MODULE
#define WM_DPMLIST_CLEAR (WM_USER + MSGID_DPM_CLEAR)
#endif

enum DPM_LIST_HEADER {
	DPM_ID = 0,
	DPM_PARAMETER,
	DPM_VALUE,
};

enum DPM_LIST_PARAMETER {
	DPM_POWER = 0,
	DPM_VOLTAGE,
	DPM_CURRENT,
	DPM_LIST_SIZE,
};


class CNSDpmListCtrl : public CXListCtrl
{
public:
	CNSDpmListCtrl();
	~CNSDpmListCtrl();

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