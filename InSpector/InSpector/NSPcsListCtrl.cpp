#include "stdafx.h"
#include "NSPcsListCtrl.h"

BEGIN_MESSAGE_MAP(CNSPcsListCtrl, CXListCtrl)
ON_MESSAGE(WM_PCSLIST_UPDATE, OnListCtrlUpdate)
#ifdef __DISCONNECT_MODULE
ON_MESSAGE(WM_PCS1_LIST_CLEAR, OnListCtrlClear)
ON_MESSAGE(WM_PCS2_LIST_CLEAR, OnListCtrlClear)
#endif
END_MESSAGE_MAP()

CNSPcsListCtrl::CNSPcsListCtrl()
{
	_hFont = NULL;
}

CNSPcsListCtrl::~CNSPcsListCtrl()
{
	DeleteObject(_hFont);
}

void CNSPcsListCtrl::Init()
{
	int i;
	LISTCTRLHEADER pcsListHeader[] =
	{
		LISTCTRLHEADER(_T("ID"),		12, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("Parameter"),	44, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("PCS-1"),		22, LVCFMT_CENTER),
		LISTCTRLHEADER(_T("PCS-2"),		22, LVCFMT_CENTER),
	};

	_hFont = CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_STRING_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));

	SendMessage(WM_SETFONT, (WPARAM)_hFont, (LPARAM)TRUE);

	InitListCtrlHeader(sizeof(pcsListHeader) / sizeof(LISTCTRLHEADER), pcsListHeader, false);

	// ***** lock window updates while filling list *****
	LockWindowUpdate();

	DeleteAllItems();

	for (i = 0; i < LIST_SIZE; i++)
		InsertItem(i, _T(""));

	SetItemText(STATUS, ID, _T("1(21)"));
	SetItemText(STATUS, PARAMETER, _T("Status"));
	SetItemText(ACTIVE_POWER, PARAMETER, _T("Active Power"));
	SetItemText(REACTIVE_POWER, PARAMETER, _T("Reactive Power"));
	SetItemText(FREQUENCY, PARAMETER, _T("Frequency"));

	SetItemText(DC_LINK_VOLTAGE, ID, _T("2(22)"));
	SetItemText(DC_LINK_VOLTAGE, PARAMETER, _T("DC-link Voltage"));
	SetItemText(VOLTAGE_RS, PARAMETER, _T("Voltage R-S"));
	SetItemText(VOLTAGE_ST, PARAMETER, _T("Voltage S-T"));
	SetItemText(VOLTAGE_TR, PARAMETER, _T("Voltage T-R"));

	SetItemText(CURRENT_R, ID, _T("3(23)"));
	SetItemText(CURRENT_R, PARAMETER, _T("Current R"));
	SetItemText(CURRENT_S, PARAMETER, _T("Current S"));
	SetItemText(CURRENT_T, PARAMETER, _T("Current T"));
	SetItemText(POWER_FACTOR, PARAMETER, _T("Power Factor"));

	SetItemText(HS_TEMPERATURE_M1, ID, _T("4(24)"));
	SetItemText(HS_TEMPERATURE_M1, PARAMETER, _T("Head-sink temperature M1"));
	SetItemText(HS_TEMPERATURE_M2, PARAMETER, _T("Head-sink temperature M2"));
	SetItemText(R_TEMPERATURE_M1, PARAMETER, _T("Reactor temperature M1"));
	SetItemText(R_TEMPERATURE_M2, PARAMETER, _T("Reactor temperature M2"));

	SetItemText(TODAY_CHARGE_ENERGY, ID, _T("5(25)"));
	SetItemText(TODAY_CHARGE_ENERGY, PARAMETER, _T("Today Charge Energy"));
	SetItemText(TODAY_DISCHARGE_ENERGY, PARAMETER, _T("Today Discharge Energy"));
	SetItemText(TOTAL_CHARGE_ENERGY, PARAMETER, _T("Total Charge Energy"));
	SetItemText(TOTAL_DISCHARGE_ENERGY, PARAMETER, _T("Total Discharge Energy"));

	SetItemText(BAT_POWER, ID, _T("6(26)"));
	SetItemText(BAT_POWER, PARAMETER, _T("BAT Power"));
	SetItemText(BAT_VOLTAGE, PARAMETER, _T("BAT Voltage"));
	SetItemText(BAT_CURRENT, PARAMETER, _T("BAT Current"));

	// ***** unlock window updates *****
	UnlockWindowUpdate();
}

LRESULT CNSPcsListCtrl::OnListCtrlUpdate(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)lParam;
	int header = (wParam >> 8) & 0xff;
	int param = wParam & 0xff;
	SetItemText(param, header, *str);
	return 0;
}

#ifdef __DISCONNECT_MODULE
LRESULT CNSPcsListCtrl::OnListCtrlClear(WPARAM wParam, LPARAM lParam)
{
	int param;
	for (param = STATUS; param <= BAT_CURRENT; param++)
		SetItemText(param, (int)wParam, _T(""));
	return 0;
}
#endif