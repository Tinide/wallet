#pragma once
#include "CoinServer.h"
struct UNSPENT
{
	std::string txid;
	int vout;
	double amount;
	int confirmations;
};

class CBtcServer :
	public CCoinServer
{
public:
	CBtcServer();
	virtual ~CBtcServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _maintain();
private:
	bool qryNewBlockInfo(std::string coin);//返回值=0，表示没有新的区块。返回值>0，表示有新的区块产生
	bool getUnspent(std::string coin, double amount, std::vector<UNSPENT>& vUnspents);
	bool createrawtransaction(std::string coin, std::vector<UNSPENT>& vUnspents, std::map<std::string, double>& vTransferPack, std::string& rawTransaction_noSign, std::string& spent);
public:
	virtual std::string newAddress(std::string coin);
	virtual bool validateAddress(std::string coin,std::string btcAddr);
	virtual bool sendCoin(std::string coin, std::map<std::string, double> transPack, double totalTransferAmount, std::string& spent,std::string& txid, std::string& fee, int& code, std::string& codeMsg);
	virtual bool walletpassphrase(std::string coin,int seconds = 1, std::string address = "");
	virtual bool signrawtransaction(std::string coin,std::string& rawTransaction_noSign, std::string& rawTransaction, std::string from = "");
	virtual bool sendrawtransaction(std::string coin,std::string& rawTransaction,std::string& txid);
	virtual bool getConfirmations(std::string coin,std::string address,Json::Value& confirmations);
	virtual bool getTransferStatus(std::string coin,std::vector<std::string> txids,std::vector<Json::Value>& transferStatus);
	virtual bool getTotalBalance(std::string coin, std::string& totalBalance);
private:
	std::map<std::string, WALLET> __m_wallet;
};

