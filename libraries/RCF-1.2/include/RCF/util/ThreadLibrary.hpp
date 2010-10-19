
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_THREADLIBRARY_HPP
#define INCLUDE_UTIL_THREADLIBRARY_HPP

#if !defined(RCF_MULTI_THREADED) && !defined(RCF_SINGLE_THREADED)
#define RCF_MULTI_THREADED
#endif

#if defined(RCF_SINGLE_THREADED) && defined(RCF_USE_BOOST_THREADS)
#undef RCF_USE_BOOST_THREADS
#endif

#if defined(RCF_MULTI_THREADED) && defined(RCF_USE_BOOST_THREADS)
#include "Platform/Threads/BoostThreads.hpp"
#elif defined(RCF_MULTI_THREADED)
#include "Platform/Threads/RcfBoostThreads.hpp"
#else
#include "Platform/Threads/ThreadsProxy.hpp"
#endif

namespace util {

    typedef Platform::Threads::mutex Mutex;
    typedef Platform::Threads::mutex::scoped_lock Lock;

    typedef Platform::Threads::try_mutex TryMutex;
    typedef Platform::Threads::try_mutex::scoped_try_lock TryLock;

    typedef Platform::Threads::read_write_mutex ReadWriteMutex;
    typedef Platform::Threads::read_write_mutex::scoped_read_lock ReadLock;
    typedef Platform::Threads::read_write_mutex::scoped_write_lock WriteLock;

    static const Platform::Threads::read_write_scheduling_policy ReaderPriority = Platform::Threads::reader_priority;
    static const Platform::Threads::read_write_scheduling_policy WriterPriority = Platform::Threads::writer_priority;

    typedef Platform::Threads::thread Thread;

    typedef Platform::Threads::condition Condition;

    template<typename T>
    struct ThreadSpecificPtr 
    {
        typedef typename Platform::Threads::thread_specific_ptr<T>::Val Val;
    };

} // namespace util

#endif // ! INCLUDE_UTIL_THREADLIBRARY_HPP
