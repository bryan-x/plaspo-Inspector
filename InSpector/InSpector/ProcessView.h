/**
* @file ProcessView.h
* @author �赿��
* @date 2016/06/30
* @brief 
*	CAN ID ���ġ�� ���ø����̼��� CPU ��뷮�� UI�� �׷����� ǥ��
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
	HANDLE	m_hThread;				/**< ������ �ڵ� */
	HANDLE	_evt;
	CNSCanManager* _can;

	CPen	m_penSolidGreen;
	CPen	m_penSolidYellow;
	CPen	m_penSolidDarkGreen;
	CPen	m_penDottedGreen;

	BOOL	m_bResize;				/**< View ũ�� ���� ��ȣ �÷��� */
	int		m_barOffset;			/**< ���� �׷��� ���� ���� */
	int		m_textHeight;			/**< ���� ���� ���� */

	ns_uint16	m_msgCnt[3];		/**< CAN ID ����(�ε��� 0: MC, 1:PCS1, 2:PCS2) */

	CPUUsage	_cpu;				/**< CPU ��� ���� */
	std::list<short> m_CpuUsageList;	/**< ���μ����� CPU ��뷮 ����Ʈ */
};

