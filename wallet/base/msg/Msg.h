#pragma once
#include <string>
#include <memory>
#include "base/common/jsoncpp/jsonRead.h"
typedef std::weak_ptr<void> websocket_hdl;
typedef std::shared_ptr<void> websocket_spHdl;
//错误码定义
enum msg_errcode
{
	ok = 0,
	prase_err,
	protocol_err,
	login_err,
	open_err,
	close_err,
	queryOrder_err,
	chart_err,
	db_err,
	instrument_err,
	token_err,
	mt4_conn_err,
	deposit_err,
	forceClose_err,
	plan_err,
	conn_err,
	qry_err,
	newAddr_err,
	addr_err,
	withdraw_err,
	coldWallet_err
};


class CMsg
{
public:
	CMsg();
	virtual ~CMsg();
public:
	virtual std::string getMsg() {return _m_msg;}
	virtual int getErrCode() { return _m_code; }
	void setErrCode(int errCode) { _m_code = errCode; }
	void setMsg(std::string str) { _m_msg = str; }

	void setWebsocketHdl(websocket_hdl hdl) { _m_hdl = hdl; }
	websocket_hdl getWebsocketHdl() { return _m_hdl; }

	void setUuid(std::string uuid) { _m_uuid = uuid; }
	std::string getUuid() { return _m_uuid; }

	virtual void putIntoDb() {}

	void setLogPrint(bool bLogPrint) { _m_logPrint = bLogPrint; }
	bool getLogPrint() { return _m_logPrint; }
protected:
	std::string _m_msg;
	int _m_code;//消息错误码
	std::string _m_uuid;
	websocket_hdl _m_hdl;
	bool _m_logPrint;
};

typedef std::shared_ptr<CMsg> CMsgPtr;