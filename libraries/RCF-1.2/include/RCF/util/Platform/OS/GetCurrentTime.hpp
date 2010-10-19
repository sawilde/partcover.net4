
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_GETCURRENTTIME_HPP
#define INCLUDE_UTIL_PLATFORM_OS_GETCURRENTTIME_HPP

#include <boost/cstdint.hpp>
#include <RCF/Export.hpp>

namespace Platform {
    namespace OS {

        RCF_EXPORT boost::uint32_t getCurrentTimeMs();

    }
}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_GETCURRENTTIME_HPP
