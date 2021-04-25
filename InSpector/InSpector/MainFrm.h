
// MainFrm.h : interface of the CMainFrame class
//

#pragma once


#include "ProcessView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "NSChiefManager.h"


class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	CNSChiefManager* GetChiefManager() { return &_chief; }
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCRibbonBar     m_wndRibbonBar;
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CProcessView      m_wndProcessView;
	COutputWnd        m_wndOutput;
	CPropertiesWnd    m_wndProperties;

	CNSChiefManager	_chief;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnAppAbout();
	afx_msg void OnAppExit();
	afx_msg void OnDestroy();	
	afx_msg void OnBtnAppStart();
	afx_msg void OnBtnAppStop();
	afx_msg void OnBtnAppDump();
	afx_msg void OnBtnLogFilter();
	afx_msg void OnBtnPcsStart();
	afx_msg void OnBtnPcsStop();
	afx_msg void OnBtnPcsFaultReset();
	afx_msg void OnBtnCanStart();
	afx_msg void OnBtnCanStop();
	afx_msg void OnBtnCanSearch();
	afx_msg void OnBtnCanConfig();
	afx_msg void OnChkLogBox();
	afx_msg void OnUpdateChkLogBox(CCmdUI *pCmdUI);
	afx_msg void OnChkProcessView();
	afx_msg void OnUpdateChkProcessView(CCmdUI *pCmdUI);
	afx_msg void OnChkProperties();
	afx_msg void OnUpdateChkProperties(CCmdUI *pCmdUI);
	afx_msg void OnBtnLogclear();
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
};


