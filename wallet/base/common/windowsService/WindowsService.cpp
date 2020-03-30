#include "WindowsService.h"
#include <iostream>

ServiceInit CWindowsService::__m_serviceInit = NULL;
ServiceUnInit CWindowsService::__m_serviceUnInit = NULL;

SERVICE_STATUS CWindowsService::ServiceStatus;
SERVICE_STATUS_HANDLE  CWindowsService::ServiceStatusHandle;

std::mutex CWindowsService::__m_exitMtx;
std::condition_variable CWindowsService::__m_exitCond;

bool CWindowsService::__m_bDebug = false;

CWindowsService::CWindowsService()
{
}


CWindowsService::~CWindowsService()
{
}

void CWindowsService::setServiceHandle(ServiceInit serviceInit, ServiceUnInit serviceUnInit)
{
	__m_serviceInit = serviceInit;
	__m_serviceUnInit = serviceUnInit;
}

void CWindowsService::ServiceMain(DWORD dwArgc, LPTSTR * lpszArgv)
{
	ServiceStatusHandle = RegisterServiceCtrlHandler(TEXT(SZAPPNAME), ServiceCtrl);
	if (!ServiceStatusHandle)
	{
		//ע��ʧ��
		return;
	}

	reportServiceStatus(SERVICE_START_PENDING);

	ServiceStart(dwArgc, lpszArgv);

	reportServiceStatus(SERVICE_STOPPED);
}

void CWindowsService::startServiceCtrlDispatcher(SERVICE_TABLE_ENTRYA * lpServiceStartTable)
{
	StartServiceCtrlDispatcher(lpServiceStartTable);
}

void WINAPI CWindowsService::ServiceCtrl(DWORD dwCtrlCode)
{
	switch (dwCtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		//�ϱ�����״̬
		reportServiceStatus(SERVICE_STOP_PENDING);
		ServiceStop();
		break;
	case SERVICE_CONTROL_PAUSE:
	case SERVICE_CONTROL_CONTINUE:
	case SERVICE_CONTROL_INTERROGATE:
	default:
		break;
	}
}


BOOL CWindowsService::reportServiceStatus(DWORD dwCurrentState)
{
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	ServiceStatus.dwCurrentState = dwCurrentState;

	return SetServiceStatus(ServiceStatusHandle, &ServiceStatus);
}


void CWindowsService::ServiceStart(DWORD dwArgc, LPTSTR* lpszArgv)
{
	////*******************����������ʼ************************

	if (!__m_serviceInit())
	{
		__m_serviceUnInit();
		return;
	}

	//*******************������������************************

	wait();

	//*******************�������************************
	__m_serviceUnInit();
	//*******************�������************************
}


void CWindowsService::ServiceStop()
{
	__m_exitCond.notify_one();
	reportServiceStatus(SERVICE_STOP_PENDING);
}

void CWindowsService::wait()
{
	if (__m_bDebug)
	{
		std::cout << "��Q��q�����˳���" << std::endl;
		char c;
		while (1)
		{
			c = getchar();
			if ('\n' == c) continue;
			if ('q' == c || 'Q' == c)
			{
				break;
			}
			else
				std::cout << "��Q��q�����˳���" << std::endl;
		}
	}
	else
	{
		reportServiceStatus(SERVICE_RUNNING);
		{
			std::unique_lock<std::mutex> lock(__m_exitMtx);
			__m_exitCond.wait(lock);
		}
	}
}


void CWindowsService::installService()
{
	char szPath[1024];
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	if (!GetModuleFileName(NULL, szPath, 1024))
	{
		GetLastError();
		return;
	}

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (schSCManager)
	{
		schService = CreateService(schSCManager,
			SZAPPNAME,
			SZAPPNAME,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			szPath,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);
		if (schService)
		{
			SERVICE_DESCRIPTION sdBuf;
			sdBuf.lpDescription = "CTP ������";
			ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sdBuf);
			CloseServiceHandle(schService);
		}
		else
		{
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
	}
}

void CWindowsService::removeService()
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	//�򿪷���������ݿ�
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager)
	{
		//��ȡ���������
		schService = OpenService(schSCManager, SZAPPNAME, SERVICE_ALL_ACCESS);
		if (schService)
		{
			//��ͼֹͣ����
			if (ControlService(schService, SERVICE_CONTROL_STOP, &ServiceStatus))
			{
				while (1)
				{
					Sleep(100);
					if (QueryServiceStatus(schService, &ServiceStatus))
					{
						if (SERVICE_STOP_PENDING == ServiceStatus.dwCurrentState)
						{
							continue;
						}
						if (SERVICE_STOPPED == ServiceStatus.dwCurrentState)
						{
							break;
						}
					}
				}
			}
		}
		DeleteService(schService);
		CloseServiceHandle(schService);
	}
	CloseServiceHandle(schSCManager);
}

void CWindowsService::debugService(int argc, char** argv)
{
	__m_bDebug = true;
	ServiceStart(argc, argv);
}

void CWindowsService::setDebug(bool debug)
{
	__m_bDebug = debug;
}
