#pragma once
#include "base\msg\Protocol.h"
class CColdWalletProto :
	public CProtocol
{
public:
	CColdWalletProto();
	virtual ~CColdWalletProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	std::string __m_rawTrasaction;
	std::string __m_coin;
	std::string __m_from;
};

