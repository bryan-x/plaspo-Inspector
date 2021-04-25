#include "stdafx.h"
#include "NSPcsFaultLog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4996)

#define DIR_FAULT	_T("FAULT")


CNSPcsFaultLog::CNSPcsFaultLog(ns_uint8 instanceID)
{
	_hFile = INVALID_HANDLE_VALUE;
	_pcs = instanceID;
}

CNSPcsFaultLog::~CNSPcsFaultLog()
{
	Destroy();
}

ns_uint32 CNSPcsFaultLog::Create(const CString& strType)
{
	ns_uint32 ret = 0;
	CString strFileName = _T("");
	time_t t;
	struct tm c;

	CreateDir(DIR_FAULT);

	t = time(NULL);
	localtime_s(&c, &t);


	strFileName.Format(_T("%s\\PCS%d_%04d%02d%02d_%02d%02d%02d_%s.txt"),
		DIR_FAULT, _pcs,
		c.tm_year + 1900, c.tm_mon + 1, c.tm_mday,
		c.tm_hour, c.tm_min, c.tm_sec,
		strType
	);

	_hFile = CreateFile(
		strFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == _hFile) {
		CloseHandle(_hFile);
		return 1;
	}

	return ret;
}

ns_uint32 CNSPcsFaultLog::Destroy()
{
	if (_hFile != INVALID_HANDLE_VALUE) 
		CloseHandle(_hFile);

	_hFile = INVALID_HANDLE_VALUE;

	return 0;
}

ns_uint32 CNSPcsFaultLog::WriteLog(const CString& strType, const CString& strFault)
{
	char *szBuf = NULL;
	ns_uint32 dwSize = 0;

	if (_hFile == INVALID_HANDLE_VALUE)
		Create(strType);

	szBuf = string2char(strFault);
	if (szBuf && _hFile != INVALID_HANDLE_VALUE) 
	{
		SetFilePointer(_hFile, 0, NULL, FILE_END);
		WriteFile(_hFile, szBuf, strlen(szBuf), &dwSize, NULL);
		free(szBuf);
	}

	Destroy();

	return dwSize;
}

void CNSPcsFaultLog::CreateDir(TCHAR *szDir)
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
