//	SynchronousServer.h: multitread syncronous server
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__SYNCRONOUS_SERVER_H__
#define __SYNCRONOUS_SERVER_H__

#include "mtsync.h"
#include <process.h>

namespace mtlib
{

template <class Server>
struct ThreadedServer: public Server
{
	ThreadedServer():m_hThread(NULL){}
	BOOL Start()
	{
		unsigned nTemp;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, &ServerThread, static_cast<Server *>(this), 0, &nTemp);
		return m_hThread != NULL;
	}
	BOOL IsStarted() const
	{
		return m_hThread != NULL;
	}
	HANDLE GetStopHandle()
	{
		ASSERT(IsStarted());
		return m_hThread;
	}
	void Close()
	{
		Server::Close();
		if (IsStarted())
		{
			VERIFY(::CloseHandle(m_hThread));
			m_hThread = NULL;
		}
	}
private:
	static unsigned __stdcall ServerThread( void *pParam)
	{
		((Server *)pParam)->Start();
		return 0;
	}
private:
	HANDLE m_hThread;
};

struct EmptyClient{};

/*
template <class Server, class Client = EmptyClient>
struct ServerWorkerBase
{
	typedef Server server;
	typedef Client client;

};
*/

template <class Server, class Worker>
struct SynchronousServer: public Server
{
	typedef SynchronousServer<Server, Worker> this_type;
	SynchronousServer(): m_pClients(NULL){}
	BOOL IsActive() const
	{
		return m_pClients != NULL;
	}
	struct Client:public Worker
	{
		HANDLE			m_hThread;
		this_type		*m_pServer;
	};
	void LockClients()
	{
		m_csClients.Lock();
	}
	void UnlockClients()
	{
		m_csClients.Lock();
	}
	Client *GetFirstClient()
	{
		return m_pClients;
	}
	Client *GetNextClient(const Client *pClient)
	{
		return static_cast<SynchronousClient *>(pClient)->Next;
	}
	Client *GetPrevClient(const Client *pClient)
	{
		return static_cast<SynchronousClient *>(pClient)->Prev;
	}
private:

	struct SynchronousClient: public Client
	{
		SynchronousClient			*m_pNext;
		SynchronousClient			*m_pPrev;
	};
	void AttachClient(SynchronousClient *pClient)
	{
		if (m_pClients)
			m_pClients->m_pPrev = pClient;
		pClient->m_pNext = m_pClients;
		m_pClients = pClient;
		pClient->m_pPrev = NULL;
		pClient->m_pServer = this;

	}
	void DetachClient(SynchronousClient *pClient)
	{
		if (!pClient->m_pPrev)
			m_pClients = pClient->m_pNext;
		else
			pClient->m_pPrev->m_pNext = pClient->m_pNext;
		if (pClient->m_pNext)
			pClient->m_pNext->m_pPrev = pClient->m_pPrev;
	}
public:
	void Start()
	{
		unsigned nTemp;
		while (TRUE)
		{
			SynchronousClient *pClient = new SynchronousClient();
			if (Accept(*pClient))
			{
				AutoLock<CriticalSection>(this->m_csClients), AttachClient(pClient);
				pClient->m_hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientThread, pClient, 0, &nTemp);
				if (pClient->m_hThread)
					continue;
				AutoLock<CriticalSection>(this->m_csClients), DetachClient(pClient);
			}
			delete pClient;
			break;
		}
		while (TRUE)
		{
			HANDLE hThread;
			{
				AutoLock<CriticalSection> lock(m_csClients);
				if (!m_pClients)
					break;
				hThread = m_pClients->m_hThread;
				m_pClients->m_hThread = NULL;
			}
			if (hThread)
			{
				WaitForSingleObject(hThread, INFINITE);
				::CloseHandle(hThread);
			}
		}
	}
private:
	static unsigned __stdcall ClientThread( void *pClient)
	{
		SynchronousClient *pThis = (SynchronousClient *)pClient;
		unsigned nResult = pThis->Run();

		{
			AutoLock<CriticalSection> lock(pThis->m_pServer->m_csClients);

			if (pThis->m_hThread)
			{
	//			pThis->m_theClient.shutdown();
				VERIFY(::CloseHandle(pThis->m_hThread));
				pThis->m_hThread = NULL;
			}
			pThis->m_pServer->DetachClient(pThis);
			delete pThis;
		}
		return nResult;
	}
private:
	SynchronousClient *m_pClients;
	CriticalSection   m_csClients;
};

}

#endif //__SYNCRONOUS_SERVER_H__
