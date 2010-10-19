
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_OS_SLEEP_HPP
#define INCLUDE_UTIL_PLATFORM_OS_SLEEP_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS) || defined(_WIN32)
#include "Windows/Sleep.hpp"
#elif defined(__CYGWIN__)
#include "Unix/Sleep.hpp"
#elif defined(__unix__)
#include "Unix/Sleep.hpp"
#elif defined(__APPLE__)
#include "Unix/Sleep.hpp"
#else
#include "UnknownOS/Sleep.hpp"
#endif

#endif // ! INCLUDE_UTIL_PLATFORM_OS_SLEEP_HPP
