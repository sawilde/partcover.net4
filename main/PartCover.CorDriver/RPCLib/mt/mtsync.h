//	mtsync.h: multitread syncronization primitives
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__MTSYNC_H__
#define __MTSYNC_H__

namespace mtlib
{

struct CriticalSection
{
	CriticalSection()
	{
		::InitializeCriticalSection(&m_cs);
	}
	~CriticalSection()
	{
		::DeleteCriticalSection(&m_cs);
	}
	void Lock()
	{
		::EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}
private:	
	CriticalSection (const CriticalSection &);
	CriticalSection & operator=(const CriticalSection  &);
private:
	CRITICAL_SECTION m_cs;
};

struct LWMutex
{
    LWMutex(volatile LONG &pFlag):m_pFlag(pFlag)
    {
    }
    LWMutex(const LWMutex &lwSource): m_pFlag(lwSource.m_pFlag)
	{
	}
    LWMutex & operator=(const LWMutex &lwSource)
	{
		m_pFlag = lwSource.m_pFlag;
		return *this;
	}
	void Lock()
	{
        while(::InterlockedExchange(&m_pFlag, TRUE))
            ::Sleep(1);
	}
	void Unlock()
	{
        ::InterlockedExchange(&m_pFlag, FALSE);
	}
private:
    volatile LONG &m_pFlag;
};


template<class T>
struct AutoLock
{
	AutoLock(T& obj):m_objLock(obj)
	{
		m_objLock.Lock();
	}
	~AutoLock()
	{
		m_objLock.Unlock();
	}
private:
	AutoLock(const AutoLock &);
	AutoLock & operator=(const AutoLock &);
private:
	T &m_objLock;
};

template <class T>
T auto_lock(T &obj)
{
	return CAutoLock<T>(obj);
}

}

#endif	//__MTSYNC_H__
