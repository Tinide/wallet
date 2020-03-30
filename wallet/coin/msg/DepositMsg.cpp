#include "DepositMsg.h"



CDepositMsg::CDepositMsg()
{
}


CDepositMsg::~CDepositMsg()
{
}

void CDepositMsg::setCoin(std::string coin)
{
	__m_coin = coin;
}

void CDepositMsg::setTxid(std::string txid)
{
	__m_txid = txid;
}

void CDepositMsg::setVout(int vout)
{
	__m_vout = vout;
}

void CDepositMsg::setAddress(std::string address)
{
	__m_address = address;
}

void CDepositMsg::setAmount(std::string amount)
{
	__m_amount = amount;
}

void CDepositMsg::setConfirmations(int confirmations)
{
	__m_confirmations = confirmations;
}
