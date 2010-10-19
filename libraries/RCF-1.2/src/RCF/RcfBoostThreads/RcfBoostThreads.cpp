
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

// RcfBoostThreads is a spin-off of version 1.33.1 of the Boost.Thread library,
// whose copyright notice follows.

// Copyright (C) 2001-2003
// William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.

#include <RCF/RcfBoostThreads/RcfBoostThreads.hpp>

#include <list>
#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <cstdio>

#include <boost/assert.hpp>
#include <boost/limits.hpp>
#include <boost/detail/workaround.hpp>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4311 )  // warning C4311: 'type cast' : pointer truncation from 'LPVOID' to 'LONG'
#pragma warning( disable : 4312 )  // warning C4312: 'type cast' : conversion from 'LONG' to 'LPVOID' of greater size
#pragma warning( disable : 4267 )  // warning C4267: '=' : conversion from 'size_t' to 'unsigned int', possible loss of data
#endif

#define RCF_BOOST_THREAD_DECL

#if defined(BOOST_HAS_FTIME)
#   include <windows.h>
#   include <boost/cstdint.hpp>
#elif defined(BOOST_HAS_GETTIMEOFDAY)
#   include <sys/time.h>
#elif defined(BOOST_HAS_MPTASKS)
#   include <DriverServices.h>
//#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/detail/force_cast.hpp>
#endif

#if defined(BOOST_HAS_WINTHREADS)
#   ifndef NOMINMAX
#      define NOMINMAX
#   endif
#   include <windows.h>
#elif defined(BOOST_HAS_PTHREADS)
#   include <errno.h>
#elif defined(BOOST_HAS_MPTASKS)
#   include <MacErrors.h>
#   include "mac/init.hpp"
#   include "mac/safe.hpp"
#endif

#if defined(BOOST_HAS_WINTHREADS)
#   include <new>
//#   include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/once.hpp>
#   include <windows.h>
#   include <time.h>
#elif defined(BOOST_HAS_PTHREADS)
#   include <errno.h>
#elif defined(BOOST_HAS_MPTASKS)
#    include <MacErrors.h>
#    include "mac/init.hpp"
#    include "mac/safe.hpp"
#endif

#if defined(BOOST_HAS_WINTHREADS)
#   if BOOST_WORKAROUND(__BORLANDC__,<= 0x551)
using std::size_t;
#   endif
#   include <windows.h>
#   if defined(BOOST_NO_STRINGSTREAM)
#       include <strstream>

class unfreezer
{
public:
    unfreezer(std::ostrstream& s) : m_stream(s) {}
    ~unfreezer() { m_stream.freeze(false); }
private:
    std::ostrstream& m_stream;
};

#   else
#       include <sstream>
#   endif
#elif defined(BOOST_HAS_MPTASKS)
#   include <Multiprocessing.h>
#endif

#if defined(BOOST_HAS_WINTHREADS)
#   include <windows.h>
#   if !defined(BOOST_NO_THREADEX)
#      include <process.h>
#   endif
#elif defined(BOOST_HAS_MPTASKS)
#   include <DriverServices.h>

#   include "init.hpp"
#   include "safe.hpp"
//#   include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/tss.hpp>
#endif

#if defined(BOOST_HAS_WINTHREADS)
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/detail/tss_hooks.hpp>
#endif

//#if defined(BOOST_HAS_WINTHREADS)
//#   include <windows.h>
//#   include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/detail/tss_hooks.hpp>
//#endif

namespace RCF {
    namespace RcfBoostThreads {

#include "boost_1_33_1/libs/thread/src/mutex.inl"
#include "boost_1_33_1/libs/thread/src/timeconv.inl"
#include "boost_1_33_1/libs/thread/src/condition.cpp"
#include "boost_1_33_1/libs/thread/src/exceptions.cpp"
#include "boost_1_33_1/libs/thread/src/mutex.cpp"
#include "boost_1_33_1/libs/thread/src/once.cpp"
#include "boost_1_33_1/libs/thread/src/thread.cpp"
#include "boost_1_33_1/libs/thread/src/tss.cpp"
#include "boost_1_33_1/libs/thread/src/tss_hooks.cpp"
#include "boost_1_33_1/libs/thread/src/xtime.cpp"

    } // namespace RcfBoostThreads
} // namespace RCF

extern "C" void tss_cleanup_implemented(void)
{}

#undef RCF_BOOST_THREAD_DECL

#ifdef _MSC_VER
#pragma warning( pop )
#endif
