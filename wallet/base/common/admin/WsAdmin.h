#pragma once
#include "base\common\admin\Admin.h"
#include "base/msg/Msg.h"

struct Session
{
	websocket_hdl hdl;
	std::string uuid;
	std::string comment;
	JsonPtr request;

	bool operator==(const Session& session) const
	{
		if (uuid == session.uuid)
		{
			return true;
		}
		return false;
	}
};

class CWsClientInfo
{
public:
	CWsClientInfo();
	~CWsClientInfo() {}
public:
	void setLogin(int login) { __m_login = login; }
	int getLogin() { return __m_login; }

	void setToken(std::string token) { __m_token = token; }
	std::string getToken() { return __m_token; }

	int who() { return __m_login; }

	bool getSubTick() {return __m_bSubTick; }
	void setSubTick(bool bSubTick) { __m_bSubTick = bSubTick; }

	void setSession(int reqNo,Session session);
	void setSession(std::string orderRef, Session session);

	Session findSession(int reqNo);
	Session findSession(std::string orderRef);
private:
	int __m_login;
	std::string __m_token;
	bool __m_bSubTick;

	std::map<int, Session> __m_mReqIdUuid;//requestNo与uuid的map
	std::map<std::string, Session> __m_mRefUuid; //orderRef 与uuid的map
};

//ws管理类 客户端连接--客户端信息
class CWsAdmin:
	public CAdmin<websocket_spHdl, CWsClientInfo>
{
public:
	CWsAdmin();
	virtual ~CWsAdmin();
public:
	void setSubTick(websocket_spHdl sphdl, bool subTick);

	void pushReqNo(websocket_spHdl sphdl, int reqNo, Session session);
	void pushOrderRef(websocket_spHdl sphdl, int reqNo, std::string orderRef, Session session);
	Session findReqId(int reqNo);
	Session findRef(std::string orderRef);
};

extern CWsAdmin gWsAdmin;
typedef std::map< websocket_spHdl, CWsClientInfo > WsClientInfos;