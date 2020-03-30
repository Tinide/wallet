#include "DepositRecordProto.h"
#include "base/server/SqlServer.h"
#include "base/common/tool/Tool.h"

CDepositRecordProto::CDepositRecordProto()
{
}


CDepositRecordProto::~CDepositRecordProto()
{
}

bool CDepositRecordProto::prase()
{
	Json::Value data = JsonVal(*_m_spReqJson, "data");
	__m_token = JsonStr(data, "token");
	Json::Value arr = JsonArr(data, "record");
	__m_begin = JsonIntEx(JsonPos(arr, 0));
	__m_end = JsonIntEx(JsonPos(arr, 1));
	return true;
}

void CDepositRecordProto::deal(void * pParam)
{
	if (!verifyToken(__m_token))
	{
		_m_code = token_err;
		_m_msg = "token is invalid";
		return;
	}

	std::string sql = string_format("select * from t_deposit where id between %d and %d",__m_begin,__m_end);
	DataHelper helper = Instance(CSqlServer)->getConn();
	if (!helper)
	{
		_m_code = qry_err;
		_m_msg = "system error!";
		return;
	}

	try
	{
		ResultSet rs = helper->query(sql);

		for (int i = 0; i < rs.size(); i++)
		{
			Json::Value deposit;
			deposit["record"] = atoi(rs[i]["id"].c_str());
			deposit["txid"] = rs[i]["txid"];
			deposit["vout"] = atoi(rs[i]["vout"].c_str());
			deposit["address"] = rs[i]["address"];
			deposit["amount"] = rs[i]["amount"];
			deposit["coin"] = rs[i]["coin"];
			deposit["create_time"] = rs[i]["create_time"];
			_m_rspJson["deposit"].append(deposit);
		}
		helper->commit();
	}
	catch (std::exception& e)
	{
		LOG_ERROR("sql:{},err:{}", sql, e.what());
		helper->rollback();

		_m_code = qry_err;
		_m_msg = "system error!";
		return;
	}
}
