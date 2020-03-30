#pragma once
#include "base\msg\Protocol.h"
class CQryTotalBalanceProto :
	public CProtocol
{
public:
	CQryTotalBalanceProto();
	virtual ~CQryTotalBalanceProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
};

