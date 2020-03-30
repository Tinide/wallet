#pragma once
#include "base\server\Server.h"
class CPraseServer :
	public CServer
{
	friend CServer;
protected:
	CPraseServer();
	virtual ~CPraseServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _pre_end();
	virtual void _end();
	virtual bool _sendMsg(CMsgPtr spMsg);
};

