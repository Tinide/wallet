#pragma once
#include "base/msg/ReqMsg.h"
#include "base/msg/Protocol.h"
class CProtocolHdl
{
public:
	CProtocolHdl(CReqMsgPtr spReqMsg);
	virtual ~CProtocolHdl();

	CProtocolPtr handle();
private:
	CProtocolPtr buildProtocol(JsonPtr spJson);
private:
	CReqMsgPtr __m_spReqMsg;
};

