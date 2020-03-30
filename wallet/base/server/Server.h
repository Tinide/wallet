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
	//����ʼ��ʼ��
	virtual bool _init() = 0;
	//����Ԥ����
	virtual void _pre_end() {}
	//���������ͷ���Դ
	virtual void _end() {}
	//������������
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
	std::shared_ptr<std::thread> __m_spRunThread[MAXTHREADSIZE];//���ķ����߳̾��
	int __m_nThread;
	std::shared_ptr<std::thread> __m_spMaintainThread;//������ά���߳�
protected:
	void* _m_exitEvent[MAXTHREADSIZE];//�����˳�����
	void* _m_maintainEvent;//ά���߳��˳�����
protected:
	std::condition_variable _m_cond[MAXTHREADSIZE];
	std::mutex _m_listLock[MAXTHREADSIZE];
	std::list<CMsgPtr> _m_msgList[MAXTHREADSIZE];//��Ϣ�������

	std::string _m_serverName;
	int _m_loopTime;
	bool __m_bMaintain;
	int _m_maintainLoopTime;//Ĭ��5000
};

