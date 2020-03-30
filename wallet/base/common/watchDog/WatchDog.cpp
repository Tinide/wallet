#include "WatchDog.h"
#include "base/common/markup/RWConfig.h"
#include "base/common/tool/Tool.h"

CWatchDog::CWatchDog()
{
}


CWatchDog::~CWatchDog()
{
}


#include <tlhelp32.h>
int JudgeProcessByName(DWORD processId, char* pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		LOG_INFO("获取进程列表失败");
		return -1;
	}
	int bRet = FALSE;
	PROCESSENTRY32 pe = { sizeof(pe) };
	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe))
	{
		if (strcmp(pe.szExeFile, pName) == 0)
		{
			if (pe.th32ProcessID == processId)
			{
				bRet = TRUE;
				break;
			}
		}
	}
	CloseHandle(hSnapshot);
	return bRet;
}


void CWatchDog::bark()
{
	CWriteConfig::WriteXMLConfig(GetCurrentProcessId(), "processId", SZAPPNAME);
	DWORD watchDogId = atoi(CReadConfig::ReadXMLConfig(SZAPPNAME+std::string(".xml"),"CONFIG","processId", "watchDog").c_str());
#ifdef NDEBUG
	if (FALSE == JudgeProcessByName(watchDogId, "watchDog.exe"))
	{
		WinExec(("watchDog.exe "+ std::string(SZAPPNAME)).c_str(), 0);
		LOG_INFO("拉起看门狗");
	}
	else
	{
		LOG_INFO("狗已经存在");
	}
#endif
}

void CWatchDog::die()
{
#ifdef NDEBUG
	DWORD watchDogId = atoi(CReadConfig::ReadXMLConfig(SZAPPNAME + std::string(".xml"), "CONFIG", "processId", "watchDog").c_str());
	WinExec(string_format("taskkill /f /pid %d",watchDogId).c_str(), 0);
#endif
}
