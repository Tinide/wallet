#include "ColdWalletProto.h"
#include "coin\server\CoinServer.h"


CColdWalletProto::CColdWalletProto()
{
}


CColdWalletProto::~CColdWalletProto()
{
}

bool CColdWalletProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_coin = JsonStr(data, "coin");
	__m_from = JsonStr(data, "from");
	__m_rawTrasaction = JsonStr(data, "rawTransaction_noSign");

	return true;
}

void CColdWalletProto::deal(void * pParam)
{
	std::string rawTransaction_sign;
	bool bret = CoinServer(__m_coin)->signrawtransaction(__m_coin, __m_rawTrasaction, rawTransaction_sign, __m_from);
	if (!bret)
	{
		_m_code = coldWallet_err;
		_m_msg = "cold wallet server is failed!";
		return;
	}

	_m_rspJson["rawTransaction"] = rawTransaction_sign;
}
