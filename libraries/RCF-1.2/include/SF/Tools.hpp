
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_TOOLS_HPP
#define INCLUDE_SF_TOOLS_HPP

#include <RCF/Tools.hpp>

#include <boost/cstdint.hpp>

//****************************************************************************
// Helper macro to generate code for fundamental types

#define SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)  \
    arg(char)                               \
    arg(int)                                \
    arg(bool)                               \
    arg(float)                              \
    arg(double)                             \
    arg(short)                              \
    arg(long)                               \
    arg(unsigned short)                     \
    arg(unsigned char)                      \
    arg(unsigned int)                       \
    arg(unsigned long)                      \
    arg(long double)                        \
    //arg(wchar_t)

#if defined(_MSC_VER) || defined(__BORLANDC__)

#define SF_FOR_EACH_FUNDAMENTAL_TYPE(arg)   \
    SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)      \
    arg(__int64)                            \
    arg(unsigned __int64)

#else

#define SF_FOR_EACH_FUNDAMENTAL_TYPE(arg)   \
    SF_FOR_EACH_FUNDAMENTAL_TYPE_(arg)      \
    arg(long long)                          \
    arg(unsigned long long)

#endif

#endif // ! INCLUDE_SF_TOOLS_HPP

