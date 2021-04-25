#pragma once


class CNSChiefManager;


// CLogFilterDlg 대화 상자입니다.

class CLogFilterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogFilterDlg)

public:
	//CLogFilterDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	CLogFilterDlg(CNSChiefManager* chief, CWnd* pParent = NULL);
	virtual ~CLogFilterDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGFILTERDLG };
#endif

private:
	afx_msg void OnBnClickedOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CNSChiefManager* _chief;
	BOOL _bCan;
	BOOL _bMc;
	BOOL _bPcs1;
	BOOL _bPcs2;
	BOOL _bFault;
	BOOL _bTraceDump;
	BOOL _bService;
	BOOL _bInform;
	BOOL _bMemory;
	BOOL _bIpc;
	BOOL _bException;
};
