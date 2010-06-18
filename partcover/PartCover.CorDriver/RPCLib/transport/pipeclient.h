#pragma once

#ifndef BIDI_NAMED_PIPE
#define BIDI_NAMED_PIPE

namespace bidi {
	struct NamedPipe
	{
	public:
		NamedPipe() : m_pipe(INVALID_HANDLE_VALUE) 
		{}

		operator HANDLE & () { return m_pipe; }

		bool open(LPCTSTR pipename) 
		{
			ATLASSERT(m_pipe == INVALID_HANDLE_VALUE);

			m_pipe = CreateFile(
				pipename, 
				GENERIC_READ|GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			 return m_pipe != INVALID_HANDLE_VALUE;
		}

		void flush() 
		{
			FlushFileBuffers(m_pipe);
		}

		void close() 
		{
			if (m_pipe == INVALID_HANDLE_VALUE) return;

			CloseHandle(m_pipe);
			m_pipe = INVALID_HANDLE_VALUE;
		}

		size_t read(void *pData, size_t nSize);
		size_t write(const void *pData, size_t nSize);

	private:
		HANDLE m_pipe;
	};

	NamedPipe & operator << (NamedPipe &ar, String const& t);
	NamedPipe & operator >> (NamedPipe &ar, String &t);
	NamedPipe & operator << (NamedPipe &ar, char const& t);
	NamedPipe & operator >> (NamedPipe &ar, char &t);
	NamedPipe & operator << (NamedPipe &ar, int const& t);
	NamedPipe & operator >> (NamedPipe &ar, int &t);
	NamedPipe & operator << (NamedPipe &ar, unsigned int const& t);
	NamedPipe & operator >> (NamedPipe &ar, unsigned int &t);
	NamedPipe & operator << (NamedPipe &ar, unsigned long const& t);
	NamedPipe & operator >> (NamedPipe &ar, unsigned long &t);
	NamedPipe & operator << (NamedPipe &ar, long const& t);
	NamedPipe & operator >> (NamedPipe &ar, long &t);
	NamedPipe & operator << (NamedPipe &ar, bool const& t);
	NamedPipe & operator >> (NamedPipe &ar, bool &t);
}

#endif