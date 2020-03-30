#pragma once
#include "base\msg\Msg.h"
class CEthShiftMsg :
	public CMsg
{
public:
	CEthShiftMsg();
	virtual ~CEthShiftMsg();
public:
	void setAmount(std::string amount) { __m_amount = amount; }
	void setAddress(std::string address) { __m_address = address; }

	std::string getAddress() { return __m_address; }
	std::string getAmount() { return __m_amount; }
private:
	std::string __m_amount;
	std::string __m_address;
};

typedef std::shared_ptr<CEthShiftMsg> CEthShiftMsgPtr;