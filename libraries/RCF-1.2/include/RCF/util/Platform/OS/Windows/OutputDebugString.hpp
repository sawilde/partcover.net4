
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP
#define INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP

#include "windows.h"

namespace Platform {

    namespace OS {

        inline void OutputDebugString(const char *sz)
        {
            ::OutputDebugStringA(sz);

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_STDERR
            fprintf(stderr, "%s", sz);
#endif

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_STDOUT
            fprintf(stdout, "%s", sz);
#endif

#ifdef UTIL_OUTPUTDEBUGSTRING_TO_FILE
            static FILE *file = fopen( "OutputDebugString.txt", "w" );
            fprintf(file, "%s", sz);
#endif

        }

    }

}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_WINDOWS_OUTPUTDEBUGSTRING_HPP
