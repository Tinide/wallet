#include "LoginProto.h"
#include "admin\WsAdmin.h"
#include "base/common/tool/Tool.h"
CLoginProto::CLoginProto()
{
}


CLoginProto::~CLoginProto()
{
}

bool CLoginProto::prase()
{
	//spJsonValue进行解析到成员变量
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_login =JsonInt(data,"login");
	__m_pwd = JsonStr(data,"password");

	return true;
}

void CLoginProto::deal(void* pParam)
{
	std::string token = newToken(__m_login);

	websocket_spHdl spHdl = gWsAdmin.whokey(__m_login);

	if (spHdl && (spHdl != _m_hdl.lock()))
	{
		LOG_INFO("[login:{}]连接已存在，清理老的连接信息", __m_login);
		gWsAdmin.clearWho(spHdl);
	}

	//登录成功
	CWsClientInfo client;
	client.setLogin(__m_login);
	client.setToken(token);
	gWsAdmin.push(_m_hdl.lock(), client);

	_m_rspJson["token"] = token;
	return;
}
