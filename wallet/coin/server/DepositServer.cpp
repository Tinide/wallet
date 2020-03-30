#include "DepositServer.h"
#include "base/common/admin/WsAdmin.h"
#include "base/msg/RspMsg.h"
#include "base/common/proxy/Proxy.h"
#include "base/common/tool/Tool.h"
#include "base/server/SqlServer.h"
CDepositServer::CDepositServer()
{
}


CDepositServer::~CDepositServer()
{
}

bool CDepositServer::_init()
{
	return true;
}

void CDepositServer::_run(void * param, int nPos)
{
	CDepositMsgPtr spMsg = std::dynamic_pointer_cast<CDepositMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
	//将记录插入数据库
	long long record = putIntodb(spMsg);
	if (!record)
	{
		return;
	}

	Json::Value msg;
	
	Json::Value data;
	data["record"] = record;
	data["coin"] = spMsg->getCoin();
	data["txid"] = spMsg->getTxid();
	data["vout"] = spMsg->getVout();
	data["address"] = spMsg->getAddress();
	data["amount"] = spMsg->getAmount();
	data["confirmations"] = spMsg->getConfirmations();
	data["create_time"] = getNow();

	msg["data"] = data;
	msg["cmd"] = "deposit";

	std::string message = Json::FastWriter().write(msg);

	WsClientInfos clients = gWsAdmin.getall();
	std::shared_ptr<std::list<CMsgPtr>> msgList = std::make_shared<std::list<CMsgPtr>>();
	for (auto it : clients)
	{
		CRspMsgPtr spMsg = std::make_shared<CRspMsg>();
		spMsg->setWebsocketHdl(it.first);
		spMsg->setMsg(message);
		spMsg->setUuid("00000000000000000000000000000000");
		msgList->push_back(std::static_pointer_cast<CMsg>(spMsg));

	}
	if (msgList->size())
	{
		CProxy::sendToWsServer(msgList);
	}
}

void CDepositServer::_maintain()
{
}

long long CDepositServer::putIntodb(CDepositMsgPtr spMsg)
{
	int sql_errcode = 0;

	for (int i = 0; i < 10; i++ )//已存在记录
	{
		std::string sql = string_format("call insertDeposit('%s','%s',%d,'%s','%s',@record)", spMsg->getCoin().c_str(),spMsg->getTxid().c_str(), spMsg->getVout(), spMsg->getAddress().c_str(), spMsg->getAmount().c_str());
		DataHelper helper = Instance(CSqlServer)->getConn();
		if (helper)
		{
			try
			{
				helper->execute(sql);

				sql = "select @record";
				ResultSet rs = helper->query(sql);

				helper->commit();

				if (rs.size())
				{
					long long id = atoi(rs[0]["@record"].c_str());
					return id;
				}
			}
			catch (std::exception& e)
			{
				sql_errcode = atoi(e.what());
				if (sql_errcode == 1062)
				{
					break;
				}
				LOG_ERROR("sql:{},err:{}", sql, e.what());
				helper->rollback();
			}
		}
		Sleep(1000);
	}
	return 0;
}
