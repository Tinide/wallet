#include "log.h"
#include <direct.h>
#include <windows.h>

std::string __init()
{
	char szPathName[MAX_PATH];
	GetModuleFileName(NULL, szPathName, MAX_PATH);
	std::string strPathName = szPathName;
	std::string strExeFile(SZAPPNAME);
	strExeFile += ".exe";
	strPathName = strPathName.substr(0, strPathName.find(strExeFile));
	if (SetCurrentDirectory(strPathName.c_str()) == FALSE)
	{
		return FALSE;
	}
	if (_access((strPathName+"log").c_str(), 02) != 0)
	{
		if( _mkdir((strPathName + "log").c_str()) != 0)
		{
			return FALSE;
		}
	}
	return strPathName;
}

std::string __pathname__ = __init();
std::shared_ptr<spdlog::logger> fileLog = spdlog::rotating_logger_mt("f", (__pathname__+"log\\"+ SZAPPNAME +".log").c_str(), 1024 * 1024 * 20, 10);
std::shared_ptr<spdlog::logger> console = spdlog::stdout_logger_mt("console");