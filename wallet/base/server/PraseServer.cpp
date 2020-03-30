#include "PraseServer.h"
#include "base/msg/ReqMsg.h"
#include "base/common/protocolHdl/ProtocolHdl.h"
#include "base/common/proxy/Proxy.h"
CPraseServer::CPraseServer()
{
}


CPraseServer::~CPraseServer()
{
}

bool CPraseServer::_init()
{
	return true;
}

void CPraseServer::_run(void * param, int nPos)
{
	CReqMsgPtr spMsg = std::dynamic_pointer_cast<CReqMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
	//进入解析，根据msg，获取cmd，生成相应的接口协议对象
	CProtocolHdl ptclHdl(spMsg);
	CProtocolPtr pctl = ptclHdl.handle();

	_sendMsg(std::static_pointer_cast<CMsg>(pctl));
}

void CPraseServer::_pre_end()
{
}

void CPraseServer::_end()
{
}

bool CPraseServer::_sendMsg(CMsgPtr spMsg)
{
	CProxy::sendToCoreServer(spMsg);
	return true;
}
