#include "EthShiftServer.h"
#include "coin/msg/EthShiftMsg.h"
#include "coin/server/EthServer.h"

CEthShiftServer::CEthShiftServer()
{
}


CEthShiftServer::~CEthShiftServer()
{
}

bool CEthShiftServer::_init()
{
	return true;
}

void CEthShiftServer::_run(void * param, int nPos)
{
	CEthShiftMsgPtr spMsg = std::dynamic_pointer_cast<CEthShiftMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
	bool bret = Instance(CEthServer)->shiftEth(spMsg->getAddress(), spMsg->getAmount());
	if (!bret)
	{
		LOG_INFO("×Ê½ð×ªÒÆÊ§°Ü!address:{} amount:{}", spMsg->getAddress(), spMsg->getAmount());
	}
}
