#include "stdafx.h"
#include "NSLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4996)

#define DIR_LOG		_T("LOG")



CNSLog::CNSLog(LPCTSTR module)
{
	_hLog = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&_cs);
	_name = module;
	_pcs = 0;
}

CNSLog::CNSLog(LPCTSTR module, ns_uint8 instanceID)
{
	_hLog = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&_cs);
	_name = module;
	_pcs = instanceID;
}

CNSLog::~CNSLog()
{
	Destroy();
	DeleteCriticalSection(&_cs);
}

ns_uint32 CNSLog::Create()
{
	TCHAR dirName[256] = { 0, };
	CString fileName = _T("");
	CString strDate = _T("");
	time_t t;
	struct tm c;

	TCHAR szLog[256] = { 0 };

	t = time(NULL);
	localtime_s(&c, &t);

	CreateDir(DIR_LOG);

	strDate.Format(_T("%04d%02d%02d"), c.tm_year + 1900, c.tm_mon + 1, c.tm_mday);
	_stprintf_s(dirName, _countof(dirName), _T("%s\\%s"), DIR_LOG, strDate);
	CreateDir(dirName);

	if (_pcs > 0)
		fileName.Format(_T("%s\\%s%d_%s.txt"), dirName, _name, _pcs, strDate);
	else
		fileName.Format(_T("%s\\%s_%s.txt"), dirName, _name, strDate);

	_hLog = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);

	return 0;
}

ns_uint32 CNSLog::Destroy()
{
	if (_hLog != INVALID_HANDLE_VALUE) CloseHandle(_hLog);
	_hLog = INVALID_HANDLE_VALUE;
	return 0;
}

ns_uint32 CNSLog::WriteLog(const CString& strLog)
{
	ns_uint32 dwSize = 0;

	if (_hLog == INVALID_HANDLE_VALUE) {
		Create();
	}

	if (_hLog != INVALID_HANDLE_VALUE) {
		SetFilePointer(_hLog, 0, NULL, FILE_END);
		WriteFile(_hLog, strLog, strLog.GetLength() * sizeof(TCHAR), &dwSize, NULL);
	}

	return dwSize;
}

void CNSLog::CreateDir(TCHAR *szDir)
{
	TCHAR szDirName[256] = { 0 };
	TCHAR *p = szDir;
	TCHAR *q = szDirName;

	while (*p) {
		if ((_T('\\') == *p) || (_T('/') == *p)) {
			if (_T(':') != *(p - 1)) {
				CreateDirectory(CString(szDirName), NULL);
			}
		}
		*q++ = *p++;
		*q = _T('\0');
	}
	CreateDirectory(CString(szDirName), NULL);
}