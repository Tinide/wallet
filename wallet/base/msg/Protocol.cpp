#include "Protocol.h"
#include "base/common/log/log.h"
#include "admin\WsAdmin.h"
#include "base/common/tool\Tool.h"
CProtocol::CProtocol():_m_bEcho(false),_m_sendtoClient(true)
{
}


CProtocol::~CProtocol()
{
}

CRspMsgPtr CProtocol::todo(void* pParam)
{
	deal(pParam);
	if (!_m_sendtoClient)
	{
		return CRspMsgPtr();
	}
	//组包一个JSOn
	Json::Value rspValue;
	Json::FastWriter writer;
	std::string rsp;

	rspValue["code"] = _m_code;
	rspValue["message"] = _m_msg;
	rspValue["cmd"] = _m_cmd;
	rspValue["comment"] = _m_comment;
	if (rspValue["code"] == 0)
	{
		rspValue["data"] = _m_rspJson;
	}
	if (_m_bEcho)
	{
		rspValue["request"] = *(_m_spReqJson);
	}

	rsp = writer.write(rspValue);

	CRspMsgPtr spRspMsg = std::make_shared<CRspMsg>();
	spRspMsg->setWebsocketHdl(_m_hdl);
	spRspMsg->setUuid(_m_uuid);
	spRspMsg->setMsg(rsp);

	return spRspMsg;
}

bool CProtocol::_tokenIsValid(std::string token)
{
	std::string hex16 =token.substr(17, 8);
	int login = strtol(hex16.c_str(), NULL, 16);
	return verifyToken(token, login);
}
