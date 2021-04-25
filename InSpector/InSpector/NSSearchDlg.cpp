// NSSearchDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "InSpector.h"
#include "NSSearchDlg.h"
#include "afxdialogex.h"
//#include "CanMessage.h"


// CNSSearchDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CNSSearchDlg, CDialogEx)

CNSSearchDlg::CNSSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SEARCH_DIALOG, pParent)
{
	_hFile = INVALID_HANDLE_VALUE;
	_id = 0;
	_hour = 0;
	_min = 0;
	_sec = 0;
	_index = 0;
}

CNSSearchDlg::~CNSSearchDlg()
{
}

void CNSSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DAY, _comboDay);
	DDX_Control(pDX, IDC_LIST_SEARCH, _list);
}


BEGIN_MESSAGE_MAP(CNSSearchDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_MONTH, &CNSSearchDlg::OnCbnSelchangeComboMonth)
	ON_BN_CLICKED(IDC_BTN_SEARCH, &CNSSearchDlg::OnBnClickedBtnSearch)
END_MESSAGE_MAP()


// CNSSearchDlg 메시지 처리기입니다.


BOOL CNSSearchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	LVCOLUMN lvcomumn;
	lvcomumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcomumn.fmt = LVCFMT_CENTER;

	lvcomumn.cx = 40;
	lvcomumn.pszText = _T("No.");
	_list.InsertColumn(0, &lvcomumn);

	lvcomumn.cx = 50;
	lvcomumn.pszText = _T("ID");
	_list.InsertColumn(1, &lvcomumn);

	lvcomumn.cx = 80;
	lvcomumn.pszText = _T("TIME");
	_list.InsertColumn(2, &lvcomumn);

	lvcomumn.cx = 70;
	lvcomumn.pszText = _T("R_No.");
	_list.InsertColumn(3, &lvcomumn);

	lvcomumn.cx = 190;
	lvcomumn.pszText = _T("DATA");
	_list.InsertColumn(4, &lvcomumn);

	// 현재시간 기준 올해 연도 콤보박스에 추가 해야함.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CNSSearchDlg::OnCbnSelchangeComboMonth()
{
	int year, month, count;
	CString strDay;

	UpdateData(TRUE);
	month = GetDlgItemInt(IDC_COMBO_MONTH);
	year = GetDlgItemInt(IDC_COMBO_YEAR);
	count = _comboDay.GetCount();
	switch (month) {
	case 2:
		if ((year % 4 == 0) && (count > 29)) {
			_comboDay.DeleteString(29);	// delete 30
			_comboDay.DeleteString(29); // delete 31
		}
		else if (count > 28) {
			_comboDay.DeleteString(28);	// delete 29
			_comboDay.DeleteString(28);	// delete 30
			_comboDay.DeleteString(28); // delete 31
		}
		break;

	case 4:  
	case 5:  
	case 9:  
	case 11: 
		if (count > 30)
			_comboDay.DeleteString(30); // delete 31
		else if (count < 30) {
			while (count < 30) {
				strDay.Format(_T("%d"), ++count);
				_comboDay.AddString(strDay);
			}
		}
		break;

	default:
		while (count < 31) {
			strDay.Format(_T("%d"), ++count);
			_comboDay.AddString(strDay);
		}
		break;
	}
}

void CNSSearchDlg::OnBnClickedBtnSearch()
{
#if 0
	ns_uint32 id, year, month, day, hour, min, sec;
	WIN32_FIND_DATA FindFileData;
	TCHAR szDirPath[MAX_PATH] = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString strSearchDir, strFile, strContents;
	ns_uint32 fTime, fNo, fId;
	ns_uint8 buf[70];
	DWORD NumberOfBytesRead = 0;
	CanMessage m;

	GetCurrentDirectory(sizeof(szDirPath), szDirPath);
	strSearchDir.Format(_T("%s\\DUMP"), szDirPath);
	strSearchDir.Format(_T("%s\\dump_%d.%d.%d"), strSearchDir, year, month, day);
	strFile.Format(_T("%s\\*"), strSearchDir);
	hFind = FindFirstFile(strFile, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind) {
		MessageBox(_T("No serch file"));
	}
	else {
		while (FindNextFile(hFind, &FindFileData) != 0) {
			_stscanf(FindFileData.cFileName, _T("%d_%d(%d)"), &fTime, &fNo, &fId);
			if ((fTime == (hour * 10000 + min * 100 + sec)) && (fId == id)) {
				strFile.Format(_T("%s\\%s"), strSearchDir, FindFileData.cFileName);
				hFile = CreateFile(strFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (INVALID_HANDLE_VALUE == hFile) {
					MessageBox(_T("File open error"));
					break;;
				}
				memset(buf, 0, sizeof(buf));
				ReadFile(hFile, buf, sizeof(buf), &NumberOfBytesRead, NULL);
				memcpy(&m, buf, sizeof(CanMessage));
				{
					strContents.Format(_T("%d"), _index);
					_list.InsertItem(_index, strContents);
					strContents.Format(_T("%d"), m.id);
					_list.SetItemText(_index, 1, strContents);
					strContents.Format(_T("%2d:%2d:%2d"), m.c.tm_hour, m.c.tm_min, m.c.tm_sec);
					_list.SetItemText(_index, 2, strContents);
					strContents.Format(_T("%d"), m.no);
					_list.SetItemText(_index, 3, strContents);
					strContents.Format(_T("%d %d %d %d %d %d %d %d"),
						m.data[0], m.data[1], m.data[2], m.data[3], m.data[4], m.data[5], m.data[6], m.data[7]);
					_list.SetItemText(_index, 4, strContents);
					_index++;
				}
				CloseHandle(hFile);
			}
		}
		FindClose(hFind);
	}
#endif

	PARTITION origin;
	DWORD dwNumberOfBytesRead = 0;
	TCHAR dirPath[MAX_PATH] = { 0 };
	CString strFile = _T("");
	ns_int strLen = STRING_LEN;
	ns_uint16 year, month, day;
#ifdef EXT_STRING_LEN
	strLen = EXT_STRING_LEN;
#endif // EXT_STRING_LEN

	_index = 0;
	_list.DeleteAllItems();

	year = GetDlgItemInt(IDC_COMBO_YEAR);
	month = GetDlgItemInt(IDC_COMBO_MONTH);
	day = GetDlgItemInt(IDC_COMBO_DAY);
	_id = GetDlgItemInt(IDC_COMBO_ID);
	_hour = GetDlgItemInt(IDC_COMBO_HOUR);
	_min = GetDlgItemInt(IDC_COMBO_MINUTE);
	_sec = GetDlgItemInt(IDC_COMBO_SECOND);

	GetCurrentDirectory(MAX_PATH, dirPath);
	strFile.Format(_T("%s\\DUMP\\dump_%04d.%02d.%02d\\dump_%02d.cxt"), dirPath, year, month, day, _id);
	_hFile = CreateFile(strFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_hFile == INVALID_HANDLE_VALUE) {
		TRACE(_T("Search target file is not exist...\n"));
		return;
	}
	
	origin.top = SetFilePointer(_hFile, 0, NULL, FILE_BEGIN);
	origin.bottom = SetFilePointer(_hFile, -61, NULL, FILE_END);
	if ((origin.top == INVALID_SET_FILE_POINTER) || (origin.bottom == INVALID_SET_FILE_POINTER)) {
		TRACE(_T("%s(%d) SetFilePointer Error : %d \n"), __FILE__, __LINE__, GetLastError());
		return;
	}

	Search(origin);
	CloseHandle(_hFile);
}


ns_uint32 CNSSearchDlg::Divide(PARTITION& first, PARTITION& second, PARTITION& third, PARTITION& fourth)
{
	ns_uint32 height, row, interval, strLen = STRING_LEN;
#ifdef EXT_STRING_LEN
	strLen = EXT_STRING_LEN;
#endif
	height = fourth.bottom - first.top;
	row = (height / (strLen + MARGINE) + 1);
	if (row <= MIN_PARTITION_SIZE)
		return 1;

	interval = row / 4;
	second.top = first.top + (strLen + MARGINE)*interval;
	third.top = first.top + (strLen + MARGINE)*(interval * 2);
	fourth.top = first.top + (strLen + MARGINE)*(interval * 3);

	first.bottom = second.top - (strLen + MARGINE);
	second.bottom = third.top - (strLen + MARGINE);
	third.bottom = fourth.top - (strLen + MARGINE);

	return 0;
}

ns_uint32 CNSSearchDlg::Extract(LONG lDistanceToMove)
{
	ns_uint32 hour, min, sec, dummy;
	DWORD dwNumberOfBytesRead = 0;
	char buf[100] = { 0 };
	//memset(buf, 0, sizeof(buf));
	if (SetFilePointer(_hFile, lDistanceToMove, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		TRACE(_T("%s(%d) SetFilePointer Error : %d \n"), __FILE__, __LINE__, GetLastError());
		return 0;
	}

#ifdef EXT_STRING_LEN
	ReadFile(_hFile, (LPVOID)buf, 19, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead != 19)
		TRACE(_T("%s(%d) NumberOfBytesToRead != NumberOfBytesRead... \n"), __FILE__, __LINE__);
	sscanf_s(buf, "%d:%d:%d %d:%d:%d", &dummy, &dummy, &dummy, &hour, &min, &sec);
#else
	ReadFile(_hFile, (LPVOID)buf, 8, &dwNumberOfBytesRead, NULL);
	if (dwNumberOfBytesRead != 8)
		TRACE(_T("%s(%d) NumberOfBytesToRead != NumberOfBytesRead... \n"), __FILE__, __LINE__);
	sscanf_s(buf, "%d:%d:%d", &hour, &min, &sec);
#endif
	return (unsigned long)(hour * 10000 + min * 100 + sec);
}

ns_uint32 CNSSearchDlg::Search(const PARTITION& p)
{
	PARTITION first, second, third, fourth;
	ns_uint32 findVal = 0;

	findVal = (_hour * 10000) + (_min * 100) + _sec;
	// 해당 구간에 찾는 데이터 없음
	if (Extract(p.top) > findVal || Extract(p.bottom) < findVal)
		return 1;

	first.top = p.top;
	fourth.bottom = p.bottom;
	// 구간을 나누기에는 범위가 좁음
	if (Divide(first, second, third, fourth) == 1) {
		AddListItem(p);
	} else {
		Search(first);
		Search(second);
		Search(third);
		Search(fourth);
	}
	return 0;
}

ns_uint32 CNSSearchDlg::AddListItem(const PARTITION& p)
{
	ns_uint32 year, month, day, hour, min, sec, no, ts, dlc, data[8], strLen = STRING_LEN;
	DWORD dwNumberOfBytesRead = 0;
	char buf[100] = { 0 };
	CString strContents = _T("");
	DWORD i = 0;
#ifdef EXT_STRING_LEN
	strLen = EXT_STRING_LEN;
#endif

	for (i = p.top; i <= p.bottom; i += (strLen + MARGINE)) {
		memset(buf, 0, sizeof(buf));
		if (SetFilePointer(_hFile, i, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			TRACE(_T("%s(%d) SetFilePointer Error : %d \n"), __FILE__, __LINE__, GetLastError());
			continue;
		}
		ReadFile(_hFile, (LPVOID)buf, (strLen+MARGINE), &dwNumberOfBytesRead, NULL);
		if (dwNumberOfBytesRead != (strLen + MARGINE)) {
			TRACE(_T("%s(%d) NumberOfBytesToRead != NumberOfBytesRead... \n"), __FILE__, __LINE__);
			continue;
		}
		sscanf_s(buf, "%04d:%02d:%02d %02d:%02d:%02d %08d %06d %d %02X %02X %02X %02X %02X %02X %02X %02X", \
			&year, &month, &day, &hour, &min, &sec, &no, &ts, &dlc, \
			&data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7]);

		if ((_hour == hour) && (_min == min) && (_sec == sec)) {
			strContents.Format(_T("%d"), _index);
			_list.InsertItem(_index, strContents);
			strContents.Format(_T("%d"), _id);
			_list.SetItemText(_index, 1, strContents);
			strContents.Format(_T("%02d:%02d:%02d"), hour, min, sec);
			_list.SetItemText(_index, 2, strContents);
			strContents.Format(_T("%d"), no);
			_list.SetItemText(_index, 3, strContents);
			strContents.Format(_T("%02X %02X %02X %02X %02X %02X %02X %02X"),
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
			_list.SetItemText(_index, 4, strContents);
			_index++;
		}
	}
	return 0;
}
