#include "ProtocolHdl.h"
#include "base/common/markup/Markup.h"
#include "log.h"
#include "protocol\LoginProto.h"
#include "protocol\NewAddressProto.h"
#include "protocol\WithdrawProto.h"
#include "protocol\DepositRecordProto.h"
#include "protocol\QryDepositProto.h"
#include "protocol\QryWithdrawProto.h"
#include "protocol\VerifyAddressProto.h"
#include "protocol\ColdWalletProto.h"
#include "protocol\balance\qryTotalBalanceProto.h"
CProtocolHdl::CProtocolHdl(CReqMsgPtr spReqMsg):__m_spReqMsg(spReqMsg)
{
}


CProtocolHdl::~CProtocolHdl()
{
}

CProtocolPtr CProtocolHdl::handle()
{
	JsonPtr spJson = std::make_shared<Json::Value>();
	Json::Reader reader;
	CProtocolPtr spProto;
	std::string msg = CMarkup::UTF8ToA(__m_spReqMsg->getMsg());
	LOG_INFO("[{}]|{}:{}", __m_spReqMsg->getWebsocketHdl().lock().get(), __m_spReqMsg->getUuid(), msg);
	if (!reader.parse(msg, *spJson))
	{
		spProto = std::make_shared<CProtocol>();
		spProto->setErrCode(msg_errcode::prase_err);
		spProto->setMsg(reader.getFormattedErrorMessages());
	}
	else
	{
		spProto = buildProtocol(spJson);
		if (spProto)
		{
			spProto->prase();
		}
	}
	spProto->setUuid(__m_spReqMsg->getUuid());
	spProto->setWebsocketHdl(__m_spReqMsg->getWebsocketHdl());
	return spProto;
}

CProtocolPtr CProtocolHdl::buildProtocol(JsonPtr spJson)
{
	CProtocolPtr spProto;
	std::string cmd = JsonStr(*spJson, "cmd");
	if ("login" == cmd)
	{
		spProto = std::make_shared<CLoginProto>();
	}
	else if ("verifyAddress" == cmd)
	{
		spProto = std::make_shared<CVerifyAddressProto>();
	}
	else if("newAddress" == cmd)
	{
		spProto = std::make_shared<CNewAddressProto>();
	}
	else if ("withdraw" == cmd)
	{
		spProto = std::make_shared<CWithdrawProto>();
	}
	else if ("depositRecord" == cmd)
	{
		spProto = std::make_shared<CDepositRecordProto>();
	}
	else if ("qryDeposit" == cmd)
	{
		spProto = std::make_shared<CQryDepositProto>();
	}
	else if ("qryWithdraw" == cmd)
	{
		spProto = std::make_shared<CQryWithdrawProto>();
	}
	else if ("coldWallet" == cmd)
	{
		spProto = std::make_shared<CColdWalletProto>();
	}
	else if ("balance/qryTotalBalance" == cmd)
	{
		spProto = std::make_shared<CQryTotalBalanceProto>();
	}
	else
	{
		spProto = std::make_shared<CProtocol>();
		spProto->setErrCode(msg_errcode::protocol_err);
		std::string errMsg = "Ð­Òé´íÎó" + cmd;
		spProto->setMsg(errMsg);
	}
	spProto->setReqJson(spJson);
	spProto->setCmd(cmd);
	spProto->setEcho(JsonBool(*spJson, "echo"));
	spProto->setComment(JsonStr(*spJson, "comment"));
	return spProto;
}
