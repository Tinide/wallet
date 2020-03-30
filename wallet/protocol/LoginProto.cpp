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
	//spJsonValue���н�������Ա����
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
		LOG_INFO("[login:{}]�����Ѵ��ڣ������ϵ�������Ϣ", __m_login);
		gWsAdmin.clearWho(spHdl);
	}

	//��¼�ɹ�
	CWsClientInfo client;
	client.setLogin(__m_login);
	client.setToken(token);
	gWsAdmin.push(_m_hdl.lock(), client);

	_m_rspJson["token"] = token;
	return;
}
