#pragma once
#include ".\XListCtrl\XListCtrl.h"

#define WM_CSLIST_UPDATE (WM_USER + MSGID_CS_UPDATE)

#ifdef __DISCONNECT_MODULE
#define WM_CSLIST_CLEAR (WM_USER + MSGID_CS_CLEAR)
#endif

enum CS_LIST_HEADER {
	CS_ID = 0,
	CS_PARAMETER,
	CS_VALUE,
};


enum CS_LIST_PARAMETER {
	CS_STATUS = 0,
	CS_PRESSURE1,
	CS_PRESSURE2,
	CS_TEMPERATURE1,
	CS_TEMPERATURE2,
	CS_TEMPERATURE3,
	CS_FLOWMETER1,
	CS_FLOWMETER2,
	CS_LIST_SIZE,
};

class CNSCsListCtrl : public CXListCtrl
{
public:
	CNSCsListCtrl();
	~CNSCsListCtrl();

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