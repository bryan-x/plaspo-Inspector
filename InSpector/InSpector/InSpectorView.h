
// InSpectorView.h : interface of the CInSpectorView class
//

#pragma once

#include "resource.h"
#include "NSPcsListCtrl.h"
#include "NSFaultListCtrl.h"
#include "NSDpmListCtrl.h"
#include "NSCsListCtrl.h"
#include "StaticTime.h"


//#define CLOCK_THREAD


class CInSpectorView : public CFormView
{
public:
	virtual ~CInSpectorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CInSpectorDoc* GetDocument() const;
	afx_msg void OnTimer(UINT_PTR nIDEvent);

protected: // create from serialization only
	CInSpectorView();
	DECLARE_DYNCREATE(CInSpectorView)

	// Generated message map functions
	afx_msg void OnDestroy();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
#ifdef CLOCK_THREAD
	int StartThread();
	void EndThread();
	static unsigned int __stdcall ClockThread(void *param);
	void DrawClock();
#endif
	void Initialize();

	// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

public:
	enum { IDD = IDD_INSPECTOR_FORM };

private:
#ifdef CLOCK_THREAD
private:
	unsigned int _tid;
	HANDLE _hClockThread;
	HANDLE _evt;
#endif
	CFont		m_font;
	CStaticTime _uiClock;

	CNSPcsListCtrl m_pcsList;
	CNSFaultListCtrl m_faultList;
	CNSDpmListCtrl m_dpmList;
	CNSCsListCtrl m_csList;
	//CXListCtrl	m_canList;
};

#ifndef _DEBUG  // debug version in InSpectorView.cpp
inline CInSpectorDoc* CInSpectorView::GetDocument() const
   { return reinterpret_cast<CInSpectorDoc*>(m_pDocument); }
#endif

