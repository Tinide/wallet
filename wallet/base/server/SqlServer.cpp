#include "SqlServer.h"
#include <algorithm>
#include "base/common/log/log.h"
#include "base/common/markup/RWConfig.h"
#include "base/common/leveldb/Leveldb.h"
#include "base/common/tool/Tool.h"
#define SQL_CONN_NUM 4

int CDataHelper::execute(std::string sql, bool bIntoleveldb)
{
	int iRet = -1;
	int sql_errcode = 0;
	if (!__m_conn)
	{
		throw std::exception("-1");
	}

	sql::Statement* stmt = NULL;
	try
	{
		stmt = __m_conn->createStatement();
		iRet = stmt->executeUpdate(sql.c_str());
	}
	catch (sql::SQLException& e)
	{
		sql_errcode = e.getErrorCode();
		LOG_ERROR("{}:{}", sql_errcode, e.what());
		if (delConn() && bIntoleveldb)//链接中断，则需要将sql先存入leveldb
		{
			writeToLeveldb(sql);
		}
		iRet = -1;
	}
	
	if (stmt)
	{
		delete stmt;
		stmt = NULL;
	}
	if (-1 == iRet)
	{
		throw std::exception(std::to_string(sql_errcode).c_str());
	}
	return iRet;
}


ResultSet CDataHelper::query(std::string sql)
{
	int iRet = -1;
	if (!__m_conn)
	{
		throw std::exception("conn failed");
	}
	sql::Statement* stmt = NULL;
	sql::ResultSet* sqlRs = NULL;
	try
	{
		stmt = __m_conn->createStatement();
		if (!stmt)
		{
			throw std::exception("stmt create failed");
		}
		sqlRs = stmt->executeQuery(sql.c_str());
		if (!sqlRs)
		{
			delete stmt; 
			stmt = NULL;
			throw std::exception("sql::ResultSet executeQuery failed");
		}
		else
		{
			iRet = 0;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
		delConn();
		iRet = -1;
	}
	if (stmt)
	{
		delete stmt;
		stmt = NULL;
	}
	if (-1 == iRet)
	{
		throw std::exception("query failed");
	}

	return ResultSet(std::shared_ptr<sql::ResultSet>(sqlRs));
}

bool CDataHelper::commit()
{
	if (!__m_conn)
	{
		throw std::exception("conn failed");
	}
	try
	{
		__m_conn->commit();
	}
	catch (sql::SQLException& e)
	{
		LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
		delConn();
		throw std::exception(e.what());
	}
	return true;
}

bool CDataHelper::rollback()
{
	if (!__m_conn)
	{
		throw std::exception("conn failed");
	}
	try
	{
		__m_conn->rollback();
	}
	catch (sql::SQLException& e)
	{
		LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
		delConn();
		return false;
	}
	return true;
}

int CDataHelper::delConn()
{
	if (!__m_conn->isValid())
	{
		try
		{
			__m_conn->close();
		}
		catch (sql::SQLException& e)
		{
			LOG_ERROR("{}", e.what());
		}
		CServer::instance<CSqlServer>()->del(__m_conn);
		return 1;
	}
	return 0;
}

bool CDataHelper::writeToLeveldb(std::string sql)
{
	CLeveldb db;
	std::string key = "sql |" + getUuid();
	bool bret = db.put(key, sql);
	if (!bret)
	{
		LOG_ERROR("writeToLeveldb failed");
	}
	else
	{
		LOG_INFO("writeToLeveldb[sql |{}]:{}", key, sql);
	}
	return bret;
}




CSqlServer::CSqlServer()
{
	__m_dbAddr = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "db", "ip");
	__m_dbAccount = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "db", "account");
	__m_dbPwd = READ_CFG(SZAPPNAME + std::string(".xml"), "CONFIG", "db", "pwd");
}


CSqlServer::~CSqlServer()
{
}

bool CSqlServer::_init()
{
	try
	{
		__m_driver = sql::mysql::get_mysql_driver_instance();
		for (int i = 0; i < SQL_CONN_NUM; i++)
		{
			conn_sptr conn(__m_driver->connect(__m_dbAddr.c_str(), __m_dbAccount.c_str(), __m_dbPwd.c_str()));
			conn->setAutoCommit(false);

			sql::Statement* stmt = conn->createStatement();
			stmt->execute("set names gbk");

			__m_connPool.add(conn);
			delete stmt;
		}
	}
	catch (sql::SQLException& e)
	{
		LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
		return false;
	}
	LOG_INFO("数据库{}连接成功", __m_dbAddr);
	return true;
}

//用于维护数据库连接池
void CSqlServer::_run(void * param, int nPos)
{
	Sleep(1000);
	__m_nloop++;

	if (__m_connPool.size() != SQL_CONN_NUM)
	{
		try
		{
			conn_sptr conn(__m_driver->connect(__m_dbAddr.c_str(), __m_dbAccount.c_str(), __m_dbPwd.c_str()));
			conn->setAutoCommit(false);
			sql::Statement* stmt = conn->createStatement();
			stmt->execute("set names gbk");
			__m_connPool.add(conn);
			delete stmt;
			LOG_INFO("新增1个数据库连接");
		}
		catch (sql::SQLException& e)
		{
			LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
		}
	}

	if (__m_nloop % 600 == 0)//每600s访问一次数据库，防止链接断开。mysql默认8小时断开一次
	{
		for (int i = 0; i < SQL_CONN_NUM; i++)
		{
			DataHelper helper;
			try
			{
				helper = getConn(i);
				if (helper)
				{
					helper->query("select now()");
					helper->commit();
				}
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("{}:{}", __FUNCTION__, e.what());
				helper->rollback();
			}
		}
	}

	if (__m_nloop % 5 == 0)//每过5s钟，读取一下leveldb
	{
		CLeveldb db;
		std::pair<std::string, std::string> pair;
		if (db.getOneItem(pair, "sql |"))
		{
			std::string sql = pair.second;
			DataHelper helper;
			try
			{
				helper = getConn();
				if (helper)
				{
					helper->execute(sql);
					helper->commit();
					db.del(pair.first);
					LOG_INFO("upload to mysql :{}", sql);
				}
			}
			catch (const std::exception&)
			{
				LOG_ERROR("leveldb's sql put into db failed again");
				helper->rollback();
			}
		}
	}
}

void CSqlServer::_pre_end()
{
}

void CSqlServer::_end()
{
	try
	{
		while(__m_connPool.size())
		{
			conn_sptr conn = __m_connPool.get();
			if (conn)
			{
				conn->close();
				__m_connPool.del(conn);
			}
			else
			{
				break;
			}
		}
	}
	catch (sql::SQLException& e)
	{
		LOG_ERROR("{}:{}", e.getErrorCode(), e.what());
	}
}

DataHelper CSqlServer::getConn()
{
	conn_sptr conn = __m_connPool.get();
	if (conn)
	{
		DataHelper connHdl = std::make_shared<CDataHelper>();
		connHdl->set(conn);
		return connHdl;
	}
	else
	{
		LOG_ERROR("DataHelper getconn failed");
		return DataHelper();
	}
}

DataHelper CSqlServer::getConn(int i)
{
	conn_sptr conn = __m_connPool.get(i);
	if (conn)
	{
		DataHelper connHdl = std::make_shared<CDataHelper>();
		connHdl->set(conn);
		return connHdl;
	}
	else
	{
		return DataHelper();
	}
}

void CSqlServer::del(conn_sptr conn)
{
	__m_connPool.del(conn);
}


CRowData ResultSet::operator[](int pos)
{
	int n = int(pos - _m_sqlRs->getRow() + 1);
	if (n > 0)
	{
		while (n--)
		{
			_m_sqlRs->next();
		}
	}
	else
	{
		while (n++)
		{
			_m_sqlRs->previous();
		}
	}
	return CRowData(_m_sqlRs);
}

int ResultSet::size()
{
	if (_m_sqlRs)
	{
		return (int)_m_sqlRs->rowsCount();
	}
	else
	{
		return 0;
	}
}

std::string CRowData::operator[](std::string field)
{
	return _m_sqlRs->getString(field.c_str()).c_str();
}
