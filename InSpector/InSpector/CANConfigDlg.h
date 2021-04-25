#pragma once
#include "afxwin.h"


// CCANConfigDlg dialog

class CCANConfigDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCANConfigDlg)

private:
	CString		m_strProtocol;			/**< Properties Wnd�� ������ CAN Protocol */
	CString		m_strBPS;				/**< Properties Wnd�� ������ CAN BPS */
	BOOL		m_bChkReset;			/**< Properties Wnd�� ������ CAN Auto Reset Flag */
	BOOL		m_bChkBusOff;			/**< Properties Wnd�� ������ CAN Bus-Off Error Flag */
	BOOL		m_bChkErrPsv;			/**< Properties Wnd�� ������ CAN Error-Passive Error Flag */
	BOOL		m_bChkWarning;			/**< Properties Wnd�� ������ CAN Warning Error Flag */

	CComboBox	m_ComboProtocol;		/**< CAN Protocol �޺��ڽ� ��Ʈ�� ���� */
	CComboBox	m_ComboBPS;				/**< CAN BPS �޺��ڽ� ��Ʈ�� ���� */
	CButton		m_chkCanReset;			/**< CAN Auto Reset üũ�ڽ� ��Ʈ�� ���� */
	CButton		m_chkBusOffError;		/**< CAN Bus-off Error üũ�ڽ� ��Ʈ�� ���� */
	CButton		m_chkErrorPassive;		/**< CAN Error-Passive Error üũ�ڽ� ��Ʈ�� ���� */
	CButton		m_chkWarningError;		/**< CAN Warning Error üũ�ڽ� ��Ʈ�� ���� */

// Dialog Data
	enum { IDD = IDD_CANCONFIGDLG };

private:
	void SearchAndSetStringItem();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CCANConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCANConfigDlg();
	virtual BOOL OnInitDialog();
	void SetCurrentItem(const CString& protocol, const CString& BPS, BOOL bChkReset, BOOL bChkBusOff, BOOL bChkErrPsv, BOOL bChkWarning);
	afx_msg void OnBnClickedOk();

	CString& GetStrProtocol() { return m_strProtocol; }
	CString& GetStrBPS() { return m_strBPS; }
	BOOL GetChkReset() const { return m_bChkReset; }
	BOOL GetChkBusOff() const { return m_bChkBusOff; }
	BOOL GetChkErrPsv() const { return m_bChkErrPsv; }
	BOOL GetChkWarning() const { return m_bChkWarning; }
};
