#include "Server.h"
#include <Windows.h>
#include <assert.h>
#include "../common/log/log.h"

CServer::CServer():__m_nThread(0), _m_loopTime(1000), __m_bMaintain(false),_m_maintainLoopTime(5000)
{
}


CServer::~CServer()
{
}

bool CServer::start(void* param)
{
	__m_nThread = 1;
	if (!_init())
	{
		return false;
	}

	_m_exitEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!_m_exitEvent[0])
	{
		return false;
	}
	__m_spRunThread[0] = std::make_shared<std::thread>(&CServer::__go, this, param, 0);
	if (!__m_spRunThread[0])
	{
		return false;
	}
	return true;
}

void CServer::stop()
{
	_pre_end();

	//维护线程退出
	if (__m_bMaintain)
	{
		SetEvent(_m_maintainEvent);
		__m_spMaintainThread->join();
		CloseHandle(_m_maintainEvent);
		_m_maintainEvent = NULL;
		__m_spMaintainThread = NULL;
	}
	//工作线程退出
	for (int i = 0; i < __m_nThread; i++)
	{
		SetEvent(_m_exitEvent[i]);
	}

	for (int i = 0; i < __m_nThread; i++)
	{
		if (__m_spRunThread[i])
		{
			__m_spRunThread[i]->join();
		}
	}
	for (int i = 0; i < __m_nThread; i++)
	{
		CloseHandle(_m_exitEvent[i]);
	}
	for (int i = 0; i < __m_nThread; i++)
	{
		__m_spRunThread[i] = NULL;
	}

	_end();
}

void CServer::__go(void* param, int nTreadPos)
{
	while (WAIT_TIMEOUT == WaitForSingleObject(_m_exitEvent[nTreadPos], 0))
	{
		_run(param, nTreadPos);
	}
}

void CServer::__go_maintain()
{
	int loopTime = 0;
	while (WAIT_TIMEOUT == WaitForSingleObject(_m_maintainEvent, loopTime))
	{
		loopTime = _m_maintainLoopTime;
		_maintain();
	}
}

void * CServer::createEvent()
{
	return CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CServer::_run(void * param, int nPos)
{
	CMsgPtr spMsg = std::dynamic_pointer_cast<CMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
}

bool CServer::receive(CMsgPtr spMsg)
{
	static UINT64 n = 0;
	n++;
	int i = n%__m_nThread;

	{
		std::lock_guard<std::mutex> guard(_m_listLock[i]);
		_m_msgList[i].push_back(spMsg);
	}
	_m_cond[i].notify_one();
	return true;
}

bool CServer::receive(CMsgPtr spMsg, int nPos)
{
	if (nPos >= __m_nThread)
	{
		nPos = 0;
	}
	{
		std::lock_guard<std::mutex> guard(_m_listLock[nPos]);
		_m_msgList[nPos].push_back(spMsg);
	}
	_m_cond[nPos].notify_one();
	return true;
}

bool CServer::receive(std::shared_ptr<std::list<CMsgPtr>> v_spMsg)
{
	static UINT64 n = 0;
	n++;
	int i = n%__m_nThread;

	{
		std::lock_guard<std::mutex> guard(_m_listLock[i]);
		_m_msgList[i].splice(_m_msgList[i].end(), *v_spMsg);
	}
	_m_cond[i].notify_one();
	return true;
}

CMsgPtr CServer::_msgPop(int nPos)
{
	std::unique_lock<std::mutex> lock(_m_listLock[nPos]);

	while (_m_msgList[nPos].empty()) {
		_m_cond[nPos].wait_for(lock, std::chrono::milliseconds(_m_loopTime));
		if (_m_msgList[nPos].empty())
		{
			return CMsgPtr();
		}
		else
		{
			break;
		}
	}
	CMsgPtr spMsg = _m_msgList[nPos].front();
	_m_msgList[nPos].pop_front();
	return spMsg;
}
