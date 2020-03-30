#include "CoreServer.h"
#include "base/msg/Protocol.h"
#include "base/common/proxy/Proxy.h"
CCoreServer::CCoreServer()
{
}


CCoreServer::~CCoreServer()
{
}

bool CCoreServer::_init()
{
	return true;
}

void CCoreServer::_run(void * param, int nPos)
{
	CProtocolPtr spProto = std::dynamic_pointer_cast<CProtocol>(_msgPop(nPos));
	if (!spProto)
	{
		return;
	}

	CRspMsgPtr spRspMsg = spProto->todo(this);

	_sendMsg(std::static_pointer_cast<CMsg>(spRspMsg));
}

void CCoreServer::_pre_end()
{
}

void CCoreServer::_end()
{
}

bool CCoreServer::_sendMsg(CMsgPtr spMsg)
{
	if (spMsg)
	{
		CProxy::sendToWsServer(spMsg);
	}
	return true;
}
