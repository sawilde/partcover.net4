
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TOOLS_HPP
#define INCLUDE_RCF_TOOLS_HPP

// Various utilities

#include <stdlib.h>
#include <time.h>

#include <deque>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <vector>

#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/current_function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/util/UnusedVariable.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp> // GetErrorString()

// Assertion mechanism
#include <RCF/util/Assert.hpp>
#define RCF_ASSERT(x) UTIL_ASSERT(x, RCF::AssertionFailureException(), (*(::RCF::getTraceChannels()[9])))

// Verification mechanism
#include <RCF/util/Throw.hpp>
#define RCF_VERIFY(cond, e) UTIL_VERIFY(cond, e, (*::RCF::getTraceChannels()[9]))

// Trace mechanism
#include <RCF/util/Trace.hpp>
//#define RCF_TRACE(x) DUMMY_VARIABLE_ARG_MACRO()
#define RCF_TRACE(x)  UTIL_TRACE(x, (*(::RCF::getTraceChannels()[0])))        // for RcfServer layer
#define RCF1_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[1])))        // for thread manager layer
#define RCF2_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[2])))        // for server transport layer
#define RCF3_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[3])))        // for client transport layer
#define RCF4_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[4])))        // for client layer
#define RCF5_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[5])))        // for init/deinit
#define RCF6_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[6])))
#define RCF7_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[7])))
#define RCF8_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[8])))
#define RCF9_TRACE(x) UTIL_TRACE(x, (*(::RCF::getTraceChannels()[9])))        // for unexpected stuff (throws, asserts, suppressed dtor errors etc.)

namespace RCF {
    //extern util::TraceChannel *pTraceChannels[10];
    RCF_EXPORT util::TraceChannel **getTraceChannels();
}

// Throw mechanism
#include <RCF/util/Throw.hpp>
#define RCF_THROW(e)          UTIL_THROW(e, (*(::RCF::getTraceChannels()[9])))

// Scope guard mechanism
#include <boost/multi_index/detail/scope_guard.hpp>

namespace RCF 
{
    class Exception;
}

// assorted tracing conveniences
#ifndef __BORLANDC__
namespace std {
#endif

    // Trace std::vector
    template<typename T>
    std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
    {
        os << "(";
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, ", "));
        os << ")";
        return os;
    }

    // Trace std::deque
    template<typename T>
    std::ostream &operator<<(std::ostream &os, const std::deque<T> &d)
    {
        os << "(";
        std::copy(d.begin(), d.end(), std::ostream_iterator<T>(os, ", "));
        os << ")";
        return os;
    }

    // Trace type_info
    RCF_EXPORT std::ostream &operator<<(std::ostream &os, const std::type_info &ti);

    // Trace exception
    RCF_EXPORT std::ostream &operator<<(std::ostream &os, const std::exception &e);

    // Trace exception
    RCF_EXPORT std::ostream &operator<<(std::ostream &os, const RCF::Exception &e);

#ifndef __BORLANDC__
} // namespace std
#endif

namespace RCF {

    // Time in ms since ca 1970, modulo 65536 s (turns over every ~18.2 hrs).
    RCF_EXPORT unsigned int getCurrentTimeMs();

    // Generate a timeout value for the given ending time.
    // Returns zero if endTime <= current time <= endTime+10%of timer resolution, otherwise returns a nonzero duration in ms.
    // Timer resolution as above (18.2 hrs).
    RCF_EXPORT unsigned int generateTimeoutMs(unsigned int endTimeMs);

} // namespace RCF

// narrow/wide string utilities
#include <RCF/util/Tchar.hpp>
namespace RCF {

    typedef util::tstring tstring;

} // namespace RCF


namespace RCF {

    // null deleter, for use with for shared_ptr
    class NullDeleter
    {
    public:
        template<typename T>
        void operator()(T)
        {}
    };

    class SharedPtrIsNull
    {
    public:
        template<typename T>
        bool operator()(boost::shared_ptr<T> spt) const
        {
            return spt.get() == NULL;
        }
    };

} // namespace RCF

// VC workaround, in case platform headers have defined the min and max macros

#if !defined(_MSC_VER) || (defined(_MSC_VER) && _MSC_VER == 1200) || defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)
template<typename T>
T _cpp_min(T t1, T t2)
{
    return (t1 <= t2) ? t1 : t2;
}
template<typename T>
T _cpp_max(T t1, T t2)
{
    return (t1 <= t2) ? t2 : t1;
}
#endif

#if defined(min)
#define RCF_MIN _cpp_min
#define RCF_MAX _cpp_max
#else
#define RCF_MIN std::min
#define RCF_MAX std::max
#endif

#include <RCF/util/DefaultInit.hpp>

namespace RCF {

    RCF_EXPORT std::string toString(const std::exception &e);

} // namespace RCF

// destructor try/catch blocks
#define RCF_DTOR_BEGIN                              \
    try {

#define RCF_DTOR_END                                \
    }                                               \
    catch (const std::exception &e)                 \
    {                                               \
        if (!util::detail::uncaught_exception())    \
        {                                           \
            throw;                                  \
        }                                           \
        else                                        \
        {                                           \
            RCF9_TRACE(RCF::toString(e));           \
        }                                           \
    }

// vc6 issues

#if defined(_MSC_VER) && _MSC_VER == 1200

typedef unsigned long ULONG_PTR;

namespace std {

    std::ostream &operator<<(std::ostream &os, __int64);

    std::ostream &operator<<(std::ostream &os, unsigned __int64);

    std::istream &operator>>(std::istream &os, __int64 &);

    std::istream &operator>>(std::istream &os, unsigned __int64 &);

}

#endif

#if (__BORLANDC__ >= 0x560) && defined(_USE_OLD_RW_STL)
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <libs/thread/src/thread.cpp>
#include <libs/thread/src/tss.cpp>

static void DummyFuncToGenerateTemplateSpecializations(void)
{
    // http://lists.boost.org/boost-users/2005/06/12412.php

    // This forces generation of
    // boost::thread::thread(boost::function0<void>&)

    boost::function0<void> A = NULL;
    boost::thread B(A);

    // This forces generation of
    // boost::detail::tss::init(boost::function1<void, void *,
    // std::allocator<boost::function_base> > *)
    // but has the consequence of requiring the deliberately undefined function
    // tss_cleanup_implemented

    boost::function1<void, void*>* C = NULL;
    boost::detail::tss D(C);
}

#endif

#if defined(_MSC_VER) && _MSC_VER >= 1310
// need this for 64 bit builds with asio 0.3.7 (boost 1.33.1)
#include <boost/mpl/equal.hpp>
#include <boost/utility/enable_if.hpp>
namespace boost {
    template<typename T>
    inline std::size_t hash_value(
        T t,
        typename boost::enable_if< boost::mpl::equal<T, std::size_t> >::type * = 0)
    {
        return t;
    }
}
#endif

#if defined(_MSC_VER) && _MSC_VER < 1310
#define RCF_PFTO_HACK long
#else
#define RCF_PFTO_HACK
#endif

// Auto linking on VC++
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "crypt32.lib")
#endif

namespace RCF {

    struct Void {};

    // Bizarre compiler errors when trying to use erase-remove with Borland 5.6,
    // so here's a manual implementation instead.

#if defined(__BORLANDC__) && __BORLANDC__ <= 0x560

    template<typename Container, typename Element>
    void eraseRemove(Container & container, const Element & element)
    {
        std::size_t i = 0;
        while( i < container.size() )
        {
            if (container[i] == element)
            {
                for (std::size_t j = i; j < container.size() - 1; ++j)
                {
                    container[j] = container[j+1];
                }
                container.pop_back();
            }
            else
            {
                ++i;
            }
        }        
    }


#else

    template<typename Container, typename Element>
    void eraseRemove(Container & container, const Element & element)
    {
        container.erase(
            std::remove(
                container.begin(),
                container.end(),
                element),
            container.end());
    }

#endif

    RCF_EXPORT boost::uint64_t fileSize(const std::string & path);

} // namespace RCF

namespace boost {
    
    template<typename T>
    inline bool operator==(
        const boost::weak_ptr<T> & lhs, 
        const boost::weak_ptr<T> & rhs)
    {
        return ! (lhs < rhs) && ! (rhs < lhs);
    }

    template<typename T>
    inline bool operator!=(
        const boost::weak_ptr<T> & lhs, 
        const boost::weak_ptr<T> & rhs)
    {
        return ! (lhs == rhs);
    }

} // namespace boost

#endif // ! INCLUDE_RCF_TOOLS_HPP
