#pragma once
#pragma warning( disable:4503 )
#include <set>
#include "websocketpp/server.hpp"
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/common/thread.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"
#include "websocketpp\config/asio.hpp"
#include "websocketpp/config/asio_client.hpp"

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;

struct testee_config : public websocketpp::config::asio {
	// pull default settings from our core config
	typedef websocketpp::config::asio core;

	typedef core::concurrency_type concurrency_type;
	typedef core::request_type request_type;
	typedef core::response_type response_type;
	typedef core::message_type message_type;
	typedef core::con_msg_manager_type con_msg_manager_type;
	typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

	typedef core::alog_type alog_type;
	typedef core::elog_type elog_type;
	typedef core::rng_type rng_type;
	typedef core::endpoint_base endpoint_base;

	static bool const enable_multithreading = true;

	struct transport_config : public core::transport_config {
		typedef core::concurrency_type concurrency_type;
		typedef core::elog_type elog_type;
		typedef core::alog_type alog_type;
		typedef core::request_type request_type;
		typedef core::response_type response_type;

		static bool const enable_multithreading = true;
	};

	typedef websocketpp::transport::asio::endpoint<transport_config>
		transport_type;

	static const websocketpp::log::level elog_level =
		websocketpp::log::elevel::none;
	static const websocketpp::log::level alog_level =
		websocketpp::log::alevel::none;

	/// permessage_compress extension
	struct permessage_deflate_config {};
};

typedef websocketpp::server<testee_config> websocketppserver;
typedef websocketppserver::message_ptr message_ptr;
typedef websocketpp::server<testee_config>::connection_ptr conn_ptr;
typedef websocketpp::client<websocketpp::config::asio_client> WebsocketClient;
typedef websocketpp::client<websocketpp::config::asio_tls_client> WebsocketsClient;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
class CWebSocket
{
public:
	CWebSocket();
	virtual ~CWebSocket();
};

