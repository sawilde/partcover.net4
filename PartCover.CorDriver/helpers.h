#pragma once

template<typename ValueType>
struct DynamicArray {
    typedef ValueType value_type;
	enum { ValueSize = sizeof(ValueType) };

private:
    value_type* _data;
	size_t _size;

public:
    DynamicArray(size_t count)
	{
		allocate(count);
	}

    ~DynamicArray() { deallocate(); }

	size_t size() const { return _size; }
	value_type* ptr() const { return _data; }

	void allocate(size_t new_size) 
	{
		_size = new_size;
		_data = new value_type[new_size];
		ZeroMemory(_data, sizeof(ValueType) * _size);
	}

	void deallocate() 
	{
		_size = 0;
		delete[] _data;
		_data = 0;
	}

	void resize(size_t new_size) 
	{
		deallocate();
		allocate(new_size);
	}

    operator value_type* () { return _data; }
    operator const value_type* () const { return _data; }
};

struct compare_no_case {
    inline bool operator()( const String &lhs, const String &rhs ) const {
        return _tcsicmp( lhs.c_str(), rhs.c_str() ) < 0;
    }
};

typedef std::map<String, String, compare_no_case> StringMap;

StringMap ParseEnvironment();

LPTSTR CreateEnvironment(const StringMap& env);
void FreeEnvironment(LPTSTR buffer);