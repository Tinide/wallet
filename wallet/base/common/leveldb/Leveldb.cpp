#include "Leveldb.h"
#include "log.h"
leveldb::DB* CLeveldb::__m_db;
CLeveldb::CLeveldb()
{
}


CLeveldb::~CLeveldb()
{
}

bool CLeveldb::init()
{
	leveldb::Options options;
	options.create_if_missing = true;
	std::string dbpath = "leveldb";
	leveldb::Status status = leveldb::DB::Open(options, dbpath, &__m_db);
	return status.ok();
}

void CLeveldb::unInit()
{
	if (__m_db)
	{
		delete __m_db;
	}
}
#include <iostream>
bool CLeveldb::put(std::string key, std::string val)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	{
		static std::mutex mtx;
		std::lock_guard<std::mutex> guard(mtx);
		status = __m_db->Put(leveldb::WriteOptions(), key, val);
	}
	bool bret = status.ok();
	if (!bret)
	{
		LOG_ERROR("{} get from leveldb failed", key);
	}
	return bret;
}

bool CLeveldb::get(std::string key, std::string& val)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	status = __m_db->Get(leveldb::ReadOptions(), key, &val);
	return status.ok();
}

bool CLeveldb::del(std::string key)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	status = __m_db->Delete(leveldb::WriteOptions(), key);
	return status.ok();
}

bool CLeveldb::getOneItem(std::pair<std::string, std::string>& pair, std::string subString)
{
	pair.first = "";
	leveldb::Iterator* it = __m_db->NewIterator(leveldb::ReadOptions());
	bool bret = it->status().ok();
	if (bret)
	{
		for (it->SeekToLast(); it->Valid(); it->Prev())
		{
			if (it->key().ToString().find(subString) == -1)
			{
				continue;
			}
			else
			{
				pair.first = it->key().ToString();
				pair.second = it->value().ToString();
				break;
			}
		}
	}
	if (it)
	{
		delete it;
	}
	if (pair.first == "")
	{
		return false;
	}
	return bret;
}

bool CLeveldb::getAllItem(std::map<std::string, std::string>& mItems, std::string subString)
{
	leveldb::Iterator* it = __m_db->NewIterator(leveldb::ReadOptions());
	bool bret = it->status().ok();

	if (bret)
	{
		for (it->SeekToFirst(); it->Valid(); it->Next())
		{
			if (it->key().ToString().find(subString) == -1)
			{
				continue;
			}
			else
			{
				mItems[it->key().ToString()] = it->value().ToString();
			}
		}
	}
	if (it)
	{
		delete it;
	}
	return bret;
}
