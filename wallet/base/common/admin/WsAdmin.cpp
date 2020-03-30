#include "WsAdmin.h"

CWsAdmin gWsAdmin;

CWsAdmin::CWsAdmin()
{
}


CWsAdmin::~CWsAdmin()
{
}

void CWsAdmin::setSubTick(websocket_spHdl sphdl, bool subTick)
{
	{
		std::lock_guard<std::mutex> guard(_m_Mtx);
		auto itor = _m_TMap.find(sphdl);
		if (itor != _m_TMap.end())
		{
			itor->second.setSubTick(subTick);

		}
		else
		{
			LOG_ERROR("{},没有找到连接句柄",__FUNCTION__);
		}
	}
}

void CWsAdmin::pushReqNo(websocket_spHdl sphdl, int reqNo, Session session)
{
	{
		std::lock_guard<std::mutex> guard(_m_Mtx);
		_m_TMap[sphdl].setSession(reqNo, session);
	}
}

void CWsAdmin::pushOrderRef(websocket_spHdl sphdl, int reqNo, std::string orderRef, Session session)
{
	{
		std::lock_guard<std::mutex> guard(_m_Mtx);
		_m_TMap[sphdl].setSession(reqNo, session);
		_m_TMap[sphdl].setSession(orderRef, session);
	}
}

Session CWsAdmin::findReqId(int reqNo)
{
	Session session;
	{
		std::lock_guard<std::mutex> guard(_m_Mtx);
		for (auto it = _m_TMap.begin(); it != _m_TMap.end(); it++)
		{
			session = it->second.findSession(reqNo);
			if (session.uuid.size())
			{
				session.hdl = it->first;
				return session;
			}
		}
	}
	return Session();
}

Session CWsAdmin::findRef(std::string orderRef)
{
	Session session;
	{
		std::lock_guard<std::mutex> guard(_m_Mtx);
		for (auto it = _m_TMap.begin(); it != _m_TMap.end(); it++)
		{
			session = it->second.findSession(orderRef);
			if (session.uuid.size())
			{
				session.hdl = it->first;
				return session;
			}
		}
	}
	return session;
}

CWsClientInfo::CWsClientInfo() :__m_login(0),__m_bSubTick(false)
{

}

void CWsClientInfo::setSession(int reqNo, Session session)
{
	__m_mReqIdUuid[reqNo] = session;
}

void CWsClientInfo::setSession(std::string orderRef, Session session)
{
	__m_mRefUuid[orderRef] = session;
}

Session CWsClientInfo::findSession(int reqNo)
{
	Session session;
	auto it = __m_mReqIdUuid.find(reqNo);
	if (it != __m_mReqIdUuid.end())
	{
		session = it->second;
		__m_mReqIdUuid.erase(it);
	}
	return session;
}

Session CWsClientInfo::findSession(std::string orderRef)
{
	Session session;
	auto it = __m_mRefUuid.find(orderRef);
	if (it != __m_mRefUuid.end())
	{
		session = it->second;
		__m_mRefUuid.erase(it);
		for (auto it = __m_mReqIdUuid.begin(); it != __m_mReqIdUuid.end(); it++)
		{
			if (it->second == session)
			{
				__m_mReqIdUuid.erase(it);
				break;
			}
		}
	}
	return session;
}
