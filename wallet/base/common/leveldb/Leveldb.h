#pragma once
#include <string>
#include "db.h"
#include <map>
class CLeveldb
{
public:
	CLeveldb();
	virtual ~CLeveldb();
public:
	static bool init();
	static void unInit();
public:
	bool put(std::string key, std::string val);
	bool get(std::string key,std::string& val);
	bool del(std::string key);
	bool getOneItem(std::pair<std::string,std::string>& pair,std::string subString = "");

	bool getAllItem(std::map<std::string, std::string>& mItems, std::string subString = "");
private:
	static leveldb::DB* __m_db;
};