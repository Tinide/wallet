#pragma once
#include "base\server\Server.h"

struct WALLET
{
	std::string serverAddr;
	std::string address;
	std::string pwd;
	double fee;
	int minConfirmations;
	int block;

	WALLET()
	{
		minConfirmations = 0;
		fee = 0.0;
		block = 0;
	}
};

class CCoinServer :
	public CServer
{
public:
	CCoinServer();
	virtual ~CCoinServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
public:
	static CCoinServer* getCoinServer(std::string coin);
public:
	virtual std::string newAddress(std::string coin) = 0;
	virtual bool validateAddress(std::string coin, std::string address) = 0;
	virtual bool sendCoin(std::string coin, std::map<std::string, double> transPack, double totalTransferAmount, std::string& spent, std::string& txid, std::string& fee, int& code, std::string& codeMsg) = 0;
	virtual bool walletpassphrase(std::string coin, int seconds = 1, std::string address = "") = 0;
	virtual bool getConfirmations(std::string coin, std::string address, Json::Value& confirmations) = 0;//查询地址上未到确认次数的充值
	virtual bool getTransferStatus(std::string coin, std::vector<std::string> txids, std::vector<Json::Value>& transferStatus) = 0;
	virtual bool signrawtransaction(std::string coin,std::string& rawTransaction_noSign, std::string& rawTransaction , std::string from = "") = 0;
	virtual bool sendrawtransaction(std::string coin, std::string& rawTransaction, std::string& txid) = 0;
	virtual bool signFromColdWallet(std::string coin, const std::string& rawTransaction_noSign,const std::string& from, std::string& rawTransaction);
	virtual bool getTotalBalance(std::string coin, std::string& totalBalance) = 0;
private:
	static std::map<std::string, std::string> __m_coldWalletServer;
};

#define CoinServer(T) (CCoinServer::getCoinServer(T))

