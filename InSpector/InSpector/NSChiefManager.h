/**
 * @file NSChiefManager.h
 * @date 2015/12/17
 * @author plaspo
 * @brief 모든 매니저를 관리한다. \n
 *        http://www.plaspo.co.kr
 */

#pragma once


class CNSCanManager;
class CNSPcsManager;
class CNSMcManager;
class COutputWnd;

class CNSChiefManager
{
public:
	CNSChiefManager();
	~CNSChiefManager();

	ns_uint32 Create();
	ns_uint32 Start();
	ns_uint32 Stop();
	ns_uint32 Destroy();

	void SetViewHandle(HWND hPcsList, HWND hFaultList, HWND hDpmList, HWND hCsList);
	HWND GetPcsListHandle() const { return _hPcsList; }
	HWND GetFaultListHandle() const { return _hFaultList; }
	HWND GetDpmListHandle() const { return _hDpmList; }
	HWND GetCsListHandle() const { return _hCsList; }
	void SetLogPanelHnadle(COutputWnd* pLogPanel) { _pLogPanel = pLogPanel; }
	HWND GetPcsLogBoxHandle(ns_uint8 instanceID);
	HWND GetMcLogBoxHandle();
	HWND GetCanLogBoxHandle();

	void SetLogFilter(ns_uint16 filterFlag) { _logFilter = filterFlag; }
	ns_uint16 GetLogFilter() const { return _logFilter; }
	void ApplyLogFilter();

	CNSCanManager* GetCanManager() { return _can; }

private:
	CNSCanManager* CreateCanManager();
	CNSPcsManager* CreatePcsManager(ns_uint instanceID);
	CNSMcManager* CreateMCManager();

private:
	CRITICAL_SECTION _cs;

	HWND _hPcsList;
	HWND _hFaultList;
	HWND _hDpmList;
	HWND _hCsList;
	COutputWnd* _pLogPanel;

	CNSCanManager *_can;
	CNSPcsManager *_pcs1;
	CNSPcsManager *_pcs2;
	CNSMcManager *_mc;

	ns_uint16 _logFilter;
};