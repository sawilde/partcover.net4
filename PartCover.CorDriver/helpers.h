#pragma once

template<bool static_assert>
struct StaticAssert;

template<> struct StaticAssert<true> {};

#define STATIC_ASSERT(expr) do { StaticAssert<expr>(); break; } while(true)
#define ASSERT_SIZEOF(type1, type2) do { StaticAssert<sizeof(type1)==sizeof(type2)>(); break; } while(true)

template<typename ValueType>
struct DynamicArray {
    typedef ValueType value_type;

    value_type* data;

    DynamicArray(size_t count) : data(new value_type[count]) {}
    ~DynamicArray() { delete[] data; }

    operator value_type* () { return data; }
    operator const value_type* () const { return data; }
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