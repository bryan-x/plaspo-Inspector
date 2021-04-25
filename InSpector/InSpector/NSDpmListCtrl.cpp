#include "stdafx.h"
#include "NSDpmListCtrl.h"

BEGIN_MESSAGE_MAP(CNSDpmListCtrl, CXListCtrl)
	ON_MESSAGE(WM_DPMLIST_UPDATE, OnListCtrlUpdate)
#ifdef __DISCONNECT_MODULE
	ON_MESSAGE(WM_DPMLIST_CLEAR, OnListCtrlClear)
#endif
END_MESSAGE_MAP()

CNSDpmListCtrl::CNSDpmListCtrl()
{
	_hFont = NULL;
}

CNSDpmListCtrl::~CNSDpmListCtrl()
{
	DeleteObject(_hFont);
}

void CNSDpmListCtrl::Init()
{
	int i;
	LISTCTRLHEADER DPMListHeader[] =
	{
		LISTCTRLHEADER(_T("ID"),		10,	LVCFMT_CENTER),
		LISTCTRLHEADER(_T("DPM"),		50, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Value"),		40, LVCFMT_CENTER),
	};

	_hFont = CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_STRING_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));

	SendMessage(WM_SETFONT, (WPARAM)_hFont, (LPARAM)TRUE);

	CXListCtrl::InitListCtrlHeader(sizeof(DPMListHeader) / sizeof(LISTCTRLHEADER), DPMListHeader, false);

	LockWindowUpdate();
	DeleteAllItems();
	for (i = 0; i < DPM_LIST_SIZE; i++)
		InsertItem(i, _T(""));

	SetItemText(DPM_POWER, DPM_ID, _T("61"));
	SetItemText(DPM_POWER, DPM_PARAMETER, _T("Power"));
	SetItemText(DPM_VOLTAGE, DPM_PARAMETER, _T("Voltage"));
	SetItemText(DPM_CURRENT, DPM_PARAMETER, _T("Current"));
	UnlockWindowUpdate();
}

LRESULT CNSDpmListCtrl::OnListCtrlUpdate(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)lParam;
	int header, param;
	header = (wParam >> 8) & 0xff;
	param = wParam & 0xff;
	SetItemText(param, header, *str);
	return 0;
}

#ifdef __DISCONNECT_MODULE
LRESULT CNSDpmListCtrl::OnListCtrlClear(WPARAM wParam, LPARAM lParam)
{
	SetItemText(DPM_POWER, DPM_VALUE, _T(""));
	SetItemText(DPM_VOLTAGE, DPM_VALUE, _T(""));
	SetItemText(DPM_CURRENT, DPM_VALUE, _T(""));
	return 0;
}
#endif