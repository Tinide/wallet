#pragma once
#include "base/msg/Msg.h"
#include <list>
class CProxy
{
public:
	CProxy();
	virtual ~CProxy();
public:
	static void sendToPraseServer(CMsgPtr spMsg);
	static void sendToWsServer(CMsgPtr spMsg);
	static void sendToWsServer(std::shared_ptr<std::list<CMsgPtr>> spMsgList);
	static void sendToDbServer(CMsgPtr spMsg);
	static void sendToCoreServer(CMsgPtr spMsg);
	static void sendToCoreServer(std::shared_ptr<std::list<CMsgPtr>> spMsgList);
	static void sendToDepositServer(CMsgPtr spMsg);
	static void sendToEthShiftServer(CMsgPtr spMsg);
};

