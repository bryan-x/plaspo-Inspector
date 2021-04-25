#include "stdafx.h"
#include "NSFaultListCtrl.h"

BEGIN_MESSAGE_MAP(CNSFaultListCtrl, CXListCtrl)
	ON_MESSAGE(WM_FAULTLIST_UPDATE, OnListCtrlUpdate)
END_MESSAGE_MAP()

CNSFaultListCtrl::CNSFaultListCtrl()
{
	_hFont = NULL;
}

CNSFaultListCtrl::~CNSFaultListCtrl()
{
	DeleteObject(_hFont);
}

void CNSFaultListCtrl::Init()
{
	LISTCTRLHEADER faultListHeader[] =
	{
		LISTCTRLHEADER(_T(""),		20,	LVCFMT_CENTER),
		LISTCTRLHEADER(_T("PCS-1"),	40, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("PCS-2"),	40, LVCFMT_CENTER),
	};

	_hFont = CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_STRING_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));

	SendMessage(WM_SETFONT, (WPARAM)_hFont, (LPARAM)TRUE);

	CXListCtrl::InitListCtrlHeader(sizeof(faultListHeader) / sizeof(LISTCTRLHEADER), faultListHeader, false);

	LockWindowUpdate();
	DeleteAllItems();
	InsertItem(0, _T("Fault M1"));
	InsertItem(1, _T("Fault M2"));
	UnlockWindowUpdate();
}

LRESULT CNSFaultListCtrl::OnListCtrlUpdate(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)lParam;
	int header, nItem;
	header = (wParam >> 8) & 0xff;
	nItem = wParam & 0xff;
	SetItemText(nItem, header, *str);
	return 0;
}