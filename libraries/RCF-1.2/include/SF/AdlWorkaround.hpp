
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_ADLWORKAROUND_HPP
#define INCLUDE_SF_ADLWORKAROUND_HPP

#include <SF/Archive.hpp>

// Argument dependent lookup doesn't work on vc6, so here's a workaround.

#if defined(_MSC_VER) && _MSC_VER < 1310

#define SF_ADL_WORKAROUND(Ns, T)                    \
namespace SF {                                      \
    inline void serialize(Archive &ar, Ns::T &t)    \
    {                                               \
        Ns::serialize(ar, t);                       \
    }                                               \
}

#else

#define SF_ADL_WORKAROUND(Ns, T)

#endif

#endif // ! INCLUDE_SF_ADLWORKAROUND_HPP
