
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_BSDSOCKETS_HPP
#define INCLUDE_UTIL_PLATFORM_OS_BSDSOCKETS_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS) || defined(_WIN32)
#include "Windows/BsdSockets.hpp"
#elif defined(__CYGWIN__)
#include "Unix/BsdSockets.hpp"
#elif defined(__unix__)
#include "Unix/BsdSockets.hpp"
#elif defined(__APPLE__)
#include "Unix/BsdSockets.hpp"
#else
#include "UnknownOS/BsdSockets.hpp"
#endif

namespace RCF {

    int getFdPort(int);

}

#endif // ! INCLUDE_UTIL_PLATFORM_OS_BSDSOCKETS_HPP
