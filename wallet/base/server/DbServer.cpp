#include "DbServer.h"
#include "base/msg/Msg.h"


CDbServer::CDbServer()
{
}


CDbServer::~CDbServer()
{
}

bool CDbServer::_init()
{
	return true;
}

void CDbServer::_run(void * param, int nPos)
{
	CMsgPtr spMsg = std::dynamic_pointer_cast<CMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
	spMsg->putIntoDb();
}

void CDbServer::_pre_end()
{
}

void CDbServer::_end()
{
}
