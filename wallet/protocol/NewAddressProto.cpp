#include "NewAddressProto.h"
#include "coin/server/BtcServer.h"
#include "base/common/tool/Tool.h"

CNewAddressProto::CNewAddressProto()
{
}


CNewAddressProto::~CNewAddressProto()
{
}

bool CNewAddressProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	__m_coin = JsonStr(data, "coin");
	return true;
}

void CNewAddressProto::deal(void * pParam)
{
	if (!verifyToken(__m_token))
	{
		_m_code = token_err;
		_m_msg = "token is invalid";
		return;
	}
	std::string btcAddr = CoinServer(__m_coin)->newAddress(__m_coin);
	if (btcAddr.size() == 0)
	{
		_m_code = newAddr_err;
		_m_msg = "创建新地址失败！";
		return;
	}
	_m_rspJson["coin"] = __m_coin;
	_m_rspJson["address"] = btcAddr;
}
