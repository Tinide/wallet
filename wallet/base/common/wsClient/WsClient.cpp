#include "WsClient.h"
#include "../log/log.h"
#include "../tool/Tool.h"
#include "base/msg/Msg.h"
#include "base/common/proxy/Proxy.h"
#include "base/common/markup/RWConfig.h"

#define NET_TIMEOUT 5*1000

CWsClient::CWsClient():__m_iStatus(0)
{
}

CWsClient::~CWsClient()
{
	realease();
}

context_ptr on_tls_init(websocketpp::connection_hdl hdl)
{
	namespace asio = websocketpp::lib::asio;

	context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tlsv1);

	return ctx;
}

bool CWsClient::connect(std::string wsUrl)
{
	realease();
	if (wsUrl != "")
	{
		__m_wsUrl = wsUrl;
	}

	bool bret = false;
	if (__m_wsUrl.substr(0, 3) == "wss")
	{
		__m_pClient = (void*)new WebsocketsClient;
		((WebsocketsClient*)__m_pClient)->set_tls_init_handler(bind(&on_tls_init, ::_1));
		create<WebsocketsClient>();
		bret = __connect<WebsocketsClient>();
	}
	else
	{
		__m_pClient = (void*)new WebsocketClient;
		create<WebsocketClient>();
		bret = __connect<WebsocketClient>();
	}

	return bret;
}

void CWsClient::disConnect()
{
	if (__m_wsUrl.substr(0, 3) == "wss")
	{
		__disConnect<WebsocketsClient>();
	}
	else
	{
		__disConnect<WebsocketClient>();
	}
}

bool CWsClient::request(Json::Value req, Json::Value & rsp)
{
	if (1 != __m_iStatus || !__m_pClient)
	{
		return false;
	}

	std::string uuid = getUuid();
	std::string reqStr = uuid + Json::FastWriter().write(req);

	WsContext context;
	context.plock = std::make_shared<std::mutex>();
	context.pCond = std::make_shared<std::condition_variable>();
	context.pRespond = std::make_shared<std::string>();

	{
		std::lock_guard<std::mutex> guard(__m_contextMtx);
		__m_context[uuid] = context;
	}

	bool bRet = false;
	try
	{
		{
			std::unique_lock<std::mutex> guard(*context.plock);

			if (__m_wsUrl.substr(0, 3) == "wss")
			{
				__send<WebsocketsClient>(reqStr);
			}
			else
			{
				__send<WebsocketClient>(reqStr);
			}

			LOG_INFO("[{}]:{}",__m_wsUrl,reqStr);

			context.pCond->wait_for(guard, std::chrono::milliseconds(5000));

			if (context.pRespond->size())
			{
				Json::Reader reader;
				if (reader.parse(*context.pRespond, rsp))
				{
					bRet = true;
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				bRet = false;
			}
		}
	}
	catch (std::exception& e)
	{
		LOG_ERROR("{}", e.what());
	}

	{
		std::lock_guard<std::mutex> guard(__m_contextMtx);
		__m_context.erase(uuid);
	}

	return bRet;
}

void CWsClient::cast(Json::Value stone)
{
	std::string uuid = getUuid();
	std::string reqStr = uuid + Json::FastWriter().write(stone);

	if (__m_wsUrl.substr(0, 3) == "wss")
	{
		__send<WebsocketsClient>(reqStr);
	}
	else
	{
		__send<WebsocketClient>(reqStr);
	}
}

void CWsClient::__on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
	std::string respond = msg->get_payload();
	
	if (respond.size() <= 32)
	{
		return;
	}
	std::string uuid = respond.substr(0, 32);
	std::string rsp = respond.substr(32);

	rsp = CMarkup::UTF8ToA(rsp);

	WsContext context;
	{
		std::lock_guard<std::mutex> guard(__m_contextMtx);
		auto it = __m_context.find(uuid);
		if (it == __m_context.end())
		{
			if (rsp.find("\"cmd\":\"depth\"") == std::string::npos)
			{
				LOG_INFO("[{}]|{}{}", __m_wsUrl, uuid, rsp);
			}

			CMsgPtr msg = std::make_shared<CMsg>();
			msg->setMsg(rsp);
			if (__m_func)
			{
				__m_func(msg);
			}
			return;
		}
		else
		{
			LOG_INFO("[{}]|{}{}", __m_wsUrl, uuid, rsp);

			context = it->second;
			{
				std::lock_guard<std::mutex> lock(*context.plock);
				*context.pRespond = rsp;
			}
			context.pCond->notify_one();
		}
	}
}

void CWsClient::__on_pong(connection_hdl hdl, std::string msg)
{
	{
		std::lock_guard<std::mutex> lock(__m_pingLock);
		__m_pong = msg;
	}
	__m_pingCond.notify_one();
	return;
}

bool CWsClient::isOk()
{
	if (1 == __m_iStatus)
	{
		return true;
	}
	return false;
}

bool CWsClient::ping()
{
	std::string payload = "ping";
	try
	{
		std::unique_lock<std::mutex> lock(__m_pingLock);

		__m_pong.clear();

		if (__m_wsUrl.substr(0, 3) == "wss")
		{
			((WebsocketsClient*)__m_pClient)->ping(__m_hdl, payload);
		}
		else
		{
			((WebsocketClient*)__m_pClient)->ping(__m_hdl, payload);
		}

		__m_pingCond.wait_for(lock, std::chrono::milliseconds(4000));

		if (__m_pong.size())
		{
			return true;
		}
		else
		{
			__m_iStatus = -1;
			return false;
		}
	}
	catch (std::exception& e)
	{
		__m_iStatus = -1;
		LOG_ERROR("{},{}", __FUNCTION__, e.what());
	}
	return false;
}

void CWsClient::bindNotify(std::function<void(CMsgPtr)> func)
{
	__m_func = func;
}

template<class T>
void CWsClient::create()
{
	T* pClient = (T*)__m_pClient;

	pClient->clear_access_channels(websocketpp::log::alevel::all);
	pClient->set_access_channels(websocketpp::log::alevel::connect);
	pClient->set_access_channels(websocketpp::log::alevel::disconnect);
	pClient->set_access_channels(websocketpp::log::alevel::app);

	pClient->init_asio();

	pClient->set_open_handler(bind(&CWsClient::__on_open, this, _1));
	pClient->set_close_handler(bind(&CWsClient::__on_close, this, _1));
	pClient->set_fail_handler(bind(&CWsClient::__on_fail, this, _1));
	pClient->set_message_handler(bind(&CWsClient::__on_message, this, _1, _2));
	pClient->set_pong_handler(bind(&CWsClient::__on_pong,this,_1,_2));

	__m_iStatus = 0;
}

template<class T>
bool CWsClient::__connect()
{
	websocketpp::lib::error_code ec;
	T::connection_ptr conn = ((T*)__m_pClient)->get_connection(__m_wsUrl, ec);
	if (ec) {
		LOG_ERROR("get_connection 失败");
		return false;
	}
	__m_hdl = conn->get_handle();
	((T*)__m_pClient)->connect(conn);

	__m_spThread = std::make_shared<std::thread>(&T::run, ((T*)__m_pClient));

	__time64_t tic = GetTickCount64();
	while (GetTickCount64() - tic < NET_TIMEOUT) {
		if (1 == __m_iStatus || -1 == __m_iStatus) {
			break;
		}
		Sleep(50);
	}
	bool bRet = false;
	if (1 == __m_iStatus) {
		bRet = true;
	}
	return bRet;
}

template<class T>
void CWsClient::__disConnect()
{
	std::error_code ec;
	((T*)__m_pClient)->close(__m_hdl, websocketpp::close::status::normal, "", ec);
	((T*)__m_pClient)->stop();
}

template<class T>
void CWsClient::__send(std::string str)
{
	((T*)__m_pClient)->send(__m_hdl, str.c_str(), websocketpp::frame::opcode::value::text);
}

void CWsClient::realease()
{
	if (__m_spThread)
	{
		disConnect();
		__m_spThread->join();
		__m_spThread = NULL;
	}

	if (__m_pClient)
	{
		if (__m_wsUrl.substr(0, 3) == "wss")
		{
			delete (WebsocketsClient*)(__m_pClient);
		}
		else
		{
			delete (WebsocketClient*)(__m_pClient);
		}
		__m_pClient = NULL;
	}
}

void CWsClient::__on_open(connection_hdl hdl)
{
	__m_iStatus = 1;
}

void CWsClient::__on_close(connection_hdl hdl)
{
	__m_iStatus = -1;
}

void CWsClient::__on_fail(connection_hdl hdl)
{
	__m_iStatus = -1;
}