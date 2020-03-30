#pragma once
#include "base/msg/Protocol.h"

class CLoginProto :
	public CProtocol
{
public:
	CLoginProto();
	virtual ~CLoginProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	int __m_login;
	std::string __m_pwd;
};

