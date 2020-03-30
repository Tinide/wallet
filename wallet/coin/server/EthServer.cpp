#include "EthServer.h"
#include "base/common/http/HttpClient.h"
#include "base/common/markup/RWConfig.h"
#include "base/common/tool/Tool.h"
#include "base/common/proxy/Proxy.h"
#include "coin/msg/EthShiftMsg.h"
#define ETH_WALLET_KEY "Today is 2018/01/21,install Ethereum Wallet."
#define GAS "0x5208"
#define GASPRICE "0x2540BE400"

CEthServer::CEthServer()
{
}


CEthServer::~CEthServer()
{
}

bool CEthServer::_init()
{
	__m_wallet.serverAddr = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "wallet", "ethAddr");
	__m_wallet.address = "0xf0Ee8906b8D1d6151E1070dB738cD41ab57cc97f";
	__m_wallet.minConfirmations = 12;
	getAllEthAddress();
	return true;
}

void CEthServer::_run(void * param, int nPos)
{
	CMsgPtr spMsg = std::dynamic_pointer_cast<CMsg>(_msgPop(nPos));
	if (!spMsg)
	{
		return;
	}
}

void CEthServer::_maintain()
{
	int blockNum = qryNewBlockInfo();
	if (blockNum > 0)
	{
		if (__m_wallet.block == 0)
		{
			for (int i = __m_wallet.minConfirmations -1; i >= 0; i--)
			{
				getBlockByNumber(blockNum -i);
			}
		}
		else if(__m_wallet.block < blockNum)
		{
			int dit = blockNum - __m_wallet.block;
			for (int i = 1; i <= dit; i++)
			{
				getBlockByNumber(__m_wallet.block + i);
			}
		}
		__m_wallet.block = blockNum;
	}

	//发送确认次数到的充值交易
	{
		std::lock_guard<std::mutex> guard(__m_depositMsgMapMtx);
		for (auto it = __m_depositMsgMap.begin();it!=__m_depositMsgMap.end();)
		{
			if (__m_wallet.block - it->first >= __m_wallet.minConfirmations-1)
			{
				for (auto iter : it->second)
				{
					
					CProxy::sendToDepositServer(iter);

					//发送给资金转移服务
					CEthShiftMsgPtr shiftMsg = std::make_shared<CEthShiftMsg>();
					shiftMsg->setAddress(iter->getAddress());
					shiftMsg->setAmount(iter->getAmount());

					CProxy::sendToEthShiftServer(shiftMsg);
				}

				it = __m_depositMsgMap.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}

void CEthServer::getAllEthAddress()
{
	Json::Value req;
	req["method"] = "personal_listAccounts";
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			Json::Value result = JsonArr(rsp, "result");
			{
				std::lock_guard<std::mutex> guard(__m_allEthAddressMtx);
				for (int i = 0; i < (int)result.size(); i++)
				{
					__m_allEthAddress[JsonPos(result, i).asString()];
				}
			}
		}
	}
}

int CEthServer::qryNewBlockInfo()
{
	Json::Value req;
	req["method"] = "eth_blockNumber";
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			int newblocks = atoi(u256(JsonStr(rsp, "result")).str().c_str());
			LOG_INFO("ETH区块[{}]产生", newblocks);
			return newblocks;
		}
	}
	return 0;
}

void CEthServer::getBlockByNumber(int block)
{
	LOG_INFO("getBlockByNumber :{}", block);
	Json::Value req;
	req["method"] = "eth_getBlockByNumber";

	Json::Value params;
	params.append(toHex(block));
	params.append(true);

	req["params"] = params;
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			Json::Value result = JsonVal(rsp, "result");
			Json::Value transaction = JsonArr(result, "transactions");
			for (int i = 0; i < (int)transaction.size(); i++)
			{
				Json::Value tx = JsonPos(transaction, i);
				std::string to = JsonStr(tx, "to");
				
				{
					std::lock_guard<std::mutex> guard(__m_allEthAddressMtx);
					auto it = __m_allEthAddress.find(to);
					if (it == __m_allEthAddress.end())
					{
						continue;
					}
				}
				std::string value = Eth2Num((u256(JsonStr(tx, "value"))).str());
				std::string txid = JsonStr(tx, "hash");

				if (to == toLower(__m_wallet.address))
				{
					LOG_INFO("主账户资金到账：txid:{} to:{} value:{}", txid, to, value);
					continue;
				}
				else
				{
					LOG_INFO("资金到账：txid:{} to:{} value:{}", txid, to, value);
				}

				CDepositMsgPtr msg = std::make_shared<CDepositMsg>();
				msg->setCoin("ETH");
				msg->setTxid(txid);
				msg->setVout(0);
				msg->setAddress(to);
				msg->setAmount(value);
				msg->setConfirmations(__m_wallet.minConfirmations);

				{
					std::lock_guard<std::mutex> guard(__m_depositMsgMapMtx);
					__m_depositMsgMap[block].push_back(msg);
				}
			}
		}
	}
	return;
}

std::string CEthServer::newAddress(std::string coin)
{
	Json::Value req;
	req["method"] = "personal_newAccount";
	
	Json::Value params;
	params.append(ETH_WALLET_KEY);

	req["params"] = params;
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;

	bool bret = false;
	static std::mutex mtx;
	{
		//单个钱包生成地址速度很慢，加上锁。多个请求，有可能会失败。
		std::lock_guard<std::mutex> guard(mtx);
		bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	}
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			std::string address = JsonStr(rsp, "result");
			{
				std::lock_guard<std::mutex> guard(__m_allEthAddressMtx);
				__m_allEthAddress[address];
			}
			return address;
		}
	}
	return std::string();
}

bool CEthServer::validateAddress(std::string coin, std::string address)
{
	if (address.substr(0, 2) != "0x")
	{
		return false;
	}
	if (address.size() != 42)
	{
		return false;
	}
	return true;
}

bool CEthServer::sendCoin(std::string coin, std::map<std::string, double> transPack, double totalTransferAmount, std::string & spent, std::string & txid, std::string& fee, int & code, std::string & codeMsg)
{
	fee = Eth2Num((u256(GAS)*u256(GASPRICE)).str());//手续费
	if (transPack.size() != 1)
	{
		code = addr_err;
		codeMsg = "address size is not single";
		return false;
	}

	bool bret = sendTransaction(__m_wallet.address, transPack.begin()->first, toHex(transPack.begin()->second*1e18), spent, txid, code, codeMsg);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "walletpassphrase failed";
		return false;
	}
	return true;
}

bool CEthServer::walletpassphrase(std::string coin, int seconds, std::string address)
{
	Json::Value req;
	req["method"] = "personal_unlockAccount";

	Json::Value params;
	if (address == "")
	{
		req["params"].append(__m_wallet.address);
	}
	else
	{
		req["params"].append(address);
	}
	req["params"].append(ETH_WALLET_KEY);
	req["params"].append(seconds);
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			return true;
		}
	}
	return false;
}

bool CEthServer::getConfirmations(std::string coin, std::string address, Json::Value & confirmations)
{
	{
		std::lock_guard<std::mutex> guard(__m_depositMsgMapMtx);
		for (auto it : __m_depositMsgMap)
		{
			if (__m_wallet.block - it.first < __m_wallet.minConfirmations - 1)
			{
				for (auto iter : it.second)
				{
					if (iter->getAddress() == toLower(address))
					{
						Json::Value confirm;
						confirm["txid"] = iter->getTxid();
						confirm["vout"] = iter->getVout();
						confirm["address"] = address;
						confirm["amount"] = iter->getAmount();
						confirm["confirmations"] = __m_wallet.block - it.first + 1;

						confirmations.append(confirm);
					}
				}
			}
		}
	}
	return true;
}

bool CEthServer::getTransferStatus(std::string coin, std::vector<std::string> txids, std::vector<Json::Value>& transferStatus)
{
	return true;
}

bool CEthServer::signrawtransaction(std::string coin, std::string & rawTransaction_noSign, std::string & rawTransaction, std::string from)
{
	bool bret = walletpassphrase("ETH", 1, from);
	if (!bret)
	{
		return false;
	}

	Json::Value req;
	req["method"] = "eth_signTransaction";

	Json::Value params;
	Json::Reader().parse(rawTransaction_noSign, params);
	req["params"].append(params);
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			rawTransaction = JsonStr(JsonVal(rsp, "result"),"raw");
			return true;
		}
	}
	return false;
}

bool CEthServer::sendrawtransaction(std::string coin, std::string & rawTransaction, std::string & txid)
{
	Json::Value req;
	req["method"] = "eth_sendRawTransaction";

	req["params"].append(rawTransaction);
	req["id"] = random();


	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);

	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			txid = JsonStr(rsp, "result");
			return true;
		}
	}
	return false;
}

bool CEthServer::getTotalBalance(std::string coin, std::string & totalBalance)
{
	Json::Value req;
	req["method"] = "eth_getBalance";
	req["params"].append(__m_wallet.address);
	req["params"].append("latest");
	req["id"] = random();

	CHttpClient hc;
	Json::Value rsp;
	bool bret = hc.post(__m_wallet.serverAddr, req, rsp, false);
	if (bret)
	{
		if (JsonVal(rsp, "error") == JSON_NULL)
		{
			totalBalance = Eth2Num(JsonStr(rsp, "result"));
			return true;
		}
	}

	return false;
}

bool CEthServer::sendTransaction(const std::string & from, const std::string & to, std::string & value, std::string & spent, std::string & txid, int & code, std::string & codeMsg)
{
	Json::Value params;
	params["from"] = from;
	params["to"] = to;
	params["gas"] = GAS;
	params["gasPrice"] = GASPRICE;//10GWei --0x2540BE400    //20GWei --- 0x4a817c800
	params["value"] = value;

	spent = Json::FastWriter().write(params);

	std::string rawTransaction;
	bool bret = signrawtransaction("", spent, rawTransaction,from);
	//bool bret = signFromColdWallet("ETH", spent, from, rawTransaction);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "signrawtransaction failed";
		return false;
	}

	bret = sendrawtransaction("", rawTransaction, txid);
	if (!bret)
	{
		code = withdraw_err;
		codeMsg = "sendrawtransaction failed";
		return false;
	}
	return true;
}

bool CEthServer::shiftEth(std::string address, std::string amount)
{
	std::string spent;
	std::string txid;
	int code = 0;
	std::string codeMsg;
	std::string sendValue = (u256(Num2Eth(amount)) - u256(GAS)*u256(GASPRICE)).str();
	bool bret = sendTransaction(address, __m_wallet.address, toHex(sendValue), spent, txid, code, codeMsg);
	if (bret)
	{
		LOG_INFO("txid:{}.资金转移[from:{},to:{},amount:{}]", txid, address,__m_wallet.address,Eth2Num(sendValue));
	}
	return bret;
}

std::string CEthServer::Eth2Num(std::string ethNum)
{
	u256 u(ethNum);
	if (u == 0)
	{
		return "0";
	}
	std::string ethNumStr = u.str();
	if (ethNumStr.size() > 18)
	{
		int n = (int)ethNumStr.size();
		std::string first = ethNumStr.substr(0, n - 18);
		std::string second = ethNumStr.substr(n - 18);
		ethNumStr = first + "." + second;
	}
	else
	{
		while (ethNumStr.size() < 18)
		{
			ethNumStr = "0" + ethNumStr;
		}
		ethNumStr = "0." + ethNumStr;
	}

	while (1)
	{
		if (ethNumStr.back() == '0')
		{
			ethNumStr = ethNumStr.substr(0, ethNumStr.size() - 1);
		}
		else
		{
			break;
		}
	}

	int pos = (int)ethNumStr.find(".");
	std::string first = ethNumStr.substr(0, pos+1);
	std::string second = ethNumStr.substr(pos + 1);

	while (second.size() < 8)
	{
		second += "0";
	}

	return first+second;
}

std::string CEthServer::Num2Eth(std::string num)
{
	if (num == "0" || num.size() == 0)
	{
		return "0";
	}
	std::string ethNum;
	int pos = (int)num.find(".");
	if (pos == -1)
	{
		ethNum = num + "000000000000000000";
	}
	else
	{
		std::string first = num.substr(0, pos);
		std::string second = num.substr(pos + 1);

		while (second.size() < 18)
		{
			second += "0";
		}
		ethNum = first + second;
	}
	while (1)
	{
		if (ethNum[0] == '0')
		{
			ethNum = ethNum.substr(1);
		}
		else
		{
			break;
		}
	}
	return ethNum;
}