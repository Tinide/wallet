#pragma once
#include "base\msg\Protocol.h"
class CNewAddressProto :
	public CProtocol
{
public:
	CNewAddressProto();
	virtual ~CNewAddressProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	std::string __m_token;
	std::string __m_coin;
};

