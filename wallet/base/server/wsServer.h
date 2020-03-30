#pragma once
#include "Server.h"
#include "base/common/websocket/WebSocket.h"
#include "base/common/admin/WsAdmin.h"
class CWsServer :
	public CServer
{
	friend CServer;
protected:
	CWsServer();
	virtual ~CWsServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _pre_end();
	virtual void _end();
	virtual void _maintain();
protected:
	void onOpen(connection_hdl hdl);
	void onClose(connection_hdl hdl);
	void onFail(connection_hdl hdl);
	void onMessage(connection_hdl hdl, websocketppserver::message_ptr msgPtr);
	void onHttp(connection_hdl hdl);
	void onSocketInit(connection_hdl, asio::ip::tcp::socket & s);
	void onPong(connection_hdl hdl, std::string pangStr);
private:
	unsigned short __m_port;
	websocketppserver __m_websocktServer;//websocketºËÐÄasio·þÎñ
	std::vector<std::shared_ptr<std::thread>> __m_ts;
public:
	void killWsConn(connection_hdl hdl);
	std::string getClientIp(connection_hdl hdl);
};

