
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_3D_ROUNDED, rectDummy, this, 1))
	{
		TRACE(_T("Failed to create output tab window\n"));
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!_pcs1LogBox.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!_pcs2LogBox.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!_mcLogBox.Create(dwStyle, rectDummy, &m_wndTabs, 4) || 
		!_canLogBox.Create(dwStyle, rectDummy, &m_wndTabs, 5)) 
	{
		TRACE(_T("Failed to create output windows\n"));
		return -1;      // fail to create
	}

	UpdateFonts();

	m_wndTabs.AddTab(&_pcs1LogBox, _T("PCS-1"), (UINT)0);
	m_wndTabs.AddTab(&_pcs2LogBox, _T("PCS-2"), (UINT)1);
	m_wndTabs.AddTab(&_mcLogBox, _T("MC"), (UINT)2);
	m_wndTabs.AddTab(&_canLogBox, _T("CAN"), (UINT)3);

	//_pcs1LogBox.Log(_T("[PCS-1 LogBox]"));
	//_pcs2LogBox.Log(_T("[PCS-2 LogBox]"));
	//_mcLogBox.Log(_T("[Master Controller LogBox]"));
	//_canLogBox.Log(_T("[CAN LogBox]"));

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}


void COutputWnd::UpdateFonts()
{
	_pcs1LogBox.SetFont(&afxGlobalData.fontRegular);
	_pcs2LogBox.SetFont(&afxGlobalData.fontRegular);
	_mcLogBox.SetFont(&afxGlobalData.fontRegular);
	_canLogBox.SetFont(&afxGlobalData.fontRegular);
}


void COutputWnd::LogClear()
{
	_pcs1LogBox.Clear();
	_pcs2LogBox.Clear();
	_mcLogBox.Clear();
	_canLogBox.Clear();
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_MESSAGE(WM_LOGBOX_UPDATE, OnLog)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}

void COutputList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}

#if 0
void COutputList::Log(LPCTSTR lpszLog)
{
	int i = AddString(lpszLog);

	// scroll into view
	if (i >= 0)
		SetTopIndex(i);

	SetCurSel(-1);
}
#endif

LRESULT COutputList::OnLog(WPARAM wParam, LPARAM lParam)
{
	CString* str = (CString*)lParam;
	int i = 0;

	i = AddString(*str);
	if (i >= 0)
		SetTopIndex(i);

	SetCurSel(-1);

	return 0;
}


void COutputList::Clear()
{
	while (GetCount() > 0)
		DeleteString(0);
}