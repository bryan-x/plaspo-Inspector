#pragma once
#include ".\XListCtrl\XListCtrl.h"

#define WM_FAULTLIST_UPDATE (WM_USER + MSGID_FAULT_UPDATE)

#ifdef __DISCONNECT_MODULE
#define WM_FAULT_CLEAR (WM_USER + MSGID_FAULT_CLEAR)
#endif

enum FAULT_LIST_HEADER {
	FAULT_NONE = 0,
	FAULT_PCS1,
	FAULT_PCS2,
};

enum FAULT_LIST_PARAMETER {
	FAULT_M1 = 0,
	FAULT_M2,
};

enum FAULT_PARAMETER {
	FAULT1_M1,
	FAULT2_M1,
	FAULT3_M1,		// spare
	FAULT4_M1,
	FAULT1_M2,
	FAULT2_M2,
	FAULT3_M2,		// spare
	FAULT4_M2,
	FAULT_SIZE,
};

class CNSFaultListCtrl : public CXListCtrl
{
public:
	CNSFaultListCtrl();
	~CNSFaultListCtrl();

	void Init();

private:
	afx_msg LRESULT OnListCtrlUpdate(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	HFONT _hFont;
};