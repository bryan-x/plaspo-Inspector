#pragma once


class CNSChiefManager;


// CLogFilterDlg ��ȭ �����Դϴ�.

class CLogFilterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLogFilterDlg)

public:
	//CLogFilterDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	CLogFilterDlg(CNSChiefManager* chief, CWnd* pParent = NULL);
	virtual ~CLogFilterDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGFILTERDLG };
#endif

private:
	afx_msg void OnBnClickedOk();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
