#include "CoinServer.h"
#include "BtcServer.h"
#include "EthServer.h"
#include "base/common/http/HttpClient.h"
#include "base/common/markup/RWConfig.h"

std::map<std::string, std::string> CCoinServer::__m_coldWalletServer;

CCoinServer::CCoinServer()
{
	__m_coldWalletServer["BTC"] = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "coldWallet", "btcAddr");
	__m_coldWalletServer["LTC"] = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "coldWallet", "ltcAddr");
	__m_coldWalletServer["BCH"] = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "coldWallet", "bchAddr");
	__m_coldWalletServer["ETH"] = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "coldWallet", "ethAddr");
}


CCoinServer::~CCoinServer()
{
}

bool CCoinServer::_init()
{
	return false;
}

void CCoinServer::_run(void * param, int nPos)
{
	CMsgPtr spMsg = std::dynamic_pointer_cast<CMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
}

CCoinServer * CCoinServer::getCoinServer(std::string coin)
{
	CCoinServer* server = Instance(CBtcServer);
	if (coin == "ETH")
	{
		server = Instance(CEthServer);
	}
	
	return server;
}

bool CCoinServer::signFromColdWallet(std::string coin, const std::string & rawTransaction_noSign, const std::string & from, std::string & rawTransaction)
{
	CHttpClient hc;
	Json::Value rsp;

	Json::Value req;
	req["cmd"] = "coldWallet";
	Json::Value data;
	data["coin"] = coin;
	data["rawTransaction_noSign"] = rawTransaction_noSign;
	data["from"] = from;

	req["data"] = data;

	bool bret = hc.post(__m_coldWalletServer[coin], req, rsp, false);
	if (bret)
	{
		if (JsonInt(rsp, "code") == 0)
		{
			rawTransaction = JsonStr(JsonVal(rsp,"data"), "rawTransaction");
			return true;
		}
	}
	return false;
}
