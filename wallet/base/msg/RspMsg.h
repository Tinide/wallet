#pragma once
#include "base\msg\Msg.h"
class CRspMsg :
	public CMsg
{
public:
	CRspMsg();
	virtual ~CRspMsg();
};

typedef std::shared_ptr<CRspMsg> CRspMsgPtr;