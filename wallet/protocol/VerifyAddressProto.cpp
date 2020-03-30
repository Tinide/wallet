#include "VerifyAddressProto.h"
#include "base/common/tool/Tool.h"
#include "coin/server/CoinServer.h"

CVerifyAddressProto::CVerifyAddressProto()
{
}


CVerifyAddressProto::~CVerifyAddressProto()
{
}

bool CVerifyAddressProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	__m_address = JsonStr(data, "address");
	__m_coin = JsonStr(data, "coin");

	return true;
}

void CVerifyAddressProto::deal(void * pParam)
{
	if (!verifyToken(__m_token))
	{
		_m_code = token_err;
		_m_msg = "token is invalid";
		return;
	}
	bool bret = CoinServer(__m_coin)->validateAddress(__m_coin, __m_address);
	if (!bret)
	{
		_m_code = addr_err;
		_m_msg = string_format("%s address is invalid",__m_coin.c_str());
		return;
	}
}
