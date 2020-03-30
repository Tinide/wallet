#pragma once
#include "base\msg\Msg.h"
#include "base\common\jsoncpp\jsonRead.h"
#include "base\msg\RspMsg.h"

class CProtocol :
	public CMsg
{
public:
	CProtocol();
	virtual ~CProtocol();
	CRspMsgPtr todo(void* pParam);

	void setReqJson(JsonPtr spJson) { _m_spReqJson = spJson; }
	JsonPtr getReqJson() { return _m_spReqJson; }

	virtual bool prase() { return true; }
	virtual void deal(void* pParam) {}

	void setCmd(std::string cmd) { _m_cmd = cmd; }
	std::string getCmd() { return _m_cmd; }

	Json::Value& getRspJson() {return _m_rspJson;}

	void setEcho(bool echo) { _m_bEcho = echo; }
	bool getEcho() { return _m_bEcho; }

	void setComment(std::string comment) { _m_comment = comment; }
	std::string getComment() { return _m_comment; }
	
	void setSendtoClient(bool sendtoClient) {_m_sendtoClient = sendtoClient;}
	bool getSendtoClient() { return _m_sendtoClient; }

protected:
	bool _tokenIsValid(std::string token);
protected:
	JsonPtr _m_spReqJson;
	Json::Value _m_rspJson;
	std::string _m_cmd;
	bool _m_bEcho;
	bool _m_sendtoClient;//是否需要向客户端进行回报标志
	std::string _m_comment;
};

typedef std::shared_ptr<CProtocol> CProtocolPtr;