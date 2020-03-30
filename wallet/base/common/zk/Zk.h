#pragma once

#include "zookeeper.h"
#include <string>
#include <vector>
#include <mutex>
typedef void(*zk_fn)(std::string val,int type,void* param);
class CZk;
struct watchParam
{
	std::string path;
	zk_fn fn;
	void* param;
	bool bDelete;
	CZk* pThis;
};
#define EXPIRED_WATCH -507
class CZk
{
public:
	CZk();
	virtual ~CZk();
public:
	static bool init();
	static void unInit();
public:
	int create(std::string path, std::string value,bool ephemeral = false);//ͬ��ģʽ
	int del(std::string path);//ͬ��ģʽ
	int set(std::string path,std::string value);//ͬ��ģʽ
	int get(std::string path, std::string& value);//ͬ��ģʽ
public:
	int aexists(std::string path, zk_fn fn, void* param);
private:
	static void __zk_watcher_g(zhandle_t* zh, int type, int state,
		const char* path, void* watcherCtx);
	static void __watcher(zhandle_t *zh, int type,
		int state, const char *path, void *watcherCtx);
	static void delWp(watchParam* pWp);
	static bool confirm(watchParam* pWp);
private:
	static zhandle_t* __m_zkhandle;
private:
	static std::mutex __m_svWpMtx;
	static std::vector<watchParam*> __m_svWp;
	static volatile bool bState;
};

