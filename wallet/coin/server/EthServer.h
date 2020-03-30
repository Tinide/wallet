#pragma once
#include "CoinServer.h"
#include "coin/msg/DepositMsg.h"
class CEthServer :
	public CCoinServer
{
public:
	CEthServer();
	virtual ~CEthServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _maintain();
private:
	void getAllEthAddress();
	int qryNewBlockInfo();
	void getBlockByNumber(int block);
public:
	static std::string Eth2Num(std::string ethNum);//以太坊数字(10,16进制的）-》10进制数字
	static std::string Num2Eth(std::string num);//10,16进制数字-》以太坊数字
public:
	virtual std::string newAddress(std::string coin);
	virtual bool validateAddress(std::string coin, std::string address);
	virtual bool sendCoin(std::string coin, std::map<std::string, double> transPack, double totalTransferAmount, std::string& spent, std::string& txid, std::string& fee, int& code, std::string& codeMsg);
	virtual bool walletpassphrase(std::string coin, int seconds = 1,std::string address = "");
	virtual bool getConfirmations(std::string coin, std::string address, Json::Value& confirmations);
	virtual bool getTransferStatus(std::string coin, std::vector<std::string> txids, std::vector<Json::Value>& transferStatus);
	virtual bool signrawtransaction(std::string coin, std::string& rawTransaction_noSign, std::string& rawTransaction, std::string from = "");
	virtual bool sendrawtransaction(std::string coin, std::string& rawTransaction, std::string& txid);
	virtual bool getTotalBalance(std::string coin, std::string& totalBalance);
	bool sendTransaction(const std::string& from, const std::string& to, std::string& value, std::string& spent, std::string& txid, int& code, std::string& codeMsg);
	bool shiftEth(std::string address,std::string amount);
private:
	WALLET __m_wallet;

	std::mutex __m_allEthAddressMtx;
	std::unordered_map<std::string,std::string> __m_allEthAddress;//value暂时不启动

	std::mutex __m_depositMsgMapMtx;
	std::unordered_map<int, std::list<CDepositMsgPtr>> __m_depositMsgMap;
};