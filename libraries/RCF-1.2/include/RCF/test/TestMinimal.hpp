
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_TESTMINIMAL_HPP
#define INCLUDE_RCF_TEST_TESTMINIMAL_HPP

// Include valarray early so it doesn't get trampled by min/max macro definitions.
#if defined(_MSC_VER) && _MSC_VER == 1310 && defined(RCF_USE_BOOST_SERIALIZATION)
#include <valarray>
#endif

// For msvc-7.1, prevent including <locale> from generating warnings.
#if defined(_MSC_VER) && _MSC_VER == 1310
#pragma warning( disable : 4995 ) //  'func': name was marked as #pragma deprecated
#include <locale>
#pragma warning( default : 4995 )
#endif

// Weird errors with Borland C++Builder 6, in WSPiApi.h. This seems to help.
#if defined(__BORLANDC__) && __BORLANDC__ < 0x561
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <WSPiApi.h>
#endif

#include <RCF/util/Platform/OS/BsdSockets.hpp>
#include <RCF/util/VariableArgMacro.hpp>

#include <RCF/test/PrintTestHeader.hpp>
#include <RCF/test/ProgramTimeLimit.hpp>
#include <RCF/test/SpCollector.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>
#include <RCF/util/TraceCommandLineOption.hpp>
#include <RCF/test/DisableGpfDialog.hpp>

#include <iostream>

#ifdef BOOST_WINDOWS

void printTestMessage(const std::string & msg)
{
    std::cout << msg << std::flush;
    OutputDebugStringA(msg.c_str());
}

#else

void printTestMessage(const std::string & msg)
{
    std::cout << msg << std::flush;
}

#endif

int gTestFailures = 0;

void reportTestFailure(
    const char * file, 
    int line, 
    const char * condition,
    const char * info)
{
    std::ostringstream os;
    
    os << file << "(" << line << "): Test failed: " << condition;
    
    if (info)
    {
        os << " : " << info;
    }
    
    os << std::endl;

    printTestMessage( os.str() );
    
    ++gTestFailures;
}

namespace boost { static const int exit_success = 0; }

int test_main(int argc, char **argv);

int main(int argc, char **argv)
{
    Platform::OS::BsdSockets::disableBrokenPipeSignals();

    int ret = test_main(argc, argv);

    std::string exitMsg;
    if (gTestFailures)
    {
        std::ostringstream os;
        os << "*** Test Failures: " << gTestFailures << " ***" << std::endl;
        exitMsg = os.str();
    }
    else
    {
        exitMsg = "*** All Tests Passed ***\n";
    }

    printTestMessage(exitMsg);

    return ret + gTestFailures;
}

// Minidump creation code, for Visual C++ 2003 and later.
#if defined(_MSC_VER) && _MSC_VER >= 1310
#include <RCF/test/MiniDump.cpp>
#endif

// Simple RCF_CHECK macro, just calls reportTestFailure().
// RCF_CHECK

//#define RCF_CHECK(arg)                                
//    if (arg);                                           
//    else reportTestFailure(__FILE__, __LINE__, #arg, NULL);

// A better RCF_CHECK macro, which can evaluate and print named arguments, e.g.
// RCF_CHECK(arg1+arg2+arg3+arg4<10)(arg1)(arg2)(arg3)(arg4)

// RCF_CHECK

class RcfCheckFunctor : public util::VariableArgMacroFunctor
{
public:

    RcfCheckFunctor & setArgs(const char * file, int line, const char * cond)
    {
        mFile = file;
        mLine = line;
        mCond = cond;
        return *this;
    }

    void deinit()
    {
        reportTestFailure(mFile, mLine, mCond, args.str().c_str());
    }

private:
    const char *    mFile;
    int             mLine;
    const char *    mCond;
};

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
#endif

DECLARE_VARIABLE_ARG_MACRO( RCF_CHECK, RcfCheckFunctor );

#define RCF_CHECK(cond)                                             \
    if (cond);                                                      \
    else                                                            \
        VariableArgMacro<RcfCheckFunctor>()                         \
            .setArgs(__FILE__, __LINE__, #cond)                     \
            .cast( (VariableArgMacro<RcfCheckFunctor> *) NULL )     \
            .RCF_CHECK_A

#define RCF_CHECK_A(x)                         RCF_CHECK_OP(x, B)
#define RCF_CHECK_B(x)                         RCF_CHECK_OP(x, A)
#define RCF_CHECK_OP(x, next)                  RCF_CHECK_A.notify_((x), #x).RCF_CHECK_ ## next

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // ! INCLUDE_RCF_TEST_TESTMINIMAL_HPP
