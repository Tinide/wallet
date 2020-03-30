#pragma once
#include "base\msg\Protocol.h"
class CWithdrawProto :
	public CProtocol
{
public:
	CWithdrawProto();
	virtual ~CWithdrawProto();
public:
	virtual bool prase();
	virtual void deal(void* pParam);
private:
	bool verifyCoinAddr();
	void putIntoDb(std::string coin,std::string& spent,std::string& txid);
private:
	std::string __m_token;
	std::map<std::string, double> __m_transPack;
	double __m_totalTransferAmount;
	std::string __m_coin;
};

