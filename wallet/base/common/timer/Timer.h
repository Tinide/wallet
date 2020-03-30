#pragma once
#include <string>
typedef void(__stdcall *timerCallBack) (void*, unsigned char);
class CTimer
{
public:
	CTimer();
	virtual ~CTimer();
public:
	bool setTimer(timerCallBack func,std::string pParam,unsigned long dueTime);
	bool killTimer();
private:
	void* __m_pParam;
	void* __m_hTimer;
	static void* __m_gtimerQueue;
};

