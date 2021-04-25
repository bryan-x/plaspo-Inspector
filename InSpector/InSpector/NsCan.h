#pragma once

//#include <locale.h>
#include "./can/canlib.h"

typedef struct {
  int		channel;
  char		name[100];
  DWORD		hwType;
  canHandle	hnd;
  int		hwIndex;
  int		hwChannel;
  int		isOnBus;
  int		driverMode;
  //int      txAck;
} CHANNEL_INFO;

enum {
	CAN_STATE_NONE = 0,
	CAN_STATE_CREATE = 1,
	CAN_STATE_START = 2,
	CAN_STATE_STOP = 3,
	CAN_STATE_DESTROY = 4
};


class CNSCanManager;
class CNSCanDump;



class CNSCanEngine 
{
public:
	CNSCanEngine();
	~CNSCanEngine();

	ns_uint32 Create(CNSCanManager* mgr);
	ns_uint32 Destroy();
	ns_uint32 Start();
	ns_uint32 Stop();

	ns_uint8 GetStatus() { return _can_state; }
private:
	static unsigned int __stdcall RecvThread( void *param );

	ns_uint32 StartRecv();
	ns_uint32 StopRecv();
	ns_uint32 Connect();
	ns_uint32 Disconnect();
	ns_uint32 Recv();
	ns_uint32 canError( char *id, canStatus stat );

	ns_uint32 Generator();

	void SetBitrate(int rate);

private:
	CRITICAL_SECTION _cs;

	int _channelCount;			/** CAN ä�� ���� */
	CHANNEL_INFO* _channelInfo;	/** CAN ä������ */

	HANDLE _hRecvThread;		/**< ������ �ڵ� */
	HANDLE _hRecvStopEvent;		/**< ������ ���� �̺�Ʈ �ڵ� */
	ns_uint8 _bEndThread;		/**< ������ ���� �÷��� */
	
	ns_uint8 _can_initialize;
	ns_uint8 _can_state;
	ns_uint32 _bitrate;			/** CAN ���۷� */
	//ns_uint _usedFlags;

	CNSCanManager* _mgr;
};

