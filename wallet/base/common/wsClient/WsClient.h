#pragma once
#include "../websocket/WebSocket.h"
#include <memory>
#include <thread>
#include "jsoncpp\jsonRead.h"
#include "base/msg/Msg.h"
struct WsContext
{
	std::shared_ptr<std::condition_variable> pCond;
	std::shared_ptr<std::mutex> plock;
	std::shared_ptr<std::string> pRespond;
};

class CWsClient :
	public CWebSocket
{
public:
	CWsClient();
	virtual ~CWsClient();
public:
	bool connect(std::string wsUrl = "");
	void disConnect();
	bool request(Json::Value req, Json::Value& rsp);
	void cast(Json::Value stone);
	bool isOk();
	bool ping();
	void bindNotify(std::function<void(CMsgPtr)> func);
	std::string getUrl() { return __m_wsUrl; }
private:
	template<class T>
	void create();
	template<class T>
	bool __connect();
	template<class T>
	void __disConnect();
	template<class T>
	void __send(std::string str);
	void realease();
private:
	void __on_open(connection_hdl hdl);
	void __on_close(connection_hdl hdl);
	void __on_fail(connection_hdl hdl);
	void __on_message(websocketpp::connection_hdl hdl, message_ptr msg);
	void __on_pong(connection_hdl hdl, std::string msg);
private:
	std::function<void(CMsgPtr)> __m_func;
	void* __m_pClient;
	connection_hdl __m_hdl;
	std::string __m_wsUrl;
	std::shared_ptr<std::thread> __m_spThread;
	int __m_iStatus;//-1 ß∞‹£¨0≥ı º£¨1≥…π¶

	std::condition_variable __m_pingCond;
	std::mutex __m_pingLock;
	std::string __m_pong;

	std::mutex __m_contextMtx;
	std::map<std::string, WsContext> __m_context;//uuid-WsContext
};

