
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_MACHINE_BYTEORDER_HPP
#define INCLUDE_UTIL_PLATFORM_MACHINE_BYTEORDER_HPP

#if defined(__sparc__) || defined(__sparc)
#include "SPARC/ByteOrder.hpp"
#elif defined(__x86__) || defined(_M_IX86)
#include "x86/ByteOrder.hpp"
#elif defined(__i386__) || defined(__i386) || defined(i386)
#include "x86/ByteOrder.hpp"
#elif defined(__x86_64) || defined(__x86_64__)
#include "x86/ByteOrder.hpp"
#elif defined(__amd64) || defined(__amd64__)
#include "x86/ByteOrder.hpp"
#elif defined(_M_IA64) || defined(_M_AMD64) || defined(_M_X64)
#include "x86/ByteOrder.hpp"
#elif defined( __ppc__ )
#include "ppc/ByteOrder.hpp"
#elif defined(__arm__)
#include "x86/ByteOrder.hpp"
#elif defined(__bfin__)
#include "x86/ByteOrder.hpp"
#else
#include "UnknownMachine/ByteOrder.hpp"
#endif

#endif // ! INCLUDE_UTIL_PLATFORM_MACHINE_BYTEORDER_HPP
