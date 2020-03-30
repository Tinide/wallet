#pragma once
#include "Server.h"
class CCoreServer :
	public CServer
{
	friend CServer;
public:
	CCoreServer();
	virtual ~CCoreServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _pre_end();
	virtual void _end();
	virtual bool _sendMsg(CMsgPtr spMsg);
};

