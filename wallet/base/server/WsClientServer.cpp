#include "WsClientServer.h"
#include "base/common/tool/Tool.h"


CWsClientServer::CWsClientServer()
{
	_m_maintainLoopTime = 10 * 1000;
}


CWsClientServer::~CWsClientServer()
{
	std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
	__m_wsClientMap.clear();
}

void CWsClientServer::_maintain()
{
	WsClientMap serverMap;
	{
		std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
		serverMap = __m_wsClientMap;
	}

	for (auto it : serverMap)
	{
			if (!it.second.first->ping() || !it.second.first->isOk())
			{
				{
					std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
					__m_wsClientMap[it.first].second.status = false;
					__m_wsClientMap[it.first].second.token = "";
				}
				LOG_ERROR("[{}]{} disconnect", it.first, it.second.first->getUrl());
				if (it.second.first->connect())
				{
					LOG_INFO("[{}]{} reconnect successfully", it.first, it.second.first->getUrl());
				}
				else
				{
					LOG_ERROR("[{}]{} connect faily", it.first, it.second.first->getUrl());
				}
			}
	}
	initStatus();
}

std::shared_ptr<CWsClient> CWsClientServer::_addWsClient(std::string wsClientName, std::string wsAddr)
{
	std::shared_ptr<CWsClient> pWsClient = std::make_shared<CWsClient>();
	pWsClient->bindNotify(std::bind((bool(CServer::*)(CMsgPtr))&CServer::receive, this, std::placeholders::_1));
	{
		std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
		__m_wsClientMap[wsClientName] = std::pair<std::shared_ptr<CWsClient>, ConnStatus>(pWsClient, ConnStatus());
	}

	bool bret = pWsClient->connect(wsAddr);
	if (bret)
	{
		LOG_INFO("[{}]{} connect successfully", wsClientName, wsAddr);
		initStatus();
	}
	else
	{
		LOG_ERROR("[{}]{} connect failly", wsClientName, wsAddr);
	}
	return pWsClient;
}

void CWsClientServer::initStatus()
{
	WsClientMap serverMap;
	{
		std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
		serverMap = __m_wsClientMap;
	}

	for (auto it: serverMap)
	{
			if (it.second.first->isOk())
			{
				if (it.second.second.status == false || (time(NULL)-it.second.second.updateTime> 0))
				{
					Json::Value rsp;
					bool bret = it.second.first->request(__m_reqJson, rsp);
					if (bret)
					{
						if (JsonInt(rsp, "code") == 0)
						{
							{
								std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
								__m_wsClientMap[it.first].second.status = true;
								__m_wsClientMap[it.first].second.token = JsonStr(JsonVal(rsp, "data"), "token");
								__m_wsClientMap[it.first].second.updateTime = time(NULL) + 6 * 24 * 3600;
							}
							LOG_INFO("[{}]{} init successfully", it.first, it.second.first->getUrl());
						}
						else
						{
							{
								std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
								__m_wsClientMap[it.first].second.status = false;
								__m_wsClientMap[it.first].second.token = "";
								__m_wsClientMap[it.first].second.updateTime = 0;
							}
							LOG_INFO("[{}]{} init failly", it.first, it.second.first->getUrl());
						}
					}
				}
			}
	}
}

WsClientStatus CWsClientServer::getWsClientStatus(std::string wsClientName)
{
	{
		std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
		auto it = __m_wsClientMap.find(wsClientName);
		if (it == __m_wsClientMap.end())
		{
			return WsClientStatus();
		}
		else
		{
			return it->second;
		}
	}
}

std::shared_ptr<CWsClient> CWsClientServer::getWsClient(std::string wsClientName)
{
	{
		std::lock_guard<std::mutex> guard(__m_wsClientMapMtx);
		auto it = __m_wsClientMap.find(wsClientName);
		if (it == __m_wsClientMap.end())
		{
			return std::make_shared<CWsClient>();
		}
		else
		{
			if (it->second.second.status)
			{
				return it->second.first;
			}
			else
			{
				return std::make_shared<CWsClient>();
			}
		}
	}
}
