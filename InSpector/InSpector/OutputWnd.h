
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window

#define WM_LOGBOX_UPDATE (WM_USER + MSGID_LOG)

class COutputList : public CListBox
{
// Construction
public:
	COutputList();

// Implementation
public:
	virtual ~COutputList();
	//void Log(LPCTSTR lpszLog);
	void Clear();

protected:

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();
	afx_msg LRESULT OnLog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};



class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();
	virtual ~COutputWnd();

	void UpdateFonts();

	HWND GetPcs1LogBox() const { return _pcs1LogBox.GetSafeHwnd(); }
	HWND GetPcs2LogBox() const { return _pcs2LogBox.GetSafeHwnd(); }
	HWND GetMcLogBox() const { return _mcLogBox.GetSafeHwnd(); }
	HWND GetCanLogBox() const { return _canLogBox.GetSafeHwnd(); }

	void LogClear();

protected:
	void AdjustHorzScroll(CListBox& wndListBox);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()


	// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;

	COutputList _pcs1LogBox;
	COutputList _pcs2LogBox;
	COutputList _mcLogBox;
	COutputList _canLogBox;
};

