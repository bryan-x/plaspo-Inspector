// CANConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InSpector.h"
#include "CANConfigDlg.h"
#include "afxdialogex.h"


// CCANConfigDlg dialog

IMPLEMENT_DYNAMIC(CCANConfigDlg, CDialogEx)

CCANConfigDlg::CCANConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCANConfigDlg::IDD, pParent)
	, m_strProtocol(_T(""))
	, m_strBPS(_T(""))
	, m_bChkReset(FALSE)
	, m_bChkBusOff(FALSE)
	, m_bChkErrPsv(FALSE)
	, m_bChkWarning(FALSE)
{

}

CCANConfigDlg::~CCANConfigDlg()
{
}

void CCANConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_ComboProtocol);
	DDX_Control(pDX, IDC_COMBO_BPS, m_ComboBPS);
	DDX_Control(pDX, IDC_CHECK_CANRESET, m_chkCanReset);
	DDX_Control(pDX, IDC_CHECK_BUSOFFERROR, m_chkBusOffError);
	DDX_Control(pDX, IDC_CHECK_ERRORPASSIVE, m_chkErrorPassive);
	DDX_Control(pDX, IDC_CHECK_WARNINGERROR, m_chkWarningError);
	DDX_CBString(pDX, IDC_COMBO_PROTOCOL, m_strProtocol);
	DDX_CBString(pDX, IDC_COMBO_BPS, m_strBPS);
	DDX_Check(pDX, IDC_CHECK_CANRESET, m_bChkReset);
	DDX_Check(pDX, IDC_CHECK_BUSOFFERROR, m_bChkBusOff);
	DDX_Check(pDX, IDC_CHECK_ERRORPASSIVE, m_bChkErrPsv);
	DDX_Check(pDX, IDC_CHECK_WARNINGERROR, m_bChkWarning);
}


BEGIN_MESSAGE_MAP(CCANConfigDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCANConfigDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCANConfigDlg message handlers


BOOL CCANConfigDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	SearchAndSetStringItem();

	m_chkCanReset.SetCheck(m_bChkReset);
	m_chkBusOffError.SetCheck(m_bChkBusOff);
	m_chkErrorPassive.SetCheck(m_bChkErrPsv);
	m_chkWarningError.SetCheck(m_bChkWarning);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


/**
* @reamarks
*	Properties Window와 공유하는 CAN Config 정보를 다이얼로그에 전달
* @author 김동현
* @date 2016/06/22
* @see
*	컨트롤 변수에서는 TRUE 값으로 음수를 허용하지 않는 것으로 보임.
*/
void CCANConfigDlg::SetCurrentItem(const CString& protocol, const CString& BPS, BOOL bChkReset, BOOL bChkBusOff, BOOL bChkErrPsv, BOOL bChkWarning)
{
	m_strProtocol	= protocol;
	m_strBPS		= BPS;
	m_bChkReset		= (bChkReset< 0)? 1 : 0;
	m_bChkBusOff	= (bChkBusOff < 0)? 1 : 0;
	m_bChkErrPsv	= (bChkErrPsv < 0)? 1 : 0;
	m_bChkWarning	= (bChkWarning < 0)? 1 : 0;
}


/**
* @reamarks
*	Properties Window에 설정된 CAN Protocol과 BPS 정보를 다이얼로그의 콤보박스 아이템중에 찾아서 초기 아이템으로 설정
* @author 김동현
* @date 2016/06/22
*/
void CCANConfigDlg::SearchAndSetStringItem()
{
	int i;
	CString str;

	for(i=0; i<m_ComboProtocol.GetCount(); i++)
	{
		m_ComboProtocol.GetLBText(i, str);

		if( str.Compare(m_strProtocol) == 0)
		{
			m_ComboProtocol.SetCurSel(i);
			break;
		}
	}

	for(i=0; i<m_ComboBPS.GetCount(); i++)
	{
		m_ComboBPS.GetLBText(i, str);

		if( str.Compare(m_strBPS) == 0)
		{
			m_ComboBPS.SetCurSel(i);
			break;
		}
	}
}

void CCANConfigDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	UpdateData(TRUE);

	CDialogEx::OnOK();
}
