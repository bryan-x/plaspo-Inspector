#include "StdAfx.h"
#include "NSCanDump.h"



#define DIR_DUMP	_T("DUMP")

CNSDumpWorker::CNSDumpWorker()
{
	no = 0;
	tid = 0;
	on = 0;
	ts = 0;
	evt = INVALID_HANDLE_VALUE;
	th = INVALID_HANDLE_VALUE;
	fp = INVALID_HANDLE_VALUE;
	arx = 0;
}

CNSDumpWorker::~CNSDumpWorker()
{
}

ns_uint32 CNSDumpWorker::Create( ns_uint8 index )
{
	ns_uint32 ret = 0;
	CString strEvt = _T("");
	arm = NULL;
	no = index;
	InitializeCriticalSection(&cs);
	strEvt.Format(_T("dts_evt_%d"), no);
	evt = CreateEvent(NULL, FALSE /*TRUE*/, FALSE, strEvt);
	th = (HANDLE)_beginthreadex(NULL, 0, DumperThread, (void*)this, 0, &tid);
	if( th == INVALID_HANDLE_VALUE ) {
		TRACE(_T("Thread Pool slot %d create fail\n"), no);
		ret = 1;
	}

	arm = (CanMessage *)malloc(sizeof(CanMessage)*MAX_MEM_SIZE);
	if( arm ) {
		memset(arm, 0x0, sizeof(CanMessage)*MAX_MEM_SIZE);
	} else {
		ret = 2;
	}
	
	return ret;
}

void CNSDumpWorker::Destroy()
{
	CanMessage *m = NULL;

	on = 0;
	SetEvent( evt );
	WaitForSingleObject( th, 100 );
	
	while( stor.empty() == FALSE ) {
		m = (CanMessage *)stor.front();
		stor.pop();
		delete m;
	}

	QFree();
	
	DeleteCriticalSection(&cs);

	CloseHandle( evt );
	CloseHandle( th );
	//if( fp != INVALID_HANDLE_VALUE ) CloseHandle( fp );	
}

CanMessage *CNSDumpWorker::QMalloc()
{
	CanMessage *m = NULL;
	if( arm ) {
		m = &arm[arx++];
		if( arx == MAX_MEM_SIZE ) arx = 0;
	}
	return m;
}

void CNSDumpWorker::QFree()
{
	if( arm ) free( arm );
	arm = NULL;
	arx = 0;
}

ns_uint32 CNSDumpWorker::QSize()
{
	ns_uint32 size = 0;
	EnterCriticalSection( &cs );
	size = stor.size();
	LeaveCriticalSection( &cs );
	return size;
}

CanMessage *CNSDumpWorker::QPop()
{
	CanMessage *m = NULL;
	EnterCriticalSection( &cs );
	if( stor.size() ) {
		m = stor.front();
		stor.pop();
	}
	LeaveCriticalSection( &cs );
	return m;
}

// 0:OK
ns_uint32 CNSDumpWorker::QPush( CanMessage * m )
{
	ns_uint32 ret = 1;
	CanMessage *m2 = NULL;

	m2 = QMalloc();
	if( m2 ) {
		EnterCriticalSection( &cs );
		memcpy( m2, m, sizeof(CanMessage) );
		stor.push( m2 );
		ret = 0;
		LeaveCriticalSection( &cs );
	}
	return ret;
}

// // 2016:10:10 02:24:40 NO TS DLC 00 00 00 00 00 00 00 00

ns_uint32 CNSDumpWorker::Save( CanMessage *m )
{
	ns_uint32 ret = 0;
	ns_uint16 cts = 0, daychange = 0;
	CString strDir, strFile;
	DWORD dwBytesWritten = 0;
	char szBuf[128];

	cts = m->c.tm_year + 1900 + ((m->c.tm_mon + 1) * 100) + m->c.tm_mday;

	if( ts != cts ) {
		ts = cts;
		daychange = 1;
	}
	
	strDir.Format(_T("%s\\dump_%04d.%02d.%02d"), DIR_DUMP, m->c.tm_year + 1900, m->c.tm_mon + 1, m->c.tm_mday);

	if( daychange ) {
		CreateDirectory(strDir, NULL);
		strFile.Format(_T("%s\\dump_%02d.cxt"), strDir, m->id);	
		
		if ( fp != INVALID_HANDLE_VALUE ) {
			CloseHandle(fp);
			fp = INVALID_HANDLE_VALUE;
		}
		
		if( fp == INVALID_HANDLE_VALUE )
			fp = CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

		if ( fp != INVALID_HANDLE_VALUE ) {
			SetFilePointer(fp, 0, NULL, FILE_END);

			memset(szBuf, 0x0, 128);
			sprintf_s(szBuf, 128, "%04d:%02d:%02d %02d:%02d:%02d %08d %06d %d %02X %02X %02X %02X %02X %02X %02X %02X\r\n", \
										m->c.tm_year+1900, m->c.tm_mon+1,m->c.tm_mday,m->c.tm_hour,m->c.tm_min,m->c.tm_sec, \
										m->no,m->ts,m->dlc, \
										m->data[0],m->data[1],m->data[2],m->data[3],m->data[4],m->data[5],m->data[6],m->data[7]);
			
			ret = WriteFile(fp, (LPCVOID)szBuf, strlen(szBuf), &dwBytesWritten, NULL);
			if( ret ) {
				if (strlen(szBuf) != dwBytesWritten)
					TRACE(_T("Dumper::Save(%d) Message:%2d - %d Write Exception \n"), no, m->id, m->no);
			} else {
					TRACE(_T("Dumper::Save(%d) Message:%2d - %d Write Fail \n"), no, m->id, m->no);
			}
		} else {
			TRACE(_T("Dumper::Save(%d) Message:%2d Write Handle Invalid  \n"), no, m->id);
		}
	} else {
		strFile.Format(_T("%s\\dump_%02d.cxt"), strDir, m->id);
		if( fp == INVALID_HANDLE_VALUE )
			fp = CreateFile(strFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

		if ( fp != INVALID_HANDLE_VALUE ) {
			SetFilePointer(fp, 0, NULL, FILE_END);

			memset(szBuf, 0x0, 128);
			sprintf_s(szBuf, 128, "%04d:%02d:%02d %02d:%02d:%02d %08d %06d %d %02X %02X %02X %02X %02X %02X %02X %02X\r\n", \
										m->c.tm_year+1900, m->c.tm_mon+1,m->c.tm_mday,m->c.tm_hour,m->c.tm_min,m->c.tm_sec, \
										m->no,m->ts,m->dlc, \
										m->data[0],m->data[1],m->data[2],m->data[3],m->data[4],m->data[5],m->data[6],m->data[7]);			
			ret = WriteFile(fp, (LPCVOID)szBuf, strlen(szBuf), &dwBytesWritten, NULL);
			if( ret ) {
				//TRACE(_T("Dumper::Save(%d) Message:%2d Write... \n"), no, m->id);
			}
		} else {
			TRACE(_T("Dumper::Save(%d) Message:%2d Write Handle Invalid  \n"), no, m->id);
		}	
	}
	return ret;

}

ns_uint32 CNSDumpWorker::Dumper()
{
	ns_uint32 ret = 0, i = 0, size = 0;	
	CanMessage *m = NULL;
	DWORD dwStatus = 0;

	on = 1;

	while( on ) {
		dwStatus = WaitForSingleObject( evt, 5 );
		if( dwStatus == WAIT_OBJECT_0 ) {
			size = QSize();
			for( i = 0; i < size; i++ ) {
				m = QPop();
				if( m ) {
					ret = Save( m );
				}
			}
			if( fp != INVALID_HANDLE_VALUE ) {
				CloseHandle( fp );
				fp = INVALID_HANDLE_VALUE;
			}

			if(on) continue;
			else break;
		}

		size = QSize();
		for( i = 0; i < size; i++ ) {
			m = QPop();
			if( m ) {
				ret = Save( m );
			}
		}
	}
	return 0;
}

unsigned int __stdcall CNSDumpWorker::DumperThread(void *param)
{
	CNSDumpWorker *me = NULL;
	ns_uint32 ret = 0;
	me = (CNSDumpWorker *)param;
	ret = me->Dumper();
	return 0;
}

void CNSDumpWorker::SetStopEvent()
{
	SetEvent( evt );
}

CNSCanDump::CNSCanDump()
{
	_dumper = NULL;
	_no = 0;
}

CNSCanDump::~CNSCanDump()
{
}

ns_uint8 CNSCanDump::Id2Index(ns_uint8 id)
{
	ns_uint8 idx = 0;
	
	switch (id) {
	case 1: idx = 1; break;
	case 2: idx = 2; break;
	case 3: idx = 3; break;
	case 4: idx = 4; break;
	case 5: idx = 5; break;
	case 6: idx = 6; break;
	case 7: idx = 7; break;
	case 8: idx = 8; break;
	case 9: idx = 9; break;
	case 10: idx = 10; break;
	case 11: idx = 11; break;
	case 12: idx = 12; break;
	case 21: idx = 13; break;
	case 22: idx = 14; break;
	case 23: idx = 15; break;
	case 24: idx = 16; break;
	case 25: idx = 17; break;
	case 26: idx = 18; break;
	case 27: idx = 19; break;
	case 28: idx = 20; break;
	case 29: idx = 21; break;
	case 30: idx = 22; break;
	case 31: idx = 23; break;
	case 32: idx = 24; break;
	case 51: idx = 25; break;
	case 52: idx = 26; break;
	case 61: idx = 27; break;
	case 62: idx = 28; break;
	case 63: idx = 29; break;
	case 64: idx = 30; break;
	case 65: idx = 31; break;
	default:
		break;
	}

	return idx;
}

ns_uint32 CNSCanDump::Create()
{
	CreateDir(DIR_DUMP);

	int i = 0;

	try {
		_dumper = new CNSDumpWorker[MAX_THREAD_DUMP];
	} catch ( ... ) {
		_dumper = NULL;
	}

	if( _dumper ) {
		while ( i < MAX_THREAD_DUMP ) {
			_dumper[i].Create( i );
			i++;
		}
	}

	return 0;
}

ns_uint32 CNSCanDump::Destroy()
{
	int i = 0;
	while( i < MAX_THREAD_DUMP ) {
		_dumper[i].Destroy();
		i++;
	}

	delete [] _dumper;
	
	return 0;
}

void CNSCanDump::CreateDir(LPCTSTR szDir)
{
	TCHAR szDirName[256] = { 0 };
	TCHAR *p = (TCHAR*)szDir;
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

// 0:OK
ns_uint32 CNSCanDump::Dump(CanMessage* m)
{
	ns_uint32 ret = 1;
	ns_uint8 index = 0;
	m->no = _no++;
	index = Id2Index( (ns_uint8)m->id );
	if( index ) {
		_dumper[index].QPush(m);
		ret = 0;
	}

	return ret;
}

void CNSCanDump::Stop()
{
	int i = 0;
	while ( i < MAX_THREAD_DUMP ) {
		_dumper[i].SetStopEvent();
		i++;
	}
}