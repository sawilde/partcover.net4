#include "stdafx.h"
#include "pipeclient.h"

namespace bidi {
	size_t NamedPipe::read(void *pData, size_t nSize)
	{
		STATIC_ASSERT(sizeof byte == 1);

		if (nSize == 0) return 0;

		byte* buffer = static_cast<byte*>(pData);
		DWORD written = 0;
		DWORD size = nSize;
		while(size > 0) {
			if (0 == ReadFile(m_pipe, buffer, size, &written, NULL))
				throw std::exception();

			size -= written;
			buffer += written;
		}

		return buffer - static_cast<byte*>(pData);
	}

	size_t NamedPipe::write(const void *pData, size_t nSize)
	{
		STATIC_ASSERT(sizeof byte == 1);

		if (nSize == 0) return 0;

		byte const* buffer = static_cast<byte const*>(pData);
		DWORD written = 0;
		DWORD size = nSize;
		while(size > 0) {
			if (0 == WriteFile(m_pipe, buffer, size, &written, NULL))
				throw std::exception();

			size -= written;
			buffer += written;
		}

		return buffer - static_cast<byte const*>(pData);
	}

	NamedPipe & operator << (NamedPipe &ar, String const& t)
	{
		String::size_type string_size = t.size();
		ar << string_size;
		ar.write(t.c_str(), t.size() * sizeof(String::value_type));
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, String &t)
	{
		String::size_type string_size;
		ar >> string_size;

		String::value_type *buffer = new String::value_type[string_size];

		ar.read(buffer, string_size * sizeof(String::value_type));

		t.assign(buffer, string_size);

		delete[] buffer;

		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, char const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, char &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, int const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, int &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, unsigned int const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, unsigned int &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, unsigned long const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, unsigned long &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, long const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, long &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator << (NamedPipe &ar, bool const& t)
	{
		ar.write(&t, sizeof t);
		return ar;
	}

	NamedPipe & operator >> (NamedPipe &ar, bool &t)
	{
		ar.read(&t, sizeof t);
		return ar;
	}
}