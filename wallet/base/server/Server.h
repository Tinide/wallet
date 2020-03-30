#pragma once
#include <thread>
#include <memory>
#include "../msg/Msg.h"
#include <mutex>
#include <list>
#include "../common/log/log.h"
#include "../common/pool/Pool.h"
#define MAXTHREADSIZE 32
#define Instance(T) (CServer::instance<T>())
class CServer
{
protected:
	CServer();
	virtual ~CServer();
public:
	template<class T>
	static T* instance()
	{
		static T instance;
		return dynamic_cast<T*>(&instance);
	}
public:
	bool start( void* param = NULL);

	template<class T = int>
	bool start(int nThread,bool bMaintain = false,T* param = NULL)
	{
		__m_nThread = nThread;
		if (!_init())
		{
			return false;
		}
		T* temp = NULL;
		for (int i = 0; i < nThread; i++)
		{
			_m_exitEvent[i] = createEvent();
			if (!_m_exitEvent[i])
			{
				return false;
			}
			if (param)
			{
				temp = param + i;
			}
			else
			{
				temp = NULL;
			}
			__m_spRunThread[i] = std::make_shared<std::thread>(&CServer::__go, this, temp, i);
			if (!__m_spRunThread[i])
			{
				return false;
			}
		}
		if (bMaintain)
		{
			__m_bMaintain = bMaintain;
			_m_maintainEvent = createEvent();
			if (!_m_maintainEvent)
			{
				return false;
			}
			__m_spMaintainThread = std::make_shared<std::thread>(&CServer::__go_maintain, this);
		}
		return true;
	}

	void stop();
private:
	virtual void __go(void* param,int nTreadPos);
	virtual void __go_maintain();
	void* createEvent();
protected:
	//服务开始初始化
	virtual bool _init() = 0;
	//结束预处理
	virtual void _pre_end() {}
	//结束处理，释放资源
	virtual void _end() {}
	//服务启动程序
	virtual void _run(void* param, int nPos = 0);
	virtual void _maintain() {};
	virtual bool _sendMsg(CMsgPtr spMsg) { return true; }
	virtual bool _sendMsg(std::shared_ptr<std::list<CMsgPtr>> l_spMsg) { return true; }
public:
	virtual bool receive(CMsgPtr spMsg);
	virtual bool receive(CMsgPtr spMsg,int nPos);
	virtual bool receive(std::shared_ptr<std::list<CMsgPtr>> l_spMsg);
protected:
	virtual CMsgPtr _msgPop(int nPos);
private:
	std::shared_ptr<std::thread> __m_spRunThread[MAXTHREADSIZE];//核心服务线程句柄
	int __m_nThread;
	std::shared_ptr<std::thread> __m_spMaintainThread;//服务框架维护线程
protected:
	void* _m_exitEvent[MAXTHREADSIZE];//服务退出条件
	void* _m_maintainEvent;//维护线程退出条件
protected:
	std::condition_variable _m_cond[MAXTHREADSIZE];
	std::mutex _m_listLock[MAXTHREADSIZE];
	std::list<CMsgPtr> _m_msgList[MAXTHREADSIZE];//消息请求队列

	std::string _m_serverName;
	int _m_loopTime;
	bool __m_bMaintain;
	int _m_maintainLoopTime;//默认5000
};

