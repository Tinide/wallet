#include "QryWithdrawProto.h"
#include "coin/server/BtcServer.h"


CQryWithdrawProto::CQryWithdrawProto()
{
}


CQryWithdrawProto::~CQryWithdrawProto()
{
}

bool CQryWithdrawProto::prase()
{
	//spJsonValue进行解析到成员变量
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	__m_coin = JsonStr(data, "coin");
	
	Json::Value txids = JsonArr(data, "txids");
	for (int i = 0; i < (int)txids.size(); i++)
	{
		__m_txids.push_back(JsonPos(txids, i).asString());
	}

	return true;
}

void CQryWithdrawProto::deal(void * pParam)
{
	if (__m_txids.size() == 0)
	{
		_m_code = qry_err;
		_m_msg = "txids is null";
		return;
	}

	std::vector<Json::Value> transferStatus;
	bool bret = true;//Instance(CBtcServer)->getTransferStatus(__m_coin,__m_txids, transferStatus);
	if (bret)
	{
		for (auto it : transferStatus)
		{
			_m_rspJson["withdrawStatus"].append(it);
		}
		_m_rspJson["coin"] = __m_coin;
	}
	else
	{
		_m_code = qry_err;
		_m_msg = "query err";
		return;
	}
}
