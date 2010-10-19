
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_WINDOWS_SLEEP_HPP
#define INCLUDE_UTIL_PLATFORM_OS_WINDOWS_SLEEP_HPP

#include "windows.h"

namespace Platform {

    namespace OS {

        // +1 here causes a context switch if Sleep(0) is called
        inline void Sleep(unsigned int seconds) { ::Sleep(1000*seconds+1); }

    }

}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_WINDOWS_SLEEP_HPP
