#include "Proxy.h"
#include "base/server/wsServer.h"
#include "base/server/SqlServer.h"
#include "base/server/PraseServer.h"
#include "base\server\DbServer.h"
#include "base/server/CoreServer.h"
#include "coin/server/DepositServer.h"
#include "coin/server/EthShiftServer.h"
CProxy::CProxy()
{
}


CProxy::~CProxy()
{
}


void CProxy::sendToPraseServer(CMsgPtr spMsg)
{
	Instance(CPraseServer)->receive(spMsg);
}

void CProxy::sendToWsServer(CMsgPtr spMsg)
{
	Instance(CWsServer)->receive(spMsg);
}

void CProxy::sendToWsServer(std::shared_ptr<std::list<CMsgPtr>> spMsgList)
{
	Instance(CWsServer)->receive(spMsgList);
}

void CProxy::sendToDbServer(CMsgPtr spMsg)
{
	Instance(CDbServer)->receive(spMsg);
}

void CProxy::sendToCoreServer(CMsgPtr spMsg)
{
	Instance(CCoreServer)->receive(spMsg);
}

void CProxy::sendToCoreServer(std::shared_ptr<std::list<CMsgPtr>> spMsgList)
{
	Instance(CCoreServer)->receive(spMsgList);
}

void CProxy::sendToDepositServer(CMsgPtr spMsg)
{
	Instance(CDepositServer)->receive(spMsg);
}

void CProxy::sendToEthShiftServer(CMsgPtr spMsg)
{
	Instance(CEthShiftServer)->receive(spMsg);
}
