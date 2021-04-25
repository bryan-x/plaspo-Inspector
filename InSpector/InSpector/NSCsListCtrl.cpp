#include "stdafx.h"
#include "NSCsListCtrl.h"

BEGIN_MESSAGE_MAP(CNSCsListCtrl, CXListCtrl)
	ON_MESSAGE(WM_CSLIST_UPDATE, OnListCtrlUpdate)
#ifdef __DISCONNECT_MODULE
	ON_MESSAGE(WM_CSLIST_CLEAR, OnListCtrlClear)
#endif
END_MESSAGE_MAP()

CNSCsListCtrl::CNSCsListCtrl()
{
	_hFont = NULL;
}

CNSCsListCtrl::~CNSCsListCtrl()
{
	DeleteObject(_hFont);
}

void CNSCsListCtrl::Init()
{
	int i;
	LISTCTRLHEADER CSListHeader[] =
	{
		LISTCTRLHEADER(_T("ID"),				10,	LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Cooling System"),	50, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Value"),				40, LVCFMT_CENTER),
	};

	_hFont = CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_STRING_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));

	SendMessage(WM_SETFONT, (WPARAM)_hFont, (LPARAM)TRUE);

	CXListCtrl::InitListCtrlHeader(sizeof(CSListHeader) / sizeof(LISTCTRLHEADER), CSListHeader, false);

	LockWindowUpdate();
	DeleteAllItems();
	for (i = 0; i < CS_LIST_SIZE; i++)
		InsertItem(i, _T(""));

	SetItemText(CS_STATUS, CS_ID, _T("62"));
	SetItemText(CS_STATUS, CS_PARAMETER, _T("Status"));
	SetItemText(CS_PRESSURE1, CS_PARAMETER, _T("Pressure 1"));
	SetItemText(CS_PRESSURE2, CS_PARAMETER, _T("Pressure 2"));
	SetItemText(CS_TEMPERATURE1, CS_PARAMETER, _T("Temperature 1"));

	SetItemText(CS_TEMPERATURE2, CS_ID, _T("63"));
	SetItemText(CS_TEMPERATURE2, CS_PARAMETER, _T("Temperature 2"));
	SetItemText(CS_TEMPERATURE3, CS_PARAMETER, _T("Temperature 3"));
	SetItemText(CS_FLOWMETER1, CS_PARAMETER, _T("Flow Meter1"));
	SetItemText(CS_FLOWMETER2, CS_PARAMETER, _T("Flow Meter2"));
	UnlockWindowUpdate();
}

LRESULT CNSCsListCtrl::OnListCtrlUpdate(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)lParam;
	int header, param;
	header = (wParam >> 8) & 0xff;
	param = wParam & 0xff;
	SetItemText(param, header, *str);
	return 0;
}

#ifdef __DISCONNECT_MODULE
LRESULT CNSCsListCtrl::OnListCtrlClear(WPARAM wParam, LPARAM lParam)
{
	int param;
	for (param = CS_STATUS; param <= CS_FLOWMETER2; param++)
		SetItemText(param, CS_VALUE, _T(""));

	return 0;
}
#endif