
#include "stdafx.h"
#include "ProcessView.h"
#include "NSCanManager.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CProcessView::CProcessView()
{
	m_hThread = INVALID_HANDLE_VALUE;
	_evt = INVALID_HANDLE_VALUE;

	m_barOffset	= 3;
	m_textHeight = 15;

	m_bResize = 1;
	m_msgCnt[0] = 0;
	m_msgCnt[1] = 0;
	m_msgCnt[2] = 0;

	_can = NULL;
}

CProcessView::~CProcessView()
{
}

BEGIN_MESSAGE_MAP(CProcessView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


int CProcessView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitDrawResource();

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, ViewThreadProc, this, 0, NULL);
	_evt = CreateEvent(NULL, FALSE, FALSE, _T("ProcView_evt"));
	if( m_hThread == INVALID_HANDLE_VALUE ) {
		TRACE(_T("ProcessView Create Trhead Fail...\n"));
		return 1;
	}

	return 0;
}

void CProcessView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	m_bResize = 1;
	Invalidate();
}

void CProcessView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect viewRect;
	GetClientRect(viewRect);

	if( m_bResize )
	{
		dc.FillSolidRect(viewRect, RGB(255, 255, 255));
		m_bResize = 0;
	}

	DrawIDShare(dc, viewRect);
	DrawCpuUsage(dc, viewRect);
}

void CProcessView::Destroy()
{
	DWORD dwExitCode = 0;

	SetEvent(_evt);
	if( m_hThread != INVALID_HANDLE_VALUE ) {
		WaitForSingleObject(m_hThread, 50);
		GetExitCodeThread(m_hThread, &dwExitCode);
		if(dwExitCode == STILL_ACTIVE) {
			TerminateThread(m_hThread, 0);
			TRACE(_T("ProcView Thread = terminated...\n"));
		}
		CloseHandle(m_hThread);
		m_hThread = INVALID_HANDLE_VALUE;
	}
	CloseHandle(_evt);
	_evt = INVALID_HANDLE_VALUE;
}

void CProcessView::OnDestroy()
{
	CDockablePane::OnDestroy();
}

/**	
* @reamarks
*	드로잉 리소스 초기화 함수
* @author 김동현
* @date 2016/06/30
*/
void CProcessView::InitDrawResource()
{
	LOGBRUSH LogBrush;
    LogBrush.lbColor = RGB(0, 128, 64);
    LogBrush.lbStyle = PS_SOLID;
	m_penDottedGreen.CreatePen(PS_COSMETIC | PS_ALTERNATE , 1, &LogBrush, 0, NULL);
	m_penSolidGreen.CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	m_penSolidYellow.CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	m_penSolidDarkGreen.CreatePen(PS_SOLID, 1, RGB(0, 128, 64));
}

unsigned int __stdcall CProcessView::ViewThreadProc(void* param)
{
	CProcessView* o = (CProcessView*)param;
	o->Draw();
	return 0;
}

/**	
* @reamarks
*	CAN ID 통계와 CPU 사용량을 체크하여 그리기 함수 호출
* @author 김동현
* @date 2016/07/01
*/
void CProcessView::Draw()
{
	ns_uint sizeLimit = 100;

	while(1) {
		if( WaitForSingleObject(_evt, 1000) == WAIT_OBJECT_0 )
			break;

		m_CpuUsageList.push_front(_cpu.usage());
		if( m_CpuUsageList.size() > sizeLimit )
			m_CpuUsageList.pop_back();
		// 메시지 카운트 받음
		if ( _can)	_can->GetMsgCount(m_msgCnt);

		Invalidate();
	}
}

/**	
* @reamarks
*	Master Controller, PCS1, PCS2의 CAN ID를 수집된 양에 따라 백분율로 표현
* @author 김동현
* @date 2016/06/30
* @param CPaintDC& dc	디바이스 컨텍스트 참조 변수			
* @param CRect viewRect	그리기 영역	
*/
void CProcessView::DrawIDShare(CPaintDC& dc, CRect viewRect)
{
	CRect rect[3] = {
		CRect(viewRect.left,		viewRect.top,	viewRect.right / 3,	viewRect.bottom / 2),
		CRect(viewRect.right / 3,		viewRect.top,	viewRect.right * 2 / 3, viewRect.bottom / 2),
		CRect(viewRect.right * 2 / 3,	viewRect.top,	viewRect.right,		viewRect.bottom / 2),
	};

	int i, w;
	double nTotal = m_msgCnt[0] + m_msgCnt[1] + m_msgCnt[2];
	double p = 0;
	int barHeight, nTotalBars, nPaintedBars;
	CRect titleRect;
	CRect meterRect;
	CRect bgrRect;
	CString strTitle = _T("%");
	CString strMeter = _T("%");

	dc.SetBkMode(TRANSPARENT);

	for (i = 0; i < sizeof(rect) / sizeof(CRect); i++) {
		w = rect[i].Width() / 20;

		if( w < 1 )
			return;

		titleRect = rect[i];
		titleRect.top += 4;
		titleRect.bottom = titleRect.top + m_textHeight;

		rect[i].top += m_textHeight;
		rect[i].DeflateRect(w, 10);
		bgrRect = rect[i];
		rect[i].DeflateRect(3,3);

		p = (nTotal > 0) ? (m_msgCnt[i] / nTotal) : 0;
		strMeter.Format(_T("%.1f "), p * 100);
		strMeter.Append(_T("%"));
		meterRect = rect[i];
		meterRect.top = meterRect.bottom - m_textHeight;

		rect[i].DeflateRect(rect[i].Width()/6, 0);
		rect[i].top += 10;
		rect[i].bottom = rect[i].top + 2;

		barHeight = meterRect.top - rect[i].top - 6;	// 6 : 간격

		nTotalBars = barHeight / m_barOffset;
		if (nTotalBars < m_barOffset)
			return;
		
		// draw background
		dc.Draw3dRect(bgrRect, RGB(128, 128, 128), RGB(128, 128, 128));
		bgrRect.DeflateRect(3,3);
		dc.FillSolidRect(bgrRect, RGB(0, 0, 0));

		switch (i) {
		case 0: strTitle = _T("MC"); break;
		case 1: strTitle = _T("PCS1"); break;
		case 2: strTitle = _T("PCS2"); break;
		}
		// draw title
		dc.SetTextColor(RGB(0, 0, 0));
		dc.DrawText(strTitle, titleRect, DT_SINGLELINE | DT_TOP | DT_CENTER);

		// draw meter
		dc.SetTextColor(RGB(0, 255, 0));
		dc.DrawText(strMeter, meterRect, DT_SINGLELINE | DT_TOP | DT_CENTER);

		
		nPaintedBars = (int)(nTotalBars * p);

		// draw chart
		dc.SelectObject(m_penDottedGreen);
		DrawBar(dc, rect[i], nTotalBars - nPaintedBars, 1);

		dc.SelectObject(m_penSolidGreen);
		DrawBar(dc, rect[i], nPaintedBars, 0);
	}
}

/**	
* @reamarks
*	수평 막대 그래프를 그리는 함수
* @author 김동현
* @date 2016/06/30
* @param CPaintDC& dc	디바이스 컨텍스트 참조 변수			
* @param CRect& rect	그리기 영역
* @param int nBars		수평 막대그래프의 수
* @param int shift		픽셀 이동 간격
*/
void CProcessView::DrawBar(CPaintDC& dc, CRect& rect, int nBars, int shift)
{
	int middle = rect.left + rect.Width()/2;

	for (int i = 0; i < nBars; i++) 
	{
		dc.MoveTo(rect.left, rect.top);
		dc.LineTo(middle-1, rect.top);
		dc.MoveTo(middle+1, rect.top);
		dc.LineTo(rect.right, rect.top);

		dc.MoveTo(rect.left+shift, rect.top+1);
		dc.LineTo(middle-1, rect.top+1);
		dc.MoveTo(middle+1+shift, rect.top+1);
		dc.LineTo(rect.right, rect.top+1);

		rect.OffsetRect(0, m_barOffset);
	}
}

/**	
* @reamarks
*	CPU 사용량을 차트 형식으로 표현
* @author 김동현
* @date 2016/06/30
* @param CPaintDC& dc	디바이스 컨텍스트 참조 변수			
* @param CRect& rect	그리기 영역
*/
void CProcessView::DrawCpuUsage(CPaintDC& dc, CRect rect)
{
	int gridOffset = 12;
	CRect titleRect;
	rect.top = rect.bottom/2;
	titleRect = CRect(rect.left + 10, rect.top + 4, rect.right, rect.bottom + m_textHeight);

	dc.SetTextColor(RGB(0, 0, 0));
	dc.DrawText(_T("CPU Usage"), titleRect, DT_SINGLELINE | DT_TOP | DT_LEFT);

	rect.top += m_textHeight;

	rect.DeflateRect(10, 10);
	dc.Draw3dRect(rect, RGB(128, 128, 128), RGB(128, 128, 128));
	rect.DeflateRect(3,3);
	dc.FillSolidRect(rect, RGB(0, 0, 0));

	dc.SelectObject(m_penSolidDarkGreen);

	for (int i = rect.bottom - gridOffset; i > rect.top; i = i - gridOffset)
	{
		dc.MoveTo(rect.left, i);
		dc.LineTo(rect.right, i);
	}

	static int leadingTick = 0;
	leadingTick = (leadingTick++) %7;

	for (int i = rect.right - (leadingTick * 2); i > rect.left; i = i - gridOffset)
	{
		dc.MoveTo(i, rect.top);
		dc.LineTo(i, rect.bottom);
	}

	DrawLineChart(dc, rect);
}

/**	
* @reamarks
*	차트 그리기 함수
* @author 김동현
* @date 2016/07/04
* @param CPaintDC& dc		디바이스 컨텍스트 참조 변수			
* @param const CRect& rect	그리기 영역
*/
void CProcessView::DrawLineChart(CPaintDC& dc, const CRect& rect)
{
	int dataUnit = 2;
	int drawIndex = rect.right;
	int prev = -1;

	dc.SelectObject(m_penSolidYellow);

	if (!m_CpuUsageList.empty()) {
		for (auto it = m_CpuUsageList.begin(); it != m_CpuUsageList.end(); it++) {
			if (drawIndex > rect.left) {
				if (prev < 0)
					dc.MoveTo(drawIndex, rect.bottom - rect.Height() * (*it) / 100);
				else
					dc.LineTo(drawIndex, rect.bottom - rect.Height() * (*it) / 100);

				drawIndex -= dataUnit;
				dc.LineTo(drawIndex, rect.bottom - rect.Height() * (*it) / 100);
				prev = (*it);
			}
		}
	}
}