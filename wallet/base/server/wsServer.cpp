#include "wsServer.h"
#include "base/msg/Msg.h"
#include "base/common/proxy/Proxy.h"
#include "base/common/markup/Markup.h"
#include "base/common/protocolHdl/ProtocolHdl.h"
#include "base/common/tool/Tool.h"
#define IOCP_SIZE 4
CWsServer::CWsServer()
{
	_m_maintainLoopTime = 30 * 1000;
	__m_port = 5186;
	//初始化webSocket
	__m_websocktServer.set_access_channels(websocketpp::log::alevel::none);
	__m_websocktServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

	// Initialize Asio Transport
	__m_websocktServer.init_asio();

	// Register handler callbacks
	__m_websocktServer.set_open_handler(bind(&CWsServer::onOpen, this, ::_1));
	__m_websocktServer.set_close_handler(bind(&CWsServer::onClose, this, ::_1));
	__m_websocktServer.set_fail_handler(bind(&CWsServer::onFail, this, ::_1));
	__m_websocktServer.set_message_handler(bind(&CWsServer::onMessage, this, ::_1, ::_2));
	__m_websocktServer.set_http_handler(bind(&CWsServer::onHttp, this, ::_1));
	__m_websocktServer.set_socket_init_handler(bind(&CWsServer::onSocketInit, this, ::_1, ::_2));
	__m_websocktServer.set_pong_handler(bind(&CWsServer::onPong, this, ::_1, ::_2));
}


CWsServer::~CWsServer()
{
	for (size_t i = 0; i < __m_ts.size(); i++) {
		if (__m_ts[i])
		{
			__m_ts[i]->join();
		}
	}
}

bool CWsServer::_init()
{
	__m_websocktServer.set_reuse_addr(true);
	__m_websocktServer.set_listen_backlog(SOMAXCONN);

	__m_websocktServer.listen(__m_port);

	__m_websocktServer.start_accept();

	// Start the ASIO io_service run loop
	try {
		for (size_t i = 0; i < IOCP_SIZE; i++) {
			__m_ts.push_back(std::make_shared<std::thread>(&websocketppserver::run, &__m_websocktServer));
		}
	}
	catch (const std::exception & e) {
		LOG_ERROR(e.what());
	}

	return true;
}

void CWsServer::_run(void * param, int nPos)
{
	std::shared_ptr<CRspMsg> spMsg = std::dynamic_pointer_cast<CRspMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
	if (spMsg->getWebsocketHdl().lock())
	{
		try
		{
			__m_websocktServer.send(spMsg->getWebsocketHdl(),spMsg->getUuid()+CMarkup::AToUTF8(spMsg->getMsg()), websocketpp::frame::opcode::value::text);
			if (spMsg->getLogPrint())
			{
				LOG_INFO("[{}]|{}:{}", spMsg->getWebsocketHdl().lock().get(), spMsg->getUuid(), spMsg->getMsg());
			}
		}
		catch (std::exception& e)
		{
			LOG_ERROR("wsHdl发送失败,{}", e.what());
		}
	}
}

void CWsServer::_pre_end()
{
}

void CWsServer::_end()
{
	gWsAdmin.clear();

	try
	{
		__m_websocktServer.stop();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("{},{}", __FUNCTION__, e.what());
	}
}

void CWsServer::_maintain()
{
	std::map<websocket_spHdl, CWsClientInfo> m = gWsAdmin.getall();
	for (auto it : m)
	{
		try
		{
			__m_websocktServer.ping(it.first, "ping");
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("{},{}", __FUNCTION__, e.what());

			//socket连接已失效，将其kill，并清除其连接信息
			killWsConn(it.first);
			gWsAdmin.pull(it.first);
		}
	}
}

void CWsServer::onOpen(connection_hdl hdl)
{
	gWsAdmin.push(hdl.lock());
}

void CWsServer::onClose(connection_hdl hdl)
{
	gWsAdmin.pull(hdl.lock());
}

void CWsServer::onFail(connection_hdl hdl)
{
	gWsAdmin.pull(hdl.lock());
}

void CWsServer::onMessage(connection_hdl hdl, websocketppserver::message_ptr msgPtr)
{
	std::string msg = msgPtr->get_payload();
	if (msg.size() <= 32)
	{
		return;
	}

	CReqMsgPtr spReqMsg = std::make_shared<CReqMsg>();
	spReqMsg->setMsg(msg.substr(32));
	spReqMsg->setWebsocketHdl(hdl);
	spReqMsg->setUuid(msg.substr(0, 32));
	CProxy::sendToPraseServer(std::static_pointer_cast<CMsg>(spReqMsg));
}

void CWsServer::onHttp(connection_hdl hdl)
{
	//http socket句柄
	conn_ptr conn_ptr = __m_websocktServer.get_con_from_hdl(hdl);

	//http 请求包
	websocketpp::http::parser::request req = conn_ptr->get_request();
	const std::string& strUri = req.get_uri();
	std::map<std::string, std::string> params = praseParams(decodeUrl(strUri));
	const std::string& strMethod = req.get_method();
	const std::string strBody = req.get_body();

	//reqMsg
	CReqMsgPtr spReqMsg = std::make_shared<CReqMsg>();
	if (strMethod == "GET")
	{
		int pos = (int)strUri.find("?");
		std::string cmd;
		if (pos == -1)
		{
			cmd = strUri.substr(1);
		}
		else
		{
			cmd = strUri.substr(1, pos - 1);
		}
		Json::Value data;
		for (auto it : params)
		{
			data[it.first] = it.second;
		}
		Json::Value req;
		req["cmd"] = cmd;
		req["data"] = data;
		spReqMsg->setMsg(Json::FastWriter().write(req));
	}
	else
	{
		spReqMsg->setMsg(strBody);
	}
	spReqMsg->setWebsocketHdl(hdl);

	//生成prtc
	CProtocolHdl ptclHdl(spReqMsg);
	std::shared_ptr<CProtocol> spPrtc = ptclHdl.handle();

	//prtc处理
	CMsgPtr spRspMsg = spPrtc->todo(this);
	

	//日志是否打印
	if (spRspMsg->getLogPrint())
	{
		LOG_INFO("[{}|http]:{}", hdl.lock().get(),spRspMsg->getMsg());
	}

	//http组包
	conn_ptr->set_body(spRspMsg->getUuid() + CMarkup::AToUTF8(spRspMsg->getMsg()));
	conn_ptr->set_status(websocketpp::http::status_code::ok);
	conn_ptr->append_header("Content-Type", "text/plain; charset=utf-8");
	//发送http

	websocketpp::lib::error_code ec;
	try
	{
		__m_websocktServer.send_http_response(hdl, ec);
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("e.what:{},ec:{}", e.what(), ec.message());
	}
}

void CWsServer::onSocketInit(connection_hdl, asio::ip::tcp::socket & s)
{
	asio::ip::tcp::no_delay option(true);
	s.set_option(option);
}

void CWsServer::onPong(connection_hdl hdl, std::string pangStr)
{
}

void CWsServer::killWsConn(connection_hdl hdl)
{
	std::string reason;
	try
	{
		__m_websocktServer.close(hdl, websocketpp::close::status::normal, reason);
	}
	catch (const std::exception&e)
	{
		LOG_ERROR("{},{}", __FUNCTION__, e.what());
	}
}

std::string CWsServer::getClientIp(connection_hdl hdl)
{
	conn_ptr conn_ptr = __m_websocktServer.get_con_from_hdl(hdl);

	return conn_ptr->get_socket().remote_endpoint().address().to_string();
}
