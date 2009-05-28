//	pipestransport.h: pipes transport
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__PIPESTRANSPORT_H__
#define __PIPESTRANSPORT_H__

struct PipeServer
{
	PipeServer():m_hPipe(INVALID_HANDLE_VALUE), m_sName(NULL)
	{
	}
	BOOL Create(LPCTSTR szName, DWORD dwTimeout = 2000, DWORD dwBufSize = 4096)
	{
		m_bStop = FALSE;
		m_sName = _tcsdup(szName);
		if (m_sName)
		{
			m_dwTimeout = dwTimeout;
			m_dwBufSize = dwBufSize;
			InternalCreatePipe();
			if (m_hPipe != INVALID_HANDLE_VALUE)
				return TRUE;
			free(m_sName);
			m_sName = NULL;
		}
		return FALSE;
	}
	BOOL Accept(HANDLE &theStream)
	{
		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			if (ConnectNamedPipe(m_hPipe, NULL) || (GetLastError() == ERROR_PIPE_CONNECTED))
			{
				if (!m_bStop)
				{
					theStream = m_hPipe;
					InternalCreatePipe();
					return TRUE;
				}
				VERIFY(::DisconnectNamedPipe(m_hPipe));
			}
			VERIFY(::CloseHandle(m_hPipe));
			m_hPipe = INVALID_HANDLE_VALUE;
		}
		return FALSE;
	}
	void Stop()
	{
		::InterlockedExchange((long *)&m_bStop, TRUE);
	//  possible hang on ::CallNamedPipe - if another pipe connected between this calls...
		DWORD dwTemp;
		::CallNamedPipe(m_sName, NULL, 0, NULL, 0, &dwTemp, NMPWAIT_NOWAIT);
	}
	void Close()
	{
		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			::DisconnectNamedPipe(m_hPipe);
			::CloseHandle(m_hPipe);
			m_hPipe = INVALID_HANDLE_VALUE;
		}
		free(m_sName);
		m_sName = NULL;
	}
private:
	void InternalCreatePipe()
	{
		m_hPipe = ::CreateNamedPipe( 
											m_sName,	// pipe name 
											PIPE_ACCESS_DUPLEX,       // read/write access 
											PIPE_TYPE_BYTE |       // message type pipe 
											PIPE_READMODE_BYTE |   // message-read mode 
											PIPE_WAIT,                // blocking mode 
											PIPE_UNLIMITED_INSTANCES, // max. instances  
											m_dwBufSize,                  // output buffer size 
											m_dwBufSize,                  // input buffer size 
											m_dwTimeout,             // client time-out 
											NULL				// no security attribute 
										);
	}
private:
	BOOL	m_bStop;
	LPTSTR	m_sName;
	HANDLE	m_hPipe;
	DWORD	m_dwTimeout;
	DWORD	m_dwBufSize;
};


#endif //__PIPESTRANSPORT_H__
