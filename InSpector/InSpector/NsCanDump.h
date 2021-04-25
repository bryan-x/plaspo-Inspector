#pragma once

#include "CanMessage.h"

#include <queue>

#define MAX_THREAD_DUMP		32
#define MAX_ID				31

#define MAX_MEM_SIZE		100

class CNSCanDump;

class CNSDumpWorker
{
public:
	CNSDumpWorker();
	~CNSDumpWorker();

	ns_uint32 Create( ns_uint8 index );
	void Destroy();

	CanMessage *QMalloc();
	void QFree();
		
	ns_uint32 QSize();
	CanMessage *QPop();
	ns_uint32 QPush( CanMessage * m );
	ns_uint32 Save( CanMessage *m );
	void SetStopEvent();
	
	ns_uint32 Dumper();
	static unsigned int __stdcall DumperThread(void *param);
	
private:
	ns_uint8 no;
	unsigned int tid;
	ns_uint8 on;				/**< thread loop */
	ns_uint16 ts;				/**< timestamp */
	CRITICAL_SECTION cs;
	HANDLE evt;
	HANDLE th;
	HANDLE fp;					/**< file handle */
	std::queue<CanMessage *> stor;
	ns_uint8 arx;
	CanMessage *arm;
};

class CNSCanDump
{
public:
	CNSCanDump();
	~CNSCanDump();

	ns_uint32 Create();
	ns_uint32 Destroy();
	ns_uint32 Dump(CanMessage* m);
	void Stop();
	
private:
	ns_uint8 Id2Index(ns_uint8 id);
	void CreateDir( LPCTSTR szDir );
	
private:
	ns_uint32 _no;
	CNSDumpWorker *_dumper;
};