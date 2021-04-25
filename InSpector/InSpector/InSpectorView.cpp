
// InSpectorView.cpp : implementation of the CInSpectorView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "InSpector.h"
#endif

#include "InSpectorDoc.h"
#include "InSpectorView.h"
#include "MainFrm.h"
#include "StaticTime.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// CInSpectorView

IMPLEMENT_DYNCREATE(CInSpectorView, CFormView)

BEGIN_MESSAGE_MAP(CInSpectorView, CFormView)
	// Standard printing commands
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CInSpectorView construction/destruction

CInSpectorView::CInSpectorView() : CFormView(CInSpectorView::IDD)
{
#ifdef CLOCK_THREAD
	_tid = 0;
	_hClockThread = INVALID_HANDLE_VALUE;
#endif
}

CInSpectorView::~CInSpectorView()
{
}

void CInSpectorView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PCS, m_pcsList);
	DDX_Control(pDX, IDC_LIST_FAULT, m_faultList);
	DDX_Control(pDX, IDC_LIST_DPM, m_dpmList);
	DDX_Control(pDX, IDC_LIST_CS, m_csList);
	//DDX_Control(pDX, IDC_LIST_CAN, m_canList);
	DDX_Control(pDX, IDC_STATIC_CLOCK, _uiClock);
}


BOOL CInSpectorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

void CInSpectorView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	_uiClock.SetColourFaded(RGB(0, 40, 0));

	Initialize();

#ifdef CLOCK_THREAD
	StartThread();
#else
	SetTimer(0, 100, NULL);
#endif
}

// CInSpectorView drawing

void CInSpectorView::OnDraw(CDC* /*pDC*/)
{
	CInSpectorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CInSpectorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CInSpectorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CInSpectorView diagnostics

#ifdef _DEBUG
void CInSpectorView::AssertValid() const
{
	CView::AssertValid();
}

void CInSpectorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CInSpectorDoc* CInSpectorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CInSpectorDoc)));
	return (CInSpectorDoc*)m_pDocument;
}
#endif //_DEBUG


/**
* @reamarks
*	리스트 컨트롤 초기화 및 쓰레드 생성 함수
* @author 김동현
* @date 2016/06/16
*/
void CInSpectorView::Initialize()
{
	COutputWnd* pOutput = NULL;

	// XListCtrl must have LVS_EX_FULLROWSELECT if combo or edit boxes are used
	DWORD dwExStyle = LVS_EX_FULLROWSELECT
					/*| LVS_EX_TRACKSELECT*/;	// for hot tracking

	dwExStyle |= LVS_EX_GRIDLINES;

	m_font.CreateFont(23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, 
              OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
              DEFAULT_PITCH | FF_SWISS, _T("맑은 고딕"));

	GetDlgItem(IDC_STATIC_PCS)->SetFont(&m_font);
	GetDlgItem(IDC_STATIC_PCSFAULT)->SetFont(&m_font);
	GetDlgItem(IDC_STATIC_DPM)->SetFont(&m_font);
	GetDlgItem(IDC_STATIC_CS)->SetFont(&m_font);
	GetDlgItem(IDC_STATIC_CAN)->SetFont(&m_font);

	m_pcsList.SetExtendedStyle(dwExStyle);
	m_pcsList.Init();
	m_faultList.SetExtendedStyle(dwExStyle);
	m_faultList.Init();
	m_dpmList.SetExtendedStyle(dwExStyle);
	m_dpmList.Init();
	m_csList.SetExtendedStyle(dwExStyle);
	m_csList.Init();
	//m_canList.SetExtendedStyle(dwExStyle);

	((CMainFrame*)AfxGetMainWnd())->GetChiefManager()->SetViewHandle(m_pcsList.GetSafeHwnd(), m_faultList.GetSafeHwnd(), m_dpmList.GetSafeHwnd(), m_csList.GetSafeHwnd());
}

void CInSpectorView::OnDestroy()
{
#ifdef CLOCK_THREAD
	EndThread();
#endif
	m_font.DeleteObject();
	CFormView::OnDestroy();
}

void CInSpectorView::OnTimer(UINT_PTR nIDEvent)
{
	_uiClock.DisplayCurrentTime();

	CFormView::OnTimer(nIDEvent);
}

#ifdef CLOCK_THREAD
int CInSpectorView::StartThread()
{
	_hClockThread = (HANDLE)_beginthreadex(NULL, 0, ClockThread, this, 0, &_tid);
	if (_hClockThread == INVALID_HANDLE_VALUE)
		return 1;

	_evt = CreateEvent(NULL, FALSE, FALSE, _T("clock_evt"));
	if( _evt == INVALID_HANDLE_VALUE )
		return 2;

	return 0;
}

void CInSpectorView::EndThread()
{
	DWORD dwExitCode = 0;
	SetEvent(_evt);
	if (INVALID_HANDLE_VALUE != _hClockThread) {
		WaitForSingleObject(_hClockThread, 50);
		GetExitCodeThread(_hClockThread, &dwExitCode);
		if (STILL_ACTIVE == dwExitCode) {
			TRACE(_T("Digital Clock Thread terminated...\n"));
			TerminateThread(_hClockThread, 0);
		}
		CloseHandle(_hClockThread);
		_hClockThread = INVALID_HANDLE_VALUE;
	}
}

void CInSpectorView::DrawClock()
{
	while (1) {
		if( WaitForSingleObject(_evt, 100) == WAIT_OBJECT_0)
			break;

		_uiClock.DisplayCurrentTime();
	}
}

unsigned int __stdcall CInSpectorView::ClockThread(void* param)
{
	ns_uint32 ret = 0;
	CInSpectorView* o = (CInSpectorView*)param;
	o->DrawClock();
	return ret;
}
#endif