// https://msdn.microsoft.com/en-us/library/bb983759(d=printer,v=vs.100).aspx?appid=dev10idef1&l=en-us&k=k(cmfcpropertygridctrl)%3bk(devlang-%22c%2b%2b%22)&rd=true


#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "InSpector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	//int cyCmb = rectCombo.Size().cy;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE(_T("Failed to create Properties Grid \n"));
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	/// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	/// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::OnProperties2()
{
	/// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	/// TODO: Add your command update UI handler code here
}

void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	//////////////////////////////////////////////////////////////////////////////
	// Properties Group1

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("Protocol"));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("CAN"), _T("2.0B"), _T("~~"));
	pProp->AddOption(_T("2.0B"));
	pProp->AddOption(_T("Sel 1"));
	pProp->AddOption(_T("Sel 2"));
	pProp->AllowEdit(FALSE);
	pGroup1->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty(_T("BPS"), _T("500K"), _T("~~"));
	pProp->AddOption(_T("500K"));
	pProp->AddOption(_T("Sel 1"));
	pProp->AddOption(_T("Sel 2"));
	pProp->AllowEdit(FALSE);
	pGroup1->AddSubItem(pProp);

	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Bus-Off Auto Reset"), (_variant_t) false, _T("~~")));

	m_wndPropList.AddProperty(pGroup1);

	//////////////////////////////////////////////////////////////////////////////
	// Properties Group2

	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Alarm Notify"));

	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Bus-Off Error"), (_variant_t) true, _T("~~")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Error-Passive Error"), (_variant_t) true, _T("~~")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Warning Error"), (_variant_t) true, _T("~~")));

	m_wndPropList.AddProperty(pGroup2);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}


/**
* @reamarks
*	Property 항목에 설정된 값을 얻어오는 함수
* @author 김동현
* @date 2016/06/22
* @param int nPropIdx		Property Group Index
* @param int nSubItemIdx	Group SubItem Index
*/
const COleVariant& CPropertiesWnd::GetPropValue(int nPropIdx, int nSubItemIdx)
{
	return m_wndPropList.GetProperty(nPropIdx)->GetSubItem(nSubItemIdx)->GetValue();
}


/**
* @reamarks
*	Property 항목에 지정된 값을 설정하는 함수
* @author 김동현
* @date 2016/06/22
* @param int nPropIdx					Property Group Index
* @param int nSubItemIdx				Group SubItem Index
* @param const COleVariant& varValue	New Property Value
*/
void CPropertiesWnd::SetPropValue(int nPropIdx, int nSubItemIdx, const COleVariant& varValue)
{
	m_wndPropList.GetProperty(nPropIdx)->GetSubItem(nSubItemIdx)->SetValue(varValue);
}
