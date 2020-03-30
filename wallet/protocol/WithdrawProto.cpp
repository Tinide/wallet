#include "WithdrawProto.h"
#include "base/common/tool/Tool.h"
#include "coin/server/BtcServer.h"
#include "base/server/SqlServer.h"
CWithdrawProto::CWithdrawProto():__m_totalTransferAmount(0.0)
{
}


CWithdrawProto::~CWithdrawProto()
{
}

bool CWithdrawProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	__m_coin = JsonStr(data, "coin");
	Json::Value pack = JsonArr(data, "pack");
	for (int i = 0; i < (int)pack.size(); i++)
	{
		Json::Value transfer = JsonPos(pack, i);
		std::string addr = JsonStr(transfer, "address");
		double amount = stod(JsonStr(transfer, "amount"));

		__m_transPack[addr] = amount;
		__m_totalTransferAmount += amount;
	}
	return true;
}

void CWithdrawProto::deal(void * pParam)
{
	if (!verifyToken(__m_token))
	{
		_m_code = token_err;
		_m_msg = "token is invalid";
		return;
	}

	if (__m_transPack.size() == 0)
	{
		_m_code = addr_err;
		_m_msg = "address size is zero";
		return;
	}

	for (auto it : __m_transPack)
	{
		if (it.second <= 1e-8)
		{
			_m_code = withdraw_err;
			_m_msg = "amount is error,too little";
			return;
		}
	}

	if (!verifyCoinAddr())
	{
		_m_code = addr_err;
		_m_msg = string_format("%s address is invalid", __m_coin.c_str());
		return;
	}
	std::string spent;
	std::string txid;
	std::string fee;

	bool bret;
	static std::mutex mtx;
	{
		std::lock_guard<std::mutex> guard(mtx);
		bret = CoinServer(__m_coin)->sendCoin(__m_coin, __m_transPack, __m_totalTransferAmount, spent, txid,fee, _m_code, _m_msg);
	}
	if (!bret)
	{
		return;
	}

	//将交易记录写入db
	putIntoDb(__m_coin,spent,txid);

	_m_rspJson["coin"] = __m_coin;
	_m_rspJson["txid"] = txid;
	_m_rspJson["fee"] = fee;
}

bool CWithdrawProto::verifyCoinAddr()
{
	bool bret;
	for (auto it : __m_transPack)
	{
		bret = CoinServer(__m_coin)->validateAddress(__m_coin,it.first);
		if (!bret)
		{
			return false;
		}
	}
	return true;
}

void CWithdrawProto::putIntoDb(std::string coin, std::string& spent, std::string& txid)
{
	std::string sql = string_format("insert into t_withdraw(coin,uuid,create_time,txid,spent) values ('%s','%s',now(),'%s','%s')",coin.c_str(), _m_uuid.c_str(),txid.c_str(),spent.c_str());
	DataHelper helper = Instance(CSqlServer)->getConn();
	if (helper)
	{
		try
		{
			helper->execute(sql);
			helper->commit();
		}
		catch (std::exception& e)
		{
			LOG_ERROR("sql:{},err:{}", sql, e.what());
			helper->rollback();
		}
	}
}
