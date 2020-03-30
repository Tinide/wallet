#pragma once
#include "base\msg\Msg.h"
class CDepositMsg :
	public CMsg
{
public:
	CDepositMsg();
	virtual ~CDepositMsg();
public:
	void setCoin(std::string coin);
	void setTxid(std::string txid);
	void setVout(int vout);
	void setAddress(std::string address);
	void setAmount(std::string amount);
	void setConfirmations(int confirmations);

	std::string getCoin() { return __m_coin; }
	std::string getTxid() { return __m_txid; }
	int getVout() { return __m_vout; }
	std::string getAddress() { return __m_address; }
	std::string getAmount() { return __m_amount; }
	int getConfirmations() { return __m_confirmations; }
private:
	std::string __m_coin;
	std::string __m_txid;
	int __m_vout;
	std::string __m_address;
	std::string __m_amount;
	int __m_confirmations;
};

typedef std::shared_ptr<CDepositMsg> CDepositMsgPtr;