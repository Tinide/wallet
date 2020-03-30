#pragma once
#include <memory>
#include <vector>
#include <mutex>

//数据池
//get以后，独占
template<class T>
class CPool
{
public:
	CPool();
	virtual ~CPool();
public:
	std::shared_ptr<T> get(int nPos = -1);
	void add(std::shared_ptr<T> water);//增加
	void del(std::shared_ptr<T> water);
	int size();
	void clear();
private:
	std::vector<std::shared_ptr<T>> __m_vPool;
	std::mutex __m_vPoolMtx;
};

template<class T>
inline CPool<T>::CPool()
{
}

template<class T>
inline CPool<T>::~CPool()
{
	__m_vPool.clear();
}

template<class T>
std::shared_ptr<T> CPool<T>::get(int nPos)
{
	std::shared_ptr<T> spConn = std::shared_ptr<T>();
	__time64_t tick = GetTickCount64();
	while (1)
	{
		if (GetTickCount64() - tick > 5 * 1000)
		{
			break;
		}
		std::lock_guard<std::mutex> guard(__m_vPoolMtx);
		if (nPos >= 0)
		{
			if (nPos >= (int)__m_vPool.size())
			{
				break;
			}
			if (__m_vPool[nPos].unique())
			{
				spConn = __m_vPool[nPos];
			}
		}
		else
		{
			for (int i = 0; i < (int)__m_vPool.size(); i++)
			{
				if (__m_vPool[i].unique())
				{
					spConn = __m_vPool[i];
					break;
				}
			}
		}
		if (spConn)
		{
			break;
		}
	}
	return spConn;
}

template<class T>
inline void CPool<T>::add(std::shared_ptr<T> water)
{
	std::lock_guard<std::mutex> guard(__m_vPoolMtx);
	__m_vPool.push_back(water);
}

template<class T>
inline void CPool<T>::del(std::shared_ptr<T> water)
{
	std::lock_guard<std::mutex> guard(__m_vPoolMtx);
	for (auto it = __m_vPool.begin();it!=__m_vPool.end();it++)
	{
		if (*it == water)
		{
			__m_vPool.erase(it);
			break;
		}
	}
}

template<class T>
inline int CPool<T>::size()
{
	std::lock_guard<std::mutex> guard(__m_vPoolMtx);
	return (int)__m_vPool.size();
}

template<class T>
inline void CPool<T>::clear()
{
	std::lock_guard<std::mutex> guard(__m_vPoolMtx);
	__m_vPool.clear();
}
