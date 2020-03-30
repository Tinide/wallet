#include "Timer.h"
#include <Windows.h>

HANDLE CTimer::__m_gtimerQueue = NULL;
CTimer::CTimer():__m_hTimer(NULL)
{
}


CTimer::~CTimer()
{
	killTimer();
	if (__m_pParam)
	{
		delete (std::string*)(__m_pParam);
		__m_pParam = NULL;
	}
}

bool CTimer::setTimer(timerCallBack func,std::string pParam, unsigned long dueTime)
{
	if (!__m_gtimerQueue)
	{
		__m_gtimerQueue = CreateTimerQueue();
	}
	if (__m_hTimer)
	{
		return false;
	}
	__m_pParam = (void*)new std::string(pParam);
	if (!CreateTimerQueueTimer(&__m_hTimer, __m_gtimerQueue, func, __m_pParam, dueTime, 0, 0))
	{
		__m_hTimer = NULL;
		return false;
	}

	return true;
}
bool CTimer::killTimer()
{
	if (__m_hTimer)
	{
		if (!DeleteTimerQueueTimer(__m_gtimerQueue, __m_hTimer, INVALID_HANDLE_VALUE))
		{
			return false;
		}
	}
	return true;
}
