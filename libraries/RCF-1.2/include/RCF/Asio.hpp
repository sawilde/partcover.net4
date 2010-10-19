
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_ASIO_HPP
#define INCLUDE_RCF_ASIO_HPP

// Some issues with asio headers.
#if defined(__MACH__) && defined(__APPLE__)
#include <limits.h>
#ifndef IOV_MAX
#define IOV_MAX 1024
#endif
#endif

#include <boost/asio.hpp>

#endif // ! INCLUDE_RCF_ASIO_HPP
