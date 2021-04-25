#pragma once

#define TRACE_BUF_SIZE			257
#define TRACE_BUF_CNT			10


enum STATUS
{
	TRACE_READY = 0,
	TRACE_START,
	TRACE_SENDING,
	TRACE_END,
};

class CNSPcsTraceDump
{
public:
	CNSPcsTraceDump(ns_uint8 instanceID);
	~CNSPcsTraceDump();

	ns_uint32 Add(ns_uint8 traceNum, float *dump);
	ns_uint32 Save();
	
private:
	ns_uint32 Create();
	ns_uint32 Destroy();
	ns_uint32 Write(const CString& str);
	void CreateDir(TCHAR *szDir);


private:
	HANDLE _hFile;
	ns_uint8 _instanceID;
	float _dump[TRACE_BUF_SIZE][TRACE_BUF_CNT];
};