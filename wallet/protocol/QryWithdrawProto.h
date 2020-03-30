#pragma once
#include "base\msg\Protocol.h"
class CQryWithdrawProto :
	public CProtocol
{
public:
	CQryWithdrawProto();
	virtual ~CQryWithdrawProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	std::string __m_token;
	std::vector<std::string> __m_txids;
	std::string __m_coin;
};

