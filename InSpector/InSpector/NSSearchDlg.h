#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#define MIN_PARTITION_SIZE	1000/50		// 1000ms / data interval
#define STRING_LEN 50
#define EXT_STRING_LEN	61
#define MARGINE	2

struct PARTITION
{
	DWORD top;
	DWORD bottom;
};

class CNSSearchDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNSSearchDlg)

public:
	CNSSearchDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CNSSearchDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEARCH_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboMonth();
	afx_msg void OnBnClickedBtnSearch();

	ns_uint32 Divide(PARTITION& first, PARTITION& second, PARTITION& third, PARTITION& fourth);
	ns_uint32 Extract(LONG lDistanceToMove);
	ns_uint32 Search(const PARTITION& p);
	ns_uint32 AddListItem(const PARTITION& p);

private:
	ns_uint32 _index;
	CListCtrl _list;
	CComboBox _comboDay;

	HANDLE _hFile;
	ns_uint8 _id, _hour, _min, _sec;
};
