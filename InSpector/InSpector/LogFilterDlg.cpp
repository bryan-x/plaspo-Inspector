// LogFilterDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "InSpector.h"
#include "LogFilterDlg.h"
#include "afxdialogex.h"
#include "NSChiefManager.h"


// CLogFilterDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CLogFilterDlg, CDialogEx)

//CLogFilterDlg::CLogFilterDlg(CWnd* pParent /*=NULL*/)
//	: CDialogEx(IDD_LOGFILTERDLG, pParent)
//	, _bCan(TRUE)
//	, _bMc(TRUE)
//	, _bPcs1(TRUE)
//	, _bPcs2(TRUE)
//	, _bFault(TRUE)
//	, _bTraceDump(TRUE)
//	, _bService(TRUE)
//	, _bInform(TRUE)
//	, _bMemory(TRUE)
//	, _bIpc(TRUE)
//	, _bException(TRUE)
//{
//}

CLogFilterDlg::CLogFilterDlg(CNSChiefManager* chief, CWnd* pParent)
	: CDialogEx(IDD_LOGFILTERDLG, pParent)
{
	ns_uint16 logFilter = 0;
	_chief = chief;
	logFilter = _chief->GetLogFilter();

	_bCan = logFilter & 0x1;
	_bMc = (logFilter >> 1) & 0x1;
	_bPcs1 = (logFilter >> 2) & 0x1;
	_bPcs2 = (logFilter >> 3) & 0x1;
	_bFault = (logFilter >> 4) & 0x1;
	_bTraceDump = (logFilter >> 5) & 0x1;
	_bService = (logFilter >> 6) & 0x1;
	_bInform = (logFilter >> 7) & 0x1;
	_bMemory = (logFilter >> 8) & 0x1;
	_bIpc = (logFilter >> 9) & 0x1;
	_bException = (logFilter >> 10) & 0x1;
}

CLogFilterDlg::~CLogFilterDlg()
{
}

void CLogFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_CANLOG, _bCan);
	DDX_Check(pDX, IDC_CHK_MCLOG, _bMc);
	DDX_Check(pDX, IDC_CHK_PCS1LOG, _bPcs1);
	DDX_Check(pDX, IDC_CHK_PCS2LOG, _bPcs2);
	DDX_Check(pDX, IDC_CHK_FAULTLOG, _bFault);
	DDX_Check(pDX, IDC_CHK_TRACEDUMPLOG, _bTraceDump);
	DDX_Check(pDX, IDC_CHK_SERVICELOG, _bService);
	DDX_Check(pDX, IDC_CHK_INFORMLOG, _bInform);
	DDX_Check(pDX, IDC_CHK_MEMLOG, _bMemory);
	DDX_Check(pDX, IDC_CHK_IPCLOG, _bIpc);
	DDX_Check(pDX, IDC_CHK_EXCEPTLOG, _bException);
}


BEGIN_MESSAGE_MAP(CLogFilterDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLogFilterDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLogFilterDlg 메시지 처리기입니다.


void CLogFilterDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	ns_uint16 filterFlag = 0;

	UpdateData(TRUE);

	filterFlag |= _bCan;
	filterFlag |= (_bMc << 1);
	filterFlag |= (_bPcs1 << 2);
	filterFlag |= (_bPcs2 << 3);
	filterFlag |= (_bFault << 4);
	filterFlag |= (_bTraceDump << 5);
	filterFlag |= (_bService << 6);
	filterFlag |= (_bInform << 7);
	filterFlag |= (_bMemory << 8);
	filterFlag |= (_bIpc << 9);
	filterFlag |= (_bException << 10);

	_chief->SetLogFilter(filterFlag);

	CDialogEx::OnOK();
}