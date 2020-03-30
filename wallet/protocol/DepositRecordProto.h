#pragma once
#include "base\msg\Protocol.h"
class CDepositRecordProto :
	public CProtocol
{
public:
	CDepositRecordProto();
	virtual ~CDepositRecordProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	std::string __m_token;
	int __m_begin;
	int __m_end;
};

