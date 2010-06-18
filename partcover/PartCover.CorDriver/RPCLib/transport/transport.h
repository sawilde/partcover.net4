#pragma once

#ifndef BIDI_BIDI_ARCHIVE
#define BIDI_BIDI_ARCHIVE

namespace bidi 
{
	template<class Stream>
	struct BidiArchive
	{
	public:
		BidiArchive(Stream& stream)
			: m_stream(stream)
		{
		}

		Stream& in() { return m_stream; }
		Stream& out() { return m_stream; }

		size_t read(void *pData, size_t nSize)
		{
			return in.read(pData, nSize);
		}

		void write(const void *pData, size_t nSize)
		{
			out.write(pData, nSize);
		}

		void flush() 
		{ 
			out().flush(); 
		}

	private:
		Stream& m_stream;
	};

	template <class Stream, class T> 
	BidiArchive<Stream> & operator << (BidiArchive<Stream> &ar, const T &t)
	{
		ar.out() << t;
		return ar;
	}

	template <class Stream, class T> 
	BidiArchive<Stream> & operator >> (BidiArchive<Stream> &ar, T &t)
	{
		ar.in() >> t;
		return ar;
	}
}

#endif

#include "pipeclient.h"
