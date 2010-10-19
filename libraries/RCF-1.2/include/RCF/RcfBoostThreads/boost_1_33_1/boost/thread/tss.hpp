
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

#ifndef RCF_BOOST_TSS_WEK070601_HPP
#define RCF_BOOST_TSS_WEK070601_HPP

#include "exceptions.hpp"

namespace boost {

namespace detail {

class RCF_BOOST_THREAD_DECL tss : private ::boost::noncopyable
{
public:
    tss(::boost::function1<void, void*>* pcleanup) {
        if (pcleanup == 0) throw thread_resource_error();
        try
        {
            init(pcleanup);
        }
        catch (...)
        {
            delete pcleanup;
            throw thread_resource_error();
        }
    }

    void* get() const;
    void set(void* value);
    void cleanup(void* p);

private:
    unsigned int m_slot; //This is a "pseudo-slot", not a native slot

    void init(::boost::function1<void, void*>* pcleanup);
};

#if defined(BOOST_HAS_MPTASKS)
void thread_cleanup();
#endif

template <typename T>
struct tss_adapter
{
    template <typename F>
    tss_adapter(const F& cleanup) : m_cleanup(cleanup) { }
    void operator()(void* p) { m_cleanup(static_cast<T*>(p)); }
    ::boost::function1<void, T*> m_cleanup;
};

} // namespace detail

template <typename T>
class thread_specific_ptr : private ::boost::noncopyable
{
public:
    thread_specific_ptr()
        : m_tss(new ::boost::function1<void, void*>(
                    /*boost::*/detail::tss_adapter<T>(
                        &thread_specific_ptr<T>::cleanup)))
    {
    }
    thread_specific_ptr(void (*clean)(T*))
        : m_tss(new ::boost::function1<void, void*>(
                    /*boost::*/detail::tss_adapter<T>(clean)))
    {
    }
    ~thread_specific_ptr() { reset(); }

    T* get() const { return static_cast<T*>(m_tss.get()); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }
    T* release() { T* temp = get(); if (temp) m_tss.set(0); return temp; }
    void reset(T* p=0)
    {
        T* cur = get();
        if (cur == p) return;
        m_tss.set(p);
        if (cur) m_tss.cleanup(cur);
    }

private:
    static void cleanup(T* p) { delete p; }
    detail::tss m_tss;
};

} // namespace boost

#endif //RCF_BOOST_TSS_WEK070601_HPP
