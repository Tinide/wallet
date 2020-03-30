#include "QryDepositProto.h"
#include "coin/server/CoinServer.h"
#include "base/common/tool/Tool.h"

CQryDepositProto::CQryDepositProto()
{
}


CQryDepositProto::~CQryDepositProto()
{
}

bool CQryDepositProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	__m_address = JsonStr(data, "address");
	__m_coin = JsonStr(data, "coin");
	return true;
}

void CQryDepositProto::deal(void * pParam)
{
	if (!verifyToken(__m_token))
	{
		_m_code = token_err;
		_m_msg = "token is invalid";
		return;
	}

	bool bret = CoinServer(__m_coin)->validateAddress(__m_coin,__m_address);
	if (!bret)
	{
		_m_code = addr_err;
		_m_msg = string_format("%s address is invalid",__m_coin.c_str());
		return;
	}

	Json::Value confirmations;
	bret = CoinServer(__m_coin)->getConfirmations(__m_coin,__m_address, confirmations);
	if (!bret)
	{
		_m_code = qry_err;
		_m_msg = "query confirmations failed";
		return;
	}
	_m_rspJson["coin"] = __m_coin;
	_m_rspJson["confirmations"] = confirmations;
}
