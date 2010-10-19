
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

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

#ifndef RCF_BOOST_THREAD_WEK070601_HPP
#define RCF_BOOST_THREAD_WEK070601_HPP

#include "mutex.hpp"

namespace boost {

struct xtime;

class RCF_BOOST_THREAD_DECL thread : private ::boost::noncopyable
{
public:
    thread();
    explicit thread(const ::boost::function0<void>& threadfunc);
    ~thread();

    bool operator==(const thread& other) const;
    bool operator!=(const thread& other) const;

    void join();

    static void sleep(const xtime& xt);
    static void yield();

private:
#if defined(BOOST_HAS_WINTHREADS)
    void* m_thread;
    unsigned int m_id;
#elif defined(BOOST_HAS_PTHREADS)
private:
    pthread_t m_thread;
#elif defined(BOOST_HAS_MPTASKS)
    MPQueueID m_pJoinQueueID;
    MPTaskID m_pTaskID;
#endif
    bool m_joinable;
};

class RCF_BOOST_THREAD_DECL thread_group : private ::boost::noncopyable
{
public:
    thread_group();
    ~thread_group();

    thread* create_thread(const ::boost::function0<void>& threadfunc);
    void add_thread(thread* thrd);
    void remove_thread(thread* thrd);
    void join_all();
        int size();

private:
    std::list<thread*> m_threads;
    mutex m_mutex;
};

} // namespace boost

#endif // RCF_BOOST_THREAD_WEK070601_HPP
