#pragma once
#include "base\server\Server.h"
class CEthShiftServer :
	public CServer
{
public:
	CEthShiftServer();
	virtual ~CEthShiftServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
};

