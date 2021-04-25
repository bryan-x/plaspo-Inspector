
/*
 * Copyright (c) 2001-2015 Plaspo co. Ltd.,
 *
 * FILE : NSChiefManager.cpp
 *
 * 2016/05/13 Chief Manager.
 */


#include "stdafx.h"
#include "NSChiefManager.h"
#include "NSCanManager.h"
#include "NSPcsManager.h"
#include "NSMcManager.h"
#include "OutputWnd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CNSChiefManager::CNSChiefManager()
{
	_can = NULL;
	_pcs1 = NULL;
	_pcs2 = NULL;
	_mc = NULL;
	_pLogPanel = NULL;

	_logFilter = 0xffff;

	InitializeCriticalSection(&_cs);
}

CNSChiefManager::~CNSChiefManager()
{
	Destroy();
}

CNSCanManager *CNSChiefManager::CreateCanManager()
{
	CNSCanManager *o = NULL;

	try {
		o = new CNSCanManager(this);
	}
	catch (...) {
		return NULL;
	}

	return o;
}

CNSPcsManager* CNSChiefManager::CreatePcsManager(ns_uint instanceID)
{
	CNSPcsManager *o = NULL;

	try {
		o = new CNSPcsManager(instanceID, this);
	}
	catch (...) {
		return NULL;
	}

	return o;
}

CNSMcManager *CNSChiefManager::CreateMCManager()
{
	CNSMcManager *o = NULL;

	try {
		o = new CNSMcManager(this);
	}
	catch (...) {
		return NULL;
	}

	return o;
}

// 0:OK
ns_uint32 CNSChiefManager::Create()
{
	ns_uint32 ret = 0;

	_can = CreateCanManager();
	if (NULL == _can) {
		TRACE(_T("Create CanManager Objcet - Fail...\n"));
		return 1;
	}
		
	_pcs1 = CreatePcsManager(IID_PCS1);
	if (NULL == _pcs1) {
		TRACE(_T("Create PCS1 Manager Objcet - Fail...\n"));
		return 2;
	}

	_pcs2 = CreatePcsManager(IID_PCS2);
	if (NULL == _pcs2) {
		TRACE(_T("Create PCS2 Manager Objcet - Fail...\n"));
		return 3;
	}
	
	_mc = CreateMCManager();
	if (NULL == _mc) {
		TRACE(_T("Create MC Manager Objcet - Fail...\n"));
		return 4;
	}

	if(_can) _can->Create(_pcs1, _pcs2);
	if(_pcs1) _pcs1->Create();
	if(_pcs2) _pcs2->Create();
	if(_mc) _mc->Create();

	return ret;
}

ns_uint32 CNSChiefManager::Destroy()
{
	ns_uint32 ret = 0;

	if (_mc) {
		_mc->Destroy();
		delete _mc;
		_mc = NULL;
	}

	if (_pcs1) {
		_pcs1->Destroy();
		delete _pcs1;
		_pcs1 = NULL;
	}

	if (_pcs2) {
		_pcs2->Destroy();
		delete _pcs2;
		_pcs2 = NULL;
	}

	if (_can) {
		_can->Destroy();
		delete _can;
		_can = NULL;
	}

	DeleteCriticalSection(&_cs);

	return ret;
}

void CNSChiefManager::SetViewHandle(HWND hPcsList, HWND hFaultList, HWND hDpmList, HWND hCsList)
{
	_hPcsList = hPcsList;
	_hFaultList = hFaultList;
	_hDpmList = hDpmList;
	_hCsList = hCsList;
}

ns_uint32 CNSChiefManager::Start()
{
	ns_uint32 ret = 0;

	if(_can) {
		if (NS_OK == _can->Start()) {
			if(_pcs1) _pcs1->Start();
			if(_pcs2) _pcs2->Start();
			if(_mc) _mc->Start();
		}
	}
	
	return ret;
}

ns_uint32 CNSChiefManager::Stop()
{
	ns_uint32 ret = 0;

	if(_mc)
		_mc->Stop();
	if(_pcs1)
		_pcs1->Stop();
	if(_pcs2)
		_pcs2->Stop();
	if(_can)
		_can->Stop();

	return ret;
}

void CNSChiefManager::ApplyLogFilter()
{
	ns_uint8 flag = _logFilter >> 4;
	_can->FilterLog((_logFilter & 0x1), flag);
	_mc->FilterLog((_logFilter & 0x2), flag);
	_pcs1->FilterLog((_logFilter & 0x4), flag);
	_pcs2->FilterLog((_logFilter & 0x8), flag);
}

HWND CNSChiefManager::GetPcsLogBoxHandle(ns_uint8 instanceID)
{
	HWND hWnd = NULL;
	if (_pLogPanel && (instanceID == IID_PCS1))
		hWnd = _pLogPanel->GetPcs1LogBox();
	else if (_pLogPanel && (instanceID == IID_PCS2))
		hWnd = _pLogPanel->GetPcs2LogBox();
	return hWnd;
}

HWND CNSChiefManager::GetMcLogBoxHandle()
{
	HWND hWnd = NULL;
	if (_pLogPanel)
		hWnd = _pLogPanel->GetMcLogBox();
	return hWnd;
}

HWND CNSChiefManager::GetCanLogBoxHandle()
{
	HWND hWnd = NULL;
	if (_pLogPanel)
		hWnd = _pLogPanel->GetCanLogBox();
	return hWnd;
}