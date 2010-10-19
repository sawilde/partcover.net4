
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

#ifndef RCF_BOOST_XLOCK_WEK070601_HPP
#define RCF_BOOST_XLOCK_WEK070601_HPP

#include "../exceptions.hpp"

namespace boost {

class condition;
struct xtime;

namespace detail { namespace thread {

template <typename Mutex>
class lock_ops : private ::boost::noncopyable
{
private:
    lock_ops() { }

public:
    typedef typename Mutex::cv_state lock_state;

    static void lock(Mutex& m)
    {
        m.do_lock();
    }
    static bool trylock(Mutex& m)
    {
        return m.do_trylock();
    }
    static bool timedlock(Mutex& m, const xtime& xt)
    {
        return m.do_timedlock(xt);
    }
    static void unlock(Mutex& m)
    {
        m.do_unlock();
    }
    static void lock(Mutex& m, lock_state& state)
    {
        m.do_lock(state);
    }
    static void unlock(Mutex& m, lock_state& state)
    {
        m.do_unlock(state);
    }
};

template <typename Mutex>
class scoped_lock : private ::boost::noncopyable
{
public:
    typedef Mutex mutex_type;

    explicit scoped_lock(Mutex& mx, bool initially_locked=true)
        : m_mutex(mx), m_locked(false)
    {
        if (initially_locked) lock();
    }
    ~scoped_lock()
    {
        if (m_locked) unlock();
    }

    void lock()
    {
        if (m_locked) throw lock_error();
        lock_ops<Mutex>::lock(m_mutex);
        m_locked = true;
    }
    void unlock()
    {
        if (!m_locked) throw lock_error();
        lock_ops<Mutex>::unlock(m_mutex);
        m_locked = false;
    }

    bool locked() const { return m_locked; }
    operator const void*() const { return m_locked ? this : 0; }

private:
    friend class boost::condition;

    Mutex& m_mutex;
    bool m_locked;
};

template <typename TryMutex>
class scoped_try_lock : private ::boost::noncopyable
{
public:
    typedef TryMutex mutex_type;

    explicit scoped_try_lock(TryMutex& mx)
        : m_mutex(mx), m_locked(false)
    {
        try_lock();
    }
    scoped_try_lock(TryMutex& mx, bool initially_locked)
        : m_mutex(mx), m_locked(false)
    {
        if (initially_locked) lock();
    }
    ~scoped_try_lock()
    {
        if (m_locked) unlock();
    }

    void lock()
    {
        if (m_locked) throw lock_error();
        lock_ops<TryMutex>::lock(m_mutex);
        m_locked = true;
    }
    bool try_lock()
    {
        if (m_locked) throw lock_error();
        return (m_locked = lock_ops<TryMutex>::trylock(m_mutex));
    }
    void unlock()
    {
        if (!m_locked) throw lock_error();
        lock_ops<TryMutex>::unlock(m_mutex);
        m_locked = false;
    }

    bool locked() const { return m_locked; }
    operator const void*() const { return m_locked ? this : 0; }

private:
    friend class boost::condition;

    TryMutex& m_mutex;
    bool m_locked;
};

template <typename TimedMutex>
class scoped_timed_lock : private ::boost::noncopyable
{
public:
    typedef TimedMutex mutex_type;

    scoped_timed_lock(TimedMutex& mx, const xtime& xt)
        : m_mutex(mx), m_locked(false)
    {
        timed_lock(xt);
    }
    scoped_timed_lock(TimedMutex& mx, bool initially_locked)
        : m_mutex(mx), m_locked(false)
    {
        if (initially_locked) lock();
    }
    ~scoped_timed_lock()
    {
        if (m_locked) unlock();
    }

    void lock()
    {
        if (m_locked) throw lock_error();
        lock_ops<TimedMutex>::lock(m_mutex);
        m_locked = true;
    }
    bool try_lock()
    {
        if (m_locked) throw lock_error();
        return (m_locked = lock_ops<TimedMutex>::trylock(m_mutex));
    }
    bool timed_lock(const xtime& xt)
    {
        if (m_locked) throw lock_error();
        return (m_locked = lock_ops<TimedMutex>::timedlock(m_mutex, xt));
    }
    void unlock()
    {
        if (!m_locked) throw lock_error();
        lock_ops<TimedMutex>::unlock(m_mutex);
        m_locked = false;
    }

    bool locked() const { return m_locked; }
    operator const void*() const { return m_locked ? this : 0; }

private:
    friend class boost::condition;

    TimedMutex& m_mutex;
    bool m_locked;
};

} // namespace thread
} // namespace detail
} // namespace boost

#endif // RCF_BOOST_XLOCK_WEK070601_HPP
