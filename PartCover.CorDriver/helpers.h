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
