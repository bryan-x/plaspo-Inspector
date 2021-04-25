#pragma once


class CNSPcsFaultLog
{
public:
	CNSPcsFaultLog(ns_uint8 instanceID);
	~CNSPcsFaultLog();
public:

	ns_uint32 Create(const CString& strType);
	ns_uint32 Destroy();
	ns_uint32 WriteLog(const CString& strType, const CString& strFault);

private:
	void CreateDir(TCHAR *szDir);

private:
	HANDLE _hFile;
	ns_uint8 _pcs;
};