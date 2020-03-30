#pragma once
#include "base\server\Server.h"
class CDbServer :
	public CServer
{
	friend CServer;
public:
	CDbServer();
	virtual ~CDbServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _pre_end();
	virtual void _end();
};

