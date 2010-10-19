
// For Visual C++, crank up the warning level to max.
#ifdef _MSC_VER

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#pragma warning( push , 4 )

#define _CRT_SECURE_NO_DEPRECATE
#pragma warning( disable : 4510 ) // 'class' : default constructor could not be generated
#pragma warning( disable : 4511 ) // 'class' : copy constructor could not be generated
#pragma warning( disable : 4512 ) // 'class' : assignment operator could not be generated
#pragma warning( disable : 4127 ) // conditional expression is constant

// Turn on some more warnings, that MS disable by default.

// "Compiler Warnings That Are Off by Default"
// http://msdn.microsoft.com/en-us/library/23k5d385(VS.80).aspx

// enumerator 'identifier' in switch of enum 'enumeration' is not handled
#pragma warning( 4 : 4062) //

// 'function' : member function does not override any base class virtual member function
#pragma warning( 4 : 4263)

// 'class' : class has virtual functions, but destructor is not virtual
#pragma warning( 4 : 4265)

// missing type specifier - int assumed. Note: C no longer supports default-int
#pragma warning( 4 : 4431)

// expression before comma evaluates to a function which is missing an argument list
#pragma warning( 4 : 4545)

// function call before comma missing argument list
#pragma warning( 4 : 4546)

// 'operator' : operator before comma has no effect; expected operator with side-effect
#pragma warning( 4 : 4547)

// 'operator' : operator before comma has no effect; did you intend 'operator'?
#pragma warning( 4 : 4549)

// 'function' : function not inlined
//#pragma warning( 4 : 4710)

#endif

// If compiling with Boost.Serialization or Boost.Asio, then this test is a lost cause.
#if defined(RCF_USE_BOOST_SERIALIZATION) || defined(RCF_USE_BOOST_ASIO)

#pragma message(__FILE__ "(" STRING(__LINE__) ")" )
#pragma message("Detected RCF_USE_BOOST_SERIALIZATION or RCF_USE_BOOST_ASIO: disabling warning test...")

// If compiling with vc6, then this test is a lost cause.
#elif defined(_MSC_VER) && _MSC_VER == 1200

#pragma message(__FILE__ "(" STRING(__LINE__) ")" )
#pragma message("Detected Visual C++ 6: disabling warning test...")

#else

#include <RCF/test/TestMinimal.hpp>
#include "../src/RCF/RCF.cpp"
#include "Test_Minimal.cpp"

#endif
