#pragma once
#include "Server.h"
#include "../mysql/include/mysql_driver.h"
#include "../mysql/include/mysql_connection.h"
#include "../mysql/include/cppconn/driver.h"
#include "../mysql/include/cppconn/statement.h"
#include "../mysql/include/cppconn/prepared_statement.h"
#include "../mysql/include/cppconn/metadata.h"
#include "../mysql/include/cppconn/exception.h"

typedef std::shared_ptr<sql::Connection> conn_sptr;

class CSqlServer;
class CDataHelper;
class ResultSet;
class CRowData
{
	friend ResultSet;
public:
	std::string operator[](std::string field);
private:
	CRowData(std::shared_ptr<sql::ResultSet> sqlRs) :_m_sqlRs(sqlRs) {}
private:
	std::shared_ptr<sql::ResultSet> _m_sqlRs;
};

class ResultSet
{
public:
	ResultSet() {}
	ResultSet(std::shared_ptr<sql::ResultSet> sqlRs) :_m_sqlRs(sqlRs){}
	CRowData operator[](int pos);
	int size();
private:
	std::shared_ptr<sql::ResultSet> _m_sqlRs;
};
class CDataHelper
{
	friend CSqlServer;
public:
	CDataHelper() {}
public:
	int execute(std::string sql, bool bIntoleveldb = false);//需要手动提交
	ResultSet query(std::string sql);
	bool commit();
	bool rollback();
private:
	void set(conn_sptr conn) { __m_conn = conn; }
	int delConn();//删除了连接，则返回1.没有删除，则返回0；
	bool writeToLeveldb(std::string sql);
private:
	conn_sptr __m_conn;
};

typedef std::shared_ptr<CDataHelper> DataHelper;

class CSqlServer :
	public CServer
{
	friend CServer;
protected:
	CSqlServer();
	virtual ~CSqlServer();
protected:
	virtual bool _init();
	virtual void _run(void* param, int nPos);
	virtual void _pre_end();
	virtual void _end();
public:
	DataHelper getConn();
	//连接损坏以后，调用者将其删除
	void del(conn_sptr conn);
private:
	DataHelper getConn(int i);
private:
	std::string __m_dbAddr;
	std::string __m_dbAccount;
	std::string __m_dbPwd;

	sql::mysql::MySQL_Driver* __m_driver;

	CPool<sql::Connection> __m_connPool;

	int __m_nloop;
};
