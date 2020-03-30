#pragma once
#include "base\msg\Protocol.h"
class CQryDepositProto :
	public CProtocol
{
public:
	CQryDepositProto();
	virtual ~CQryDepositProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	std::string __m_token;
	std::string __m_address;
	std::string __m_coin;
};

