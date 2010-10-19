
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_UNIX_SLEEP_HPP
#define INCLUDE_UTIL_PLATFORM_OS_UNIX_SLEEP_HPP

#include <unistd.h>

namespace Platform {

    namespace OS {

        inline void Sleep(unsigned int seconds) { ::sleep(seconds); }

    }

}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_UNIX_SLEEP_HPP
