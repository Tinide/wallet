#include "BtcServer.h"
#include "base/common/http/HttpClient.h"
#include "base/common/markup/RWConfig.h"
#include "proxy\Proxy.h"
#include "coin/msg/DepositMsg.h"
#include "base/common/tool/Tool.h"
CBtcServer::CBtcServer()
{

}


CBtcServer::~CBtcServer()
{
}

bool CBtcServer::_init()
{
	_m_maintainLoopTime = 10 * 1000;
	
	__m_wallet["BTC"].serverAddr = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "btcAddr");
	//__m_wallet["BTC"].pwd = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "btcPwd");
	__m_wallet["BTC"].pwd = "Today is 2018/01/05,install Bitcoin.";
	__m_wallet["BTC"].address = "1DDpZPDyesugqfmRRGmhkZsEzZQfxK51HC";
	__m_wallet["BTC"].fee = 0.0002;
	__m_wallet["BTC"].minConfirmations = 1;

	__m_wallet["LTC"].serverAddr = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "ltcAddr");
	//__m_wallet["LTC"].pwd = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "ltcPwd");
	__m_wallet["LTC"].pwd = "Today is 2018-01-05,install Litecoin.";
	__m_wallet["LTC"].address = "LRPqFcGEAvSq6CqexxfX7erkKU6GpL9TTU";
	__m_wallet["LTC"].fee = 0.0005;
	__m_wallet["LTC"].minConfirmations = 6;

	__m_wallet["BCH"].serverAddr = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "bchAddr");
	//__m_wallet["BCH"].pwd = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "bchPwd");
	__m_wallet["BCH"].pwd = "Today is 2018-01-05,install Bitcoin ABC.";
	__m_wallet["BCH"].address = "15Qq1Mhp5mM7pfmEwdzYRnuSWSTQBSbUcs";
	__m_wallet["BCH"].fee = 0.00005;
	__m_wallet["BCH"].minConfirmations = 2;

	return true;
}

void CBtcServer::_run(void * param, int nPos)
{
	CMsgPtr spMsg = std::dynamic_pointer_cast<CMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
}

void CBtcServer::_maintain()
{
	for (auto it : __m_wallet)
	{
		if (qryNewBlockInfo(it.first))
		{
			Json::Value req;
			req["method"] = "listunspent";

			req["params"].append(it.second.minConfirmations);
			req["params"].append(it.second.minConfirmations+2);//查询确认次数=1的交易
		
			CHttpClient hc;
			Json::Value rsp;
			bool bret = hc.post(__m_wallet[it.first].serverAddr, req, rsp, true);
			if (bret)
			{
				if (JsonVal(rsp, "error") == JSON_NULL)
				{
					Json::Value result = JsonArr(rsp, "result");
					for (int i = 0; i < (int)result.size(); i++)
					{
						Json::Value utxo = JsonPos(result, i);
						LOG_INFO(Json::FastWriter().write(utxo));

						std::string btcAddr = JsonStr(utxo, "address");

						if (btcAddr == __m_wallet[it.first].address)
						{
							continue;
						}
						//推送充值信息
						CDepositMsgPtr msg = std::make_shared<CDepositMsg>();
						msg->setCoin(it.first);
						msg->setTxid(JsonStr(utxo, "txid"));
						msg->setVout(JsonInt(utxo, "vout"));
						msg->setAddress(JsonStr(utxo, "address"));
						msg->setAmount(d2Str_8(JsonDouble(utxo, "amount")));
						msg->setConfirmations(JsonInt(utxo, "confirmations"));
						CProxy::sendToDepositServer(msg);
					}
				}
			}
		}
	}
 }

bool CBtcServer::qryNewBlockInfo(std::string coin)
{
	Json::Value req;
	req["method"] = "getblockchaininfo";

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			int newblocks = JsonInt(JsonVal(rsp, "result"), "blocks");
			if (newblocks > __m_wallet[coin].block)
			{
				LOG_INFO("{}区块[{}]产生",coin,newblocks);
				__m_wallet[coin].block = newblocks;
				return true;
			}
		}
	}
	return false;
}

std::string CBtcServer::newAddress(std::string coin)
{
	Json::Value req;
	req["method"] = "getnewaddress";

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr,req,rsp,true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			return JsonStr(rsp, "result");
		}
	}
	return std::string();
}

bool CBtcServer::validateAddress(std::string coin,std::string btcAddr)
{
	Json::Value req;
	req["method"] = "validateaddress";
	req["params"].append(btcAddr);

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			return JsonBool(JsonVal(rsp, "result"),"isvalid");
		}
	}
	return false;
}

bool CBtcServer::sendCoin(std::string coin, std::map<std::string, double> transPack, double totalTransferAmount, std::string & spent, std::string& txid, std::string& fee,int& code,std::string& codeMsg)
{
	std::vector<UNSPENT> unspents;
	bool bret = getUnspent(coin, totalTransferAmount, unspents);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = string_format("%s amount in wallet is not enough", coin.c_str());
		return false;
	}

	std::string rawTransaction_nosign;
	bret = createrawtransaction(coin, unspents, transPack, rawTransaction_nosign, spent);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "createrawtransaction failed";
		return false;
	}
	fee = d2Str_8(__m_wallet[coin].fee*transPack.size());//手续费
	std::string rawTransaction;
	//bret = signFromColdWallet(coin, rawTransaction_nosign, "", rawTransaction);
	bret = signrawtransaction(coin, rawTransaction_nosign, rawTransaction);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "signrawtransaction failed";
		return false;
	}

	bret = sendrawtransaction(coin, rawTransaction, txid);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "sendrawtransaction failed";
		return false;
	}
	return bret;
}

bool CBtcServer::getUnspent(std::string coin,double amount,std::vector<UNSPENT>& vUnspents)
{
	double total_amount = 0;
	Json::Value req;
	req["method"] = "listunspent";
	req["params"].append(__m_wallet[coin].minConfirmations);
	req["params"].append(9999999);//确认次数>=min次的未花费

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			Json::Value result = JsonArr(rsp, "result");
			for (int i = 0; i < (int)result.size(); i++)
			{
				Json::Value value = JsonPos(result, i);
				bool spendable = JsonBool(value, "spendable");
				if (!spendable)
				{
					continue;
				}

				UNSPENT unspent;
				unspent.txid = JsonStr(value, "txid");
				unspent.vout = JsonInt(value, "vout");
				unspent.amount = JsonDouble(value, "amount");
				unspent.confirmations = JsonInt(value, "confirmations");

				total_amount += unspent.amount;

				vUnspents.push_back(unspent);
				if (total_amount > amount)//最大旷工费
				{
					return true;
				}
			}
			LOG_ERROR("%s amount in wallet is not enough",coin.c_str());
		}
	}
	return false;
}

bool CBtcServer::createrawtransaction(std::string coin,std::vector<UNSPENT>& vUnspents, std::map<std::string, double>& vTransferPack,std::string& rawTransaction_noSign, std::string& spent)
{
	Json::Value unspents;
	double total_unspent_amount = 0.0;
	for (auto it : vUnspents)
	{
		Json::Value unspent;
		unspent["txid"] = it.txid;
		unspent["vout"] = it.vout;
		unspents.append(unspent);

		total_unspent_amount += it.amount;
	}

	double total_transfer_amount = 0.0;
	Json::Value transfer;
	for (auto itor : vTransferPack)
	{
		transfer[itor.first] = d2Str_8(itor.second);
		total_transfer_amount += itor.second;
	}

	//找零
	double change_amount = total_unspent_amount - total_transfer_amount - __m_wallet[coin].fee*vTransferPack.size();//旷工费

	auto e = vTransferPack.find(__m_wallet[coin].address);
	if (e != vTransferPack.end())
	{
		transfer[__m_wallet[coin].address] = transfer[__m_wallet[coin].address].asDouble() + change_amount;
	}
	else
	{
		transfer[__m_wallet[coin].address] = d2Str_8(change_amount);
	}

	Json::Value req;
	Json::Value params;
	params.append(unspents);
	params.append(transfer);

	req["method"] = "createrawtransaction";
	req["params"] = params;

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			spent = Json::FastWriter().write(req);
			LOG_INFO(spent);
			rawTransaction_noSign = JsonStr(rsp, "result");
			return true;
		}
	}
	return false;
}

bool CBtcServer::walletpassphrase(std::string coin,int seconds, std::string address)
{
	Json::Value req;
	req["method"] = "walletpassphrase";
	req["params"].append(__m_wallet[coin].pwd);
	req["params"].append(seconds);

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			return true;
		}
	}
	return false;
}

bool CBtcServer::signrawtransaction(std::string coin,std::string & rawTransaction_noSign, std::string & rawTransaction, std::string from)
{
	bool bret = walletpassphrase(coin);
	if (!bret)
	{
		return false;
	}
	Json::Value req;
	req["method"] = "signrawtransaction";
	req["params"].append(rawTransaction_noSign);

	CHttpClient hc;
	Json::Value rsp;
	bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			if (JsonVal(JsonVal(rsp, "result"), "errors") == JSON_NULL)
			{
				rawTransaction = JsonStr(JsonVal(rsp, "result"), "hex");
				return true;
			}
		}
	}
	return false;
}

bool CBtcServer::sendrawtransaction(std::string coin, std::string & rawTransaction, std::string& txid)
{
	Json::Value req;
	req["method"] = "sendrawtransaction";
	req["params"].append(rawTransaction);

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		LOG_INFO(Json::FastWriter().write(rsp));
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			txid = JsonStr(rsp, "result");
			return true;
		}
	}
	return false;
}

bool CBtcServer::getConfirmations(std::string coin, std::string address, Json::Value& confirmations)
{
	Json::Value req;
	req["method"] = "listunspent";
	req["params"].append(0);
	req["params"].append(__m_wallet[coin].minConfirmations-1);//查询确认次数=min-1的交易
	
	Json::Value add;
	add.append(address);
	req["params"].append(add);

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			Json::Value result = JsonArr(rsp, "result");
			for (int i = 0; i < (int)result.size(); i++)
			{
				Json::Value utxo = JsonPos(result, i);

				Json::Value confirm;
				confirm["txid"] = JsonStr(utxo, "txid");
				confirm["vout"] = JsonInt(utxo, "vout");
				confirm["address"] = JsonStr(utxo, "address");
				confirm["amount"] = d2Str_8(JsonDouble(utxo, "amount"));
				confirm["confirmations"] = JsonInt(utxo, "confirmations");

				confirmations.append(confirm);
			}
			return true;
		}
	}
	return false;
}

bool CBtcServer::getTransferStatus(std::string coin,std::vector<std::string> txids, std::vector<Json::Value>& transferStatus)
{
	CHttpClient hc;
	Json::Value rsp;

	for (auto it : txids)
	{
		Json::Value req;
		req["method"] = "gettransaction";
		req["params"].append(it);

		bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
		if (bret && JsonVal(rsp, "error") == JSON_NULL)
		{
			Json::Value result = JsonVal(rsp, "result");
			Json::Value status;
			status["confirmations"] = JsonInt(result, "confirmations");
			status["txid"] = JsonStr(result, "txid");

			transferStatus.push_back(status);
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool CBtcServer::getTotalBalance(std::string coin, std::string & totalBalance)
{
	Json::Value req;
	req["method"] = "getwalletinfo";

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet[coin].serverAddr, req, rsp, true);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			totalBalance = d2Str_8(JsonDouble(JsonVal(rsp,"result"),"balance"));
			return true;
		}
	}
	return false;
}
