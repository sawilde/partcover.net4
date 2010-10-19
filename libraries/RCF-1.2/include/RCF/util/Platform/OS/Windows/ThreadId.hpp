
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_WINDOWS_THREADID_HPP
#define INCLUDE_UTIL_PLATFORM_OS_WINDOWS_THREADID_HPP

#include "windows.h"

namespace Platform {

    namespace OS {

        typedef int ThreadId;
        inline ThreadId GetCurrentThreadId() { return ::GetCurrentThreadId(); }

    }
}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_WINDOWS_THREADID_HPP
