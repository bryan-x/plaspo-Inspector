/**
* @file ProcessView.h
* @author 김동현
* @date 2016/06/30
* @brief 
*	CAN ID 통계치와 어플리케이션의 CPU 사용량을 UI에 그래프로 표현
*/

#pragma once

#include "cpuu.h"
#include <list>

class CNSCanManager;
class CPUUsage;


class CProcessView : public CDockablePane
{
public:
	CProcessView();
	virtual ~CProcessView();

	void Destroy();
	void SetCanManager(CNSCanManager* can) { _can = can; }

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	static unsigned int __stdcall ViewThreadProc(void *param);

	void InitDrawResource();
	void Draw();
	void DrawIDShare(CPaintDC& dc, CRect rect);
	void DrawBar(CPaintDC&, CRect& rect, int nBars, int shift);
	void DrawCpuUsage(CPaintDC& dc, CRect rect);
	void DrawLineChart(CPaintDC& dc, const CRect& rect);

private:
	HANDLE	m_hThread;				/**< 쓰레드 핸들 */
	HANDLE	_evt;
	CNSCanManager* _can;

	CPen	m_penSolidGreen;
	CPen	m_penSolidYellow;
	CPen	m_penSolidDarkGreen;
	CPen	m_penDottedGreen;

	BOOL	m_bResize;				/**< View 크기 변경 신호 플래그 */
	int		m_barOffset;			/**< 막대 그래프 세로 간격 */
	int		m_textHeight;			/**< 문자 영역 높이 */

	ns_uint16	m_msgCnt[3];		/**< CAN ID 갯수(인덱스 0: MC, 1:PCS1, 2:PCS2) */

	CPUUsage	_cpu;				/**< CPU 사용 정보 */
	std::list<short> m_CpuUsageList;	/**< 프로세스의 CPU 사용량 리스트 */
};

