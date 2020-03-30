#pragma once
#include "base\server\Server.h"
#include "base/common/wsClient/WsClient.h"

struct ConnStatus
{
	bool status;
	std::string token;
	time_t updateTime;

	ConnStatus()
	{
		status = false;
		token = "";
		updateTime = 0;
	}
};
typedef std::pair<std::shared_ptr<CWsClient>, ConnStatus> WsClientStatus;
typedef std::map<std::string, WsClientStatus> WsClientMap;//lp-(ws,status)

class CWsClientServer :
	public CServer
{
public:
	CWsClientServer();
	virtual ~CWsClientServer();
protected:
	//Î¬»¤Ïß³Ì
	virtual void _maintain();
	std::shared_ptr<CWsClient> _addWsClient(std::string wsClientName,std::string wsAddr);
protected:
	Json::Value __m_reqJson;
	std::mutex __m_wsClientMapMtx;
	WsClientMap __m_wsClientMap;
protected:
	virtual void initStatus();
	WsClientStatus getWsClientStatus(std::string wsClientName);
	std::shared_ptr<CWsClient> getWsClient(std::string wsClientName);
};