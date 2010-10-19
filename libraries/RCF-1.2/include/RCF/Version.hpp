
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_VERSION_HPP
#define INCLUDE_RCF_VERSION_HPP

#include <RCF/Export.hpp>

// RCF 0.9c - 903
// RCF 0.9d - 9040
// RCF 1.0 - 10000
// RCF 1.1 - 11000
// RCF 1.2 - 12000
#define RCF_VERSION 12000

namespace RCF {

    // legacy       - version number 1

    // 2007-04-26   - version number 2
    // Released in 0.9c

    // 2008-03-29   - version number 3
    //      - Using I_SessionObjectFactory instead of I_ObjectFactoryService for session object creation and deletion.
    // Released in 0.9d

    // 2008-09-06   - version number 4
    //      - ByteBuffer compatible with std::vector etc.
    // Released in 1.0

    // 2008-12-06   - version number 5
    //      - Pingback field in MethodInvocationRequest
    // Released in 1.1

    // 2010-01-21   - version number 6
    //      - Archive version field in MethodInvocationRequest
    //      - Embedded version stamps in SF archives.
    // Released in 1.2

    RCF_EXPORT int getRuntimeVersionInherent();
    RCF_EXPORT int getRuntimeVersion();
    RCF_EXPORT void setRuntimeVersion(int version);

} // namespace RCF

#endif // ! INCLUDE_RCF_VERSION_HPP
