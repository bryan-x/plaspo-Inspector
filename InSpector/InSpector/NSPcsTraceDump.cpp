#include "stdafx.h"
#include "NSPcsTraceDump.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4996)


#define DIR_TRACE	_T("TRACE")


CNSPcsTraceDump::CNSPcsTraceDump(ns_uint8 instanceID)
{
	_hFile = INVALID_HANDLE_VALUE;

	_instanceID = instanceID;
	
	ZeroMemory(_dump, sizeof(_dump));
}

CNSPcsTraceDump::~CNSPcsTraceDump()
{
	Destroy();
}

ns_uint32 CNSPcsTraceDump::Create()
{
	CString strFileName = _T("");

	time_t t;
	struct tm c;

	t = time(NULL);
	localtime_s(&c, &t);
	CreateDir(DIR_TRACE);

	strFileName.Format(_T("%s\\PCS%d_TraceDump(%04d%02d%02d_%02d%02d%02d).csv"),
		DIR_TRACE, _instanceID,
		c.tm_year + 1900, c.tm_mon + 1, c.tm_mday,
		c.tm_hour, c.tm_min, c.tm_sec
	);

	_hFile = CreateFile(
		strFileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,
		NULL);

	if (INVALID_HANDLE_VALUE == _hFile)
	{
		CloseHandle(_hFile);
		return 1;
	}

	return 0;
}

ns_uint32 CNSPcsTraceDump::Destroy()
{
	if (_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(_hFile);

	_hFile = INVALID_HANDLE_VALUE;

	ZeroMemory(_dump, sizeof(_dump));

	return 0;
}

ns_uint32 CNSPcsTraceDump::Add(ns_uint8 traceNum, float *dump)
{
	ns_uint32 i = 0;
	//TRACE(_T("NSTraceLog::Add( %d )\n"), index );
	if (traceNum >= TRACE_BUF_CNT)
		return 1;

	if (dump) {
		for (i = 0; i < TRACE_BUF_SIZE; ++i) {
			_dump[i][traceNum] = dump[i];
		}
		return 0;
	}

	return 1;
}

ns_uint32 CNSPcsTraceDump::Write(const CString& str)
{
	DWORD dwBytesWritten = 0;
	DWORD writeBytesSize = 0;

	if (_hFile == INVALID_HANDLE_VALUE) {
		Create();
	}

	if (_hFile != INVALID_HANDLE_VALUE) 
	{
		SetFilePointer(_hFile, 0, NULL, FILE_END);

		writeBytesSize = str.GetLength() * sizeof(TCHAR);
		WriteFile(_hFile, str, writeBytesSize, &dwBytesWritten, NULL);
	}

	return dwBytesWritten;
}

ns_uint32 CNSPcsTraceDump::Save()
{
	ns_uint32 ret = 0, i = 0;
	CString strTrace = _T("");
	CString strHeader = _T("");

	strHeader = _T("trace no,Idc,Iqe_ref,Iqe,Vqe_flt,Fault_SW1,Fault_SW2,Flag_gating,Vbat_flt,Vdc1_flt,Vdc2_flt\r\n");
	if (Write(strHeader) == 0){
		ret = 1;
	}

	strHeader = _T("trace Code, 1031,1084,1064,1070,1369,1370,2011,1038,1742,1744\r\n");
	if (Write(strHeader) == 0) {
		ret = 1;
	}

	for (i = 0; i < TRACE_BUF_SIZE; ++i)
	{
		strTrace.Format(_T("%d, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f\r\n"),
			i,
			_dump[i][0],
			_dump[i][1],
			_dump[i][2],
			_dump[i][3],
			_dump[i][4],
			_dump[i][5],
			_dump[i][6],
			_dump[i][7],
			_dump[i][8],
			_dump[i][9]
		);
		if (Write(strTrace) == 0){
			ret = 1;
		}
	}
	Destroy();

	return 0;
}

void CNSPcsTraceDump::CreateDir(TCHAR *szDir)
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