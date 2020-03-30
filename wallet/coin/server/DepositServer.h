#pragma once
#include "base\server\Server.h"
#include "coin/msg/DepositMsg.h"
class CDepositServer :
	public CServer
{
public:
	CDepositServer();
	virtual ~CDepositServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _maintain();
private:
	long long putIntodb(CDepositMsgPtr spMsg);
};

