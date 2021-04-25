
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "InSpector.h"

#include "MainFrm.h"
#include "CANConfigDlg.h"
#include "LogFilterDlg.h"
#include "NSSearchDlg.h"
#include "SplashWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_BTN_CANCONFIG, &CMainFrame::OnBtnCanConfig)
	ON_COMMAND(ID_BTN_APPSTART, &CMainFrame::OnBtnAppStart)
	ON_COMMAND(ID_BTN_APPSTOP, &CMainFrame::OnBtnAppStop)
	ON_COMMAND(ID_BTN_APPDUMP, &CMainFrame::OnBtnAppDump)
	ON_COMMAND(ID_BTN_PCSSTART, &CMainFrame::OnBtnPcsStart)
	ON_COMMAND(ID_BTN_PCSSTOP, &CMainFrame::OnBtnPcsStop)
	ON_COMMAND(ID_BTN_PCSFAULTRESET, &CMainFrame::OnBtnPcsFaultReset)
	ON_COMMAND(ID_BTN_CANSTART, &CMainFrame::OnBtnCanStart)
	ON_COMMAND(ID_BTN_CANSTOP, &CMainFrame::OnBtnCanStop)
	ON_COMMAND(ID_BTN_CANSEARCH, &CMainFrame::OnBtnCanSearch)
	ON_COMMAND(ID_APP_ABOUT, &CMainFrame::OnAppAbout)
	ON_COMMAND(ID_APP_EXIT, &CMainFrame::OnAppExit)
	ON_COMMAND(ID_CHK_LOGBOX, &CMainFrame::OnChkLogBox)
	ON_UPDATE_COMMAND_UI(ID_CHK_LOGBOX, &CMainFrame::OnUpdateChkLogBox)
	ON_COMMAND(ID_CHK_PROCESSVIEW, &CMainFrame::OnChkProcessView)
	ON_UPDATE_COMMAND_UI(ID_CHK_PROCESSVIEW, &CMainFrame::OnUpdateChkProcessView)
	ON_COMMAND(ID_CHK_PROPERTIES, &CMainFrame::OnChkProperties)
	ON_UPDATE_COMMAND_UI(ID_CHK_PROPERTIES, &CMainFrame::OnUpdateChkProperties)
	ON_COMMAND(ID_BTN_LOGFILTER, &CMainFrame::OnBtnLogFilter)
	ON_COMMAND(ID_BTN_LOGCLEAR, &CMainFrame::OnBtnLogclear)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	/// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CSplashWnd::ShowSplashScreen(3000, IDB_SPLASH, this);

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE(_T("Failed to create docking windows\n"));
		return -1;
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE(_T("Failed to create status bar\n"));
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	m_wndProcessView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProcessView);

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);

	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (NS_OK != _chief.Create()) {
		_chief.Destroy();
		return 1;
	}
	_chief.SetLogPanelHnadle(&m_wndOutput);
	m_wndProcessView.SetCanManager(_chief.GetCanManager());

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	/// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs


	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	// Create Process view
	if (!m_wndProcessView.Create(_T("Process View"), this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CUSTOMIZE, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE(_T("Failed to create Process View window\n"));
		return FALSE; // failed to create
	}

	// Create LogBox Window
	if (!m_wndOutput.Create(_T("LogBox"), this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE(_T("Failed to create LogBox\n"));
		return FALSE; // failed to create
	}

	// Create CAN Config window
	if (!m_wndProperties.Create(_T("CAN Config"), this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	{
		TRACE(_T("Failed to create CAN Config window\n"));
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// base class does the real work

	if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	return TRUE;
}


void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}


void CMainFrame::OnDestroy()
{
	m_wndProcessView.Destroy();
	_chief.Destroy();

	CFrameWndEx::OnDestroy();

	/// TODO: Add your message handler code here

}


void CMainFrame::OnAppAbout()
{
}

void CMainFrame::OnAppExit()
{
	ASSERT(AfxGetMainWnd() != NULL);
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
}

void CMainFrame::OnBtnAppStart()
{
	/// TODO: Add your command handler code here
	
	_chief.Start();
}


void CMainFrame::OnBtnAppStop()
{
	/// TODO: Add your command handler code here
	BeginWaitCursor();
	_chief.Stop();
	EndWaitCursor();
}


void CMainFrame::OnBtnAppDump()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnLogFilter()
{
	/// TODO: 여기에 명령 처리기 코드를 추가합니다.
	CLogFilterDlg LogFilterDlg(&_chief);

	if (LogFilterDlg.DoModal() == IDOK) {
		_chief.ApplyLogFilter();
	}
}

void CMainFrame::OnBtnLogclear()
{
	m_wndOutput.LogClear();
}

void CMainFrame::OnBtnPcsStart()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnPcsStop()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnPcsFaultReset()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnCanStart()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnCanStop()
{
	/// TODO: Add your command handler code here
}


void CMainFrame::OnBtnCanSearch()
{
	CNSSearchDlg dlg;
	dlg.DoModal();
}

void CMainFrame::OnBtnCanConfig()
{
	/// TODO: Add your command handler code here

	CCANConfigDlg CanConfigDlg;

	CanConfigDlg.SetCurrentItem(
		m_wndProperties.GetPropValue(0, 0),
		m_wndProperties.GetPropValue(0, 1),
		m_wndProperties.GetPropValue(0, 2).iVal,
		m_wndProperties.GetPropValue(1, 0).iVal,
		m_wndProperties.GetPropValue(1, 1).iVal,
		m_wndProperties.GetPropValue(1, 2).iVal);

	if (CanConfigDlg.DoModal() == IDOK)	{
		m_wndProperties.SetPropValue(0, 0, CanConfigDlg.GetStrProtocol());
		m_wndProperties.SetPropValue(0, 1, CanConfigDlg.GetStrBPS());
		m_wndProperties.SetPropValue(0, 2, (CanConfigDlg.GetChkReset() == 0) ? (_variant_t)false : (_variant_t)true);
		m_wndProperties.SetPropValue(1, 0, (CanConfigDlg.GetChkBusOff() == 0) ? (_variant_t)false : (_variant_t)true);
		m_wndProperties.SetPropValue(1, 1, (CanConfigDlg.GetChkErrPsv() == 0) ? (_variant_t)false : (_variant_t)true);
		m_wndProperties.SetPropValue(1, 2, (CanConfigDlg.GetChkWarning() == 0) ? (_variant_t)false : (_variant_t)true);
	}
}

void CMainFrame::OnChkProcessView()
{
	/// TODO: Add your command handler code here
	m_wndProcessView.ShowPane(m_wndProcessView.IsVisible() ? FALSE : TRUE, FALSE, FALSE);
}


void CMainFrame::OnUpdateChkProcessView(CCmdUI *pCmdUI)
{
	/// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_wndProcessView.IsVisible());
}

void CMainFrame::OnChkLogBox()
{
	/// TODO: Add your command handler code here
	m_wndOutput.ShowPane(m_wndOutput.IsVisible() ? FALSE : TRUE, FALSE, FALSE);
}


void CMainFrame::OnUpdateChkLogBox(CCmdUI *pCmdUI)
{
	/// TODO: Add your command update UI handler code here

	pCmdUI->SetCheck(m_wndOutput.IsVisible());
}


void CMainFrame::OnChkProperties()
{
	/// TODO: Add your command handler code here
	m_wndProperties.ShowPane(m_wndProperties.IsVisible() ? FALSE : TRUE, FALSE, FALSE);
}


void CMainFrame::OnUpdateChkProperties(CCmdUI *pCmdUI)
{
	/// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_wndProperties.IsVisible());
}