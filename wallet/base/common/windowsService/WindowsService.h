#pragma once
#include <Windows.h>
#include "log.h"

typedef bool(*ServiceInit)();
typedef void(*ServiceUnInit)();

class CWindowsService
{
public:
	CWindowsService();
	virtual ~CWindowsService();
public:
	static void setServiceHandle(ServiceInit serviceInit,ServiceUnInit serviceUnInit);
	static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
	static void startServiceCtrlDispatcher(SERVICE_TABLE_ENTRYA    *lpServiceStartTable);
	static void installService();
	static void removeService();
	static void debugService(int argc, char** argv);
	static void setDebug(bool debug);
private:
	static void WINAPI ServiceCtrl(DWORD dwCtrlCode);
	static BOOL reportServiceStatus(DWORD dwCurrentState);
	static void ServiceStart(DWORD dwArgc, LPTSTR* lpszArgv);
	static void ServiceStop();
	static void wait();
private:
	static SERVICE_STATUS ServiceStatus;//����״̬
	static SERVICE_STATUS_HANDLE  ServiceStatusHandle;//����״̬���

	static ServiceInit __m_serviceInit;
	static ServiceUnInit __m_serviceUnInit;

	static std::mutex __m_exitMtx;//�����˳���
	static std::condition_variable __m_exitCond;//�����˳�����

	static bool __m_bDebug;
};

