#include "Zk.h"
#include "zookeeper_log.h"
#include <memory>
#include <iostream>
#include <future>
zhandle_t* CZk::__m_zkhandle = NULL;
std::vector<watchParam*> CZk::__m_svWp;
std::mutex CZk::__m_svWpMtx;
volatile bool CZk::bState = false;
CZk::CZk()
{
}

CZk::~CZk()
{
	{
		std::lock_guard<std::mutex> guard(__m_svWpMtx);
		for (auto it = __m_svWp.begin(); it != __m_svWp.end(); it++)
		{
			if((*it)->pThis == this)
			{
				(*it)->bDelete = true;
			}
		}
	}
}

bool CZk::init()
{
	const char* host = "47.91.247.252:2181";
	zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	bState = false;
	__m_zkhandle = zookeeper_init(host,__watcher, 30000, 0, "hello zookeeper", 0);
	if (__m_zkhandle == NULL) {
		return false;
	}
	for (int i = 0; i < 50; i++)
	{
		if (CZk::bState)
		{
			return true;
		}
		Sleep(100);
	}
	zookeeper_close(__m_zkhandle);
	return false;
}

void CZk::unInit()
{
	if (__m_zkhandle)
	{
		zookeeper_close(__m_zkhandle);
	}
	{
		std::lock_guard<std::mutex> guard(__m_svWpMtx);
		for (auto it : __m_svWp)
		{
			delete it;
		}
		__m_svWp.clear();
	}
}

int CZk::create(std::string path, std::string value,bool ephemeral)
{
	if (!CZk::bState)
	{
		return false;
	}
	char buf[256];
	int ret = zoo_create(__m_zkhandle, path.c_str(), value.c_str(), (int)value.size(), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, buf, 256);
	
	return ret;
}

int CZk::del(std::string path)
{
	if (!CZk::bState)
	{
		return false;
	}
	int ret = zoo_delete(__m_zkhandle, path.c_str(), -1);
	return ret;
}

int CZk::set(std::string path, std::string value)
{
	if (!CZk::bState)
	{
		return false;
	}
	int ret = zoo_set(__m_zkhandle, path.c_str(), value.c_str(), (int)value.size(), -1);
	return ret;
}

int CZk::get(std::string path,std::string& value)
{
	if (!CZk::bState)
	{
		return false;
	}
	char buf[256] = {};
	int n = 256;
	Stat stat;
	int ret = zoo_get(__m_zkhandle, path.c_str(), 0, buf,&n , &stat);
	value = buf;
	return ret;
}


void zktest_stat_completion(int rc, const struct Stat *stat, const void *data)
{
	if (data)
	{
		((std::promise<int>*)data)->set_value(rc);
	}
}

void zktest_data_completion(int rc, const char *value, int value_len,
	const struct Stat *stat, const void *data)
{
}

void CZk::__zk_watcher_g(zhandle_t* zh, int type, int state,
	const char* path, void* watcherCtx)
{
	//获取最新node
	CZk zk;
	std::string val;
	zk.get(path, val);

	//调用回调
	watchParam* pWp = (watchParam*)watcherCtx;
	if (CZk::confirm(pWp))
	{
		if (!pWp->bDelete)
		{
			pWp->fn(val, type, pWp->param);
		}
		CZk::delWp(pWp);
	}
}


void CZk::__watcher(zhandle_t *zh, int type,
	int state, const char *path, void *watcherCtx)
{
	std::cout << type << "  " << state << std::endl;
	if (state == ZOO_CONNECTED_STATE && type == ZOO_SESSION_EVENT)
	{
		CZk::bState = true;
	}
	else
	{
		CZk::bState = false;
		{
			std::lock_guard<std::mutex> guard(__m_svWpMtx);
			for (auto it : __m_svWp)
			{
				it->fn("", ZSYSTEMERROR, it->param);
				delete it;
			}
			__m_svWp.clear();
		}
	}

	if (state == ZOO_EXPIRED_SESSION_STATE)
	{
		CZk::bState = false;
		while (1)
		{
			CZk::unInit();
			if (CZk::init())
			{
				break;
			}
			Sleep(5000);
		}
	}
}

int CZk::aexists(std::string path, zk_fn fn, void* param)
{
	if (!CZk::bState)
	{
		return false;
	}
	watchParam* pWp = new watchParam();
	pWp->path = path;
	pWp->fn = fn;
	pWp->bDelete = false;
	pWp->param = param;
	pWp->pThis = this;

	{
		std::lock_guard<std::mutex> guard(__m_svWpMtx);
		__m_svWp.push_back(pWp);
	}

	std::promise<int> prom;
	std::future<int> fut = prom.get_future();
	zoo_awexists(__m_zkhandle, path.c_str(), __zk_watcher_g, pWp, zktest_stat_completion, &prom);

	int iret = fut.get();
	return iret;
}

void CZk::delWp(watchParam *pWp)
{
	if(pWp)
	{
		std::lock_guard<std::mutex> guard(__m_svWpMtx);
		for (auto it = __m_svWp.begin(); it != __m_svWp.end(); it++)
		{
			if (*it == pWp)
			{
				__m_svWp.erase(it);
				break;
			}
		}
		delete  pWp;
	}
}

bool CZk::confirm(watchParam * pWp)
{
	if (pWp)
	{
		std::lock_guard<std::mutex> guard(__m_svWpMtx);
		for (auto it : __m_svWp)
		{
			if (it == pWp)
			{
				return true;
			}
		}
	}
	return false;
}

