#pragma once

////////////////////////////////////////////////////////
// Service Type
#define SERVICE_FAULT		0x0001
#define SERVICE_TRACEDUMP	0x0002
#define SERVICE_ETC			0x0004
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Error Type
#define ERROR_INFORM		0x0010
#define ERROR_MEMORY		0x0020
#define ERROR_IPC			0x0040
#define ERROR_EXCEPTION		0x0080
////////////////////////////////////////////////////////

class CNSLog
{
public:
	CNSLog() {}
	CNSLog(LPCTSTR module);
	CNSLog(LPCTSTR module, ns_uint8 instanceID);
	~CNSLog();

	ns_uint32 WriteLog(const CString& strLog);

private:
	ns_uint32 Create();
	ns_uint32 Destroy();
	void CreateDir(TCHAR *szDir);

private:
	HANDLE _hLog;
	CRITICAL_SECTION _cs;
	CString _name;
	ns_uint8 _pcs;
};