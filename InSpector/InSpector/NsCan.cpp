// http://www.kvaser.com/canlib-webhelp/index.html

#include "StdAfx.h"
#include "NSCan.h"
#include "NSCanManager.h"
#include "NSLog.h"


//////defines
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CNSCanEngine::CNSCanEngine()
{
	InitializeCriticalSection( &_cs );

	_mgr = NULL;

	_bitrate = canBITRATE_500K;
	//_usedFlags = 0;

	_hRecvThread	= INVALID_HANDLE_VALUE;
	_hRecvStopEvent = INVALID_HANDLE_VALUE;

	_bEndThread = 0;

	_channelCount = 0;
	_channelInfo = NULL;

	_can_initialize = 0;
	_can_state = CAN_STATE_DESTROY;
}

CNSCanEngine::~CNSCanEngine()
{
	DeleteCriticalSection( &_cs );
}

ns_uint32 CNSCanEngine::Create(CNSCanManager* mgr)
{
	ns_uint32 ret = 0;

	if (_can_state != CAN_STATE_DESTROY) return 1;

	_mgr = mgr;

	if (_can_initialize == 0) {
		canInitializeLibrary();		// initialize CAN lib
		_can_initialize = 1;
	}

	_hRecvStopEvent = CreateEvent(NULL, TRUE, FALSE, _T("NSCanEngineEvent"));
	if (INVALID_HANDLE_VALUE == _hRecvStopEvent) {
		_mgr->Log(ERROR_IPC, _T("Create Engine Stop Event Fail!!"));
		return 2;
	}

	_can_state = CAN_STATE_CREATE;

	return ret;
}

ns_uint32 CNSCanEngine::Destroy()
{
	ns_uint32 ret = 0;

	Stop();

	if (INVALID_HANDLE_VALUE != _hRecvStopEvent) {
		CloseHandle(_hRecvStopEvent);
		_hRecvStopEvent = INVALID_HANDLE_VALUE;
	}

	_can_state = CAN_STATE_DESTROY;

	return 0;
}

ns_uint32 CNSCanEngine::Start()
{
	ns_uint32 ret = 0;

	if (_can_state == CAN_STATE_CREATE || _can_state == CAN_STATE_STOP) {
		ret = Connect();
		if (NS_OK == ret) {
			_mgr->Log(SERVICE_ETC, _T("CAN Connected"));
			ret = StartRecv();
			if (NS_OK == ret) {
				_mgr->Log(SERVICE_ETC, _T("Engine Start"));
				_can_state = CAN_STATE_START;
			}
			else
				_mgr->Log(ERROR_INFORM, _T("CAN Connect Fail!!"));
		}
	}

	return ret;
}

ns_uint32 CNSCanEngine::Stop()
{
	ns_uint32 ret = 0;

	if (_can_state != CAN_STATE_START) return 1;
	
	ret = StopRecv();
	if (NS_OK == ret) {
		_mgr->Log(SERVICE_ETC, _T("Engine Stop"));
		Disconnect();
		_mgr->Log(SERVICE_ETC, _T("CAN Disconnected"));
		_can_state = CAN_STATE_STOP;
	}
	return ret;
}

// 0:OK
ns_uint32 CNSCanEngine::Connect()
{
	ns_uint32 ret = 0;
#if defined(_CAN_SIM_)
	return 0;
#endif
	canStatus stat;
	canHandle hnd;
	int i;

	stat = canGetNumberOfChannels(&_channelCount);
	if (stat != canOK) {
		_mgr->Log(ERROR_INFORM, _T("canGetNumberOfChannels Fail!!"));
		return canError("canGetNumberOfChannels", stat);
	}

	_channelInfo = new CHANNEL_INFO[_channelCount];
	_mgr->Log(SERVICE_ETC, _T("Get %d Channels"), _channelCount);

	for (i = 0; i < _channelCount; i++) {
		_channelInfo[i].channel = i;
		_channelInfo[i].hnd = -1;
		_channelInfo[i].isOnBus = 0;
		//_channelInfo[i].txAck = 0;
		_channelInfo[i].driverMode = canDRIVER_NORMAL;

		//obtain some hardware info from CANlib
		canGetChannelData(
			i,
			canCHANNELDATA_CHANNEL_NAME,
			_channelInfo[i].name,
			sizeof(_channelInfo[i].name));

		canGetChannelData(
			i,
			canCHANNELDATA_CARD_TYPE,
			&_channelInfo[i].hwType,
			sizeof(DWORD));

		//open CAN channel
		hnd = canOpenChannel(i, canOPEN_ACCEPT_VIRTUAL);
		if (hnd < 0) {
			_mgr->Log(ERROR_INFORM, _T("Channel %d : Open Fail!!"), i);
			canError("canOpenChannel", stat);
		}
		else {
			_mgr->Log(SERVICE_ETC, _T("Channel %d : Opened"), i);
			_channelInfo[i].hnd = hnd;
			stat = canFlushReceiveQueue(hnd);
			if (stat < 0)
				canError("canFlushReceiveQueue", stat);
		}

		//set the channels busparameters
		stat = canSetBusParams(hnd, _bitrate, 0, 0, 0, 0, 0);
		if (stat < 0) {
			_mgr->Log(ERROR_INFORM, _T("Channel %d : SetBusParam Fail!!"), i);
			canError("canSetBusParams", stat);
		}

		//go on bus (every channel)
		stat = canBusOn(_channelInfo[i].hnd);
		if (stat == canOK) {
			_channelInfo[i].isOnBus = 1;
		}
		else {
			_mgr->Log(ERROR_INFORM, _T("Channel %d : canBusOn Fail!!"), i);
			canError("canBusOn", stat);
		}
	}

	return ret;
}

ns_uint32 CNSCanEngine::Disconnect()
{
	ns_uint32 ret = 0;
#if defined(_CAN_SIM_)
	return 0;
#endif
	canStatus stat;
	int i;

	// CAN 종료
	for (i = 0; i < _channelCount; i++) {
		stat = canBusOff(_channelInfo[i].hnd);
		if (stat < 0) {
			_mgr->Log(ERROR_INFORM, _T("Channel %d : canBusOff Fail!!"), i);
			canError("canBusOff", stat);
			ret = 1; // BusOff Error
		}

		stat = canClose(_channelInfo[i].hnd);
		if (stat < 0) {
			_mgr->Log(ERROR_INFORM, _T("Channel %d : Close Fail!!"), i);
			canError("canClose", stat);
			ret = 1; // Can Close Error
		}
		_mgr->Log(SERVICE_ETC, _T("Channel %d : Closed"), i);
	}

	if (NULL != _channelInfo) {
		delete[] _channelInfo;
		_channelInfo = NULL;
	}

	_channelCount = 0;

	return ret;
}

ns_uint32 CNSCanEngine::StartRecv()
{
	_hRecvThread = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, NULL);
	if (INVALID_HANDLE_VALUE == _hRecvThread) {
		_mgr->Log(ERROR_IPC, _T("Engine Start Fail!!"));
		return 1;
	}

	return 0;
}

ns_uint32 CNSCanEngine::StopRecv()
{
	DWORD dwExitCode = 0;

	_bEndThread = 1;
	SetEvent(_hRecvStopEvent);

	// 쓰레드 종료
	if (INVALID_HANDLE_VALUE != _hRecvThread) {
		WaitForSingleObject(_hRecvThread, THREAD_TIMEOUT_WAIT);
		GetExitCodeThread(_hRecvThread, &dwExitCode);

		if (STILL_ACTIVE == dwExitCode) {
			TerminateThread(_hRecvThread, 0);
			TRACE(_T("CAN Thread -  terminated...\n"));
		}

		CloseHandle(_hRecvThread);
		_hRecvThread = INVALID_HANDLE_VALUE;
		ResetEvent(_hRecvStopEvent);
	}

	return 0;
}

unsigned int __stdcall CNSCanEngine::RecvThread(void *param)
{
	ns_uint32 ret = 0;
	CNSCanEngine *o = (CNSCanEngine *)param;
#if defined(_CAN_SIM_)
	ret = o->Generator();
#else
	ret = o->Recv();
#endif
	return 0;
}

ns_uint32 CNSCanEngine::Generator()
{
	ns_uint32 ret = 0;
#if defined(_CAN_SIM_)
	ns_uint32 ms = 50;
	DWORD dwResult = 0;
	long id = 1;
	CanMessage m;
	time_t t;
	ns_uint32 i = 0, j;
	ns_uint8 tg = 0;
	memset(&m, 0x0, sizeof(CanMessage));

	_bEndThread = 0;
	while( !_bEndThread ) {
		dwResult = WaitForSingleObject(_hRecvStopEvent, ms);
		if( WAIT_OBJECT_0 == dwResult ) {
			_bEndThread = 1;
		} else if( WAIT_TIMEOUT == dwResult ) {
			for (i = CID_PCS_1; i < CID_MAX; i++ ) {
				m.id = i;
				m.flag = 4;
				m.dlc = 8;
				t = time(NULL);
				localtime_s(&m.c, &t);
# if 1
				for(j=0; j<8; j++) {
					m.data[j] = tg;
				}
#endif
				if( i >= CID_PCS_1 && i <= CID_PCS_12_FAULT ) {
					ret = _mgr->map_write(&m);
				}

				if( i >= CID_PCS_21 && i <= CID_PCS_32_FAULT ) {
					ret = _mgr->map_write(&m);
				}

				if( i >= CID_PCS1_TRACE && i <= CID_PCS2_TRACE ) {
					//ret = _mgr->map_write(&m);
				}

				if( i >= CID_DPM_61 && i <= CID_EMS ) {
					ret = _mgr->map_write(&m);
				}
			}
# if 1
			tg++;
#endif
		}
	}
#endif
	return ret;
}


// 0:OK 1:ERR
ns_uint32 CNSCanEngine::Recv()
{
	ns_uint32 ret = 0;
	DWORD dwResult = 0;
	canStatus stat;
	time_t t;
	int i, j;

	CanMessage canMsg;

#ifdef __DISCONNECT_MODULE
	unsigned int cntTime = 0;
	unsigned int cntPCS1 = 0;
	unsigned int cntPCS2 = 0;
	unsigned int cntDPM = 0;
	unsigned int cntCS = 0;
#endif

	_bEndThread = 0;

	while (!_bEndThread) {
		dwResult = WaitForSingleObject(_hRecvStopEvent, 1);
		if (WAIT_OBJECT_0 == dwResult) {
			_bEndThread = 1;
		}
		else if (WAIT_TIMEOUT == dwResult) {
			for (i = 0; i < _channelCount; i++) {
				memset(&canMsg, 0, sizeof(CanMessage));
				canMsg.ch = i;
				stat = canRead(_channelInfo[i].hnd, &canMsg.id, canMsg.data, &canMsg.dlc, &canMsg.flag, &canMsg.ts);
				switch (stat) {
				case canOK:
#ifdef CAN_ENGINE_MESSAGE_TRACE
					TRACE(_T("RxMsg: Ch:%u ID:%2d DLC:%u Flg:%02x T:%08x Data:"), _channelInfo[i].channel, canMsg.id, canMsg.dlc, canMsg.flag, canMsg.ts);
#endif
					if ((canMsg.flag & canMSG_RTR) == 0) {
						if (canMSG_ERROR_FRAME == canMsg.flag) {
							TRACE(_T("Message is an error frame\n"));
							_mgr->Log(ERROR_EXCEPTION, _T("Message is an error frame."));
						}
						else {
							if (canMsg.flag & canMSG_EXT) {
								t = time(NULL);
								localtime_s(&canMsg.c, &t);
								_mgr->map_write(&canMsg);
#ifdef CAN_ENGINE_MESSAGE_TRACE
								for (j = 0; (ns_uint)j < canMsg.dlc; j++)
									TRACE(_T("%02x"), canMsg.data[j]);

								TRACE(_T("\n"));
#endif
							}
							else if (canMsg.flag & canMSG_STD) {
								// Standard message
							}
						}
					}
					break;

				case canERR_NOMSG:
					break;

				default:
					TRACE(_T("ERROR canRead() FAILED,channel= %d, Err= %d \n"), i, stat);
					break;
				}
			}
#ifdef __DISCONNECT_MODULE
			const int module_timeout = 5000;
			if (cntTime++ > module_timeout) {
				if (cntPCS1 == 0)	_mgr->ClearPcsList(1); /*PCS1 Clear List*/
				if (cntPCS2 == 0)	_mgr->ClearPcsList(2); /*PCS2 Clear List*/ 
				if (cntDPM == 0)	_mgr->ClearDpmList(); /*DPM Clear List*/
				if (cntCS == 0)		_mgr->ClearCsList(); /*CS Clear List*/ 
				cntTime = cntPCS1 = cntPCS2 = cntDPM = cntCS = 0;
			}

			if ((canMsg.id >= CID_PCS_1) && (canMsg.id <= CID_PCS_12_FAULT))	// +TRACE?
				cntPCS1++;
			else if ((canMsg.id >= CID_PCS_21) && (canMsg.id <= CID_PCS_32_FAULT)) // +TRACE?
				cntPCS2++;
			else if (canMsg.id == CID_DPM_61)
				cntDPM++;
			else if ((canMsg.id >= CID_COOL_62) && (canMsg.id <= CID_COOL_63))
				cntCS++;
#endif
		}
	}

	for (i = 0; i < _channelCount; ++i) {
		stat = canFlushReceiveQueue(_channelInfo[i].hnd);
		if (stat != canOK) {
			TRACE(_T("Connect(%d) canFlushReceiveQueue(%d) FAILED Err=%d\n"), __LINE__, i, stat);
			ret = 1;
		}
	}

	return ret;
}

// 0:OK 1:ERR
ns_uint32 CNSCanEngine::canError( char *id, canStatus stat )
{
	char buf[50];
	if( stat != canOK ) {
		buf[0] = '\0';
		canGetErrorText(stat, buf, sizeof(buf));
		TRACE(_T("%S: failed, stat=%d (%S)\n"), id, (int)stat, buf);
	}
	return 1;
}

void CNSCanEngine::SetBitrate(int rate)
{
	switch (rate) {
	case 1000:
		_bitrate = canBITRATE_1M;
		break;
	case 500:
		_bitrate = canBITRATE_500K;
		break;
	case 250:
		_bitrate = canBITRATE_250K;
		break;
	case 125:
		_bitrate = canBITRATE_125K;
		break;
	case 100:
		_bitrate = canBITRATE_100K;
		break;
	case 62:
		_bitrate = canBITRATE_62K;
		break;
	case 50:
		_bitrate = canBITRATE_50K;
		break;
	default:
		TRACE(_T("Baudrate set to 500 kbit/s. \n"));
		_bitrate = canBITRATE_500K;
		break;
	}

	if (1000 == rate)
		_mgr->Log(SERVICE_ETC, _T("Set Bitrate 1M"));
	else
		_mgr->Log(SERVICE_ETC, _T("Set Bitrate %dK"), rate);
}