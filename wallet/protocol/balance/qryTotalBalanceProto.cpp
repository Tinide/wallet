#include "qryTotalBalanceProto.h"
#include "coin/server/CoinServer.h"


CQryTotalBalanceProto::CQryTotalBalanceProto()
{
}


CQryTotalBalanceProto::~CQryTotalBalanceProto()
{
}

bool CQryTotalBalanceProto::prase()
{
	return true;
}

void CQryTotalBalanceProto::deal(void * pParam)
{
	std::string btcBalance;
	std::string ltcBalance;
	std::string bchBalance;
	std::string ethBalance;

	bool bret;
	bret = CoinServer("BTC")->getTotalBalance("BTC", btcBalance);
	bret = CoinServer("LTC")->getTotalBalance("LTC", ltcBalance);
	bret = CoinServer("BCH")->getTotalBalance("BCH", bchBalance);
	bret = CoinServer("ETH")->getTotalBalance("ETH", ethBalance);

	_m_rspJson["BTC"] = btcBalance;
	_m_rspJson["LTC"] = ltcBalance;
	_m_rspJson["BCH"] = bchBalance;
	_m_rspJson["ETH"] = ethBalance;
}
