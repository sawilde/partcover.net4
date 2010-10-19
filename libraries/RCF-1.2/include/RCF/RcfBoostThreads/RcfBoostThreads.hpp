
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

#ifndef INCLUDE_RCF_RCFBOOSTTHREADS_RCFBOOSTTHREADS_HPP
#define INCLUDE_RCF_RCFBOOSTTHREADS_RCFBOOSTTHREADS_HPP

#define __STDC_CONSTANT_MACROS

#include <list>
#include <memory>
#include <string>
#include <stdexcept>

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>

#ifndef RCF_SINGLE_THREADED
#include <boost/config/requires_threads.hpp>
#endif

#include <RCF/Export.hpp>

#define RCF_BOOST_THREAD_DECL RCF_EXPORT

#if defined(BOOST_HAS_PTHREADS)
#   include <pthread.h>
//#   include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/condition.hpp>
#elif defined(BOOST_HAS_MPTASKS)
#   include <Multiprocessing.h>
#endif

#if defined(BOOST_HAS_MPTASKS)
#   include "scoped_critical_region.hpp"
#endif

namespace RCF {
    namespace RcfBoostThreads {

#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/thread.hpp>
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/tss.hpp>
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/mutex.hpp>
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/condition.hpp>
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/xtime.hpp>
#include <RCF/RcfBoostThreads/boost_1_33_1/boost/thread/once.hpp>

    } // namespace RcfBoostThreads
} // namespace RCF

#undef RCF_BOOST_THREAD_DECL

#endif // ! INCLUDE_RCF_RCFBOOSTTHREADS_RCFBOOSTTHREADS_HPP
