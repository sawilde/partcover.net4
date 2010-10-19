
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Version.hpp>

#include <RCF/ClientStub.hpp>
#include <RCF/Exception.hpp>
#include <RCF/RcfSession.hpp>
#include <RCF/ThreadLocalData.hpp>

namespace RCF {

    const int gRcfRuntimeVersionInherent = 6;

    int gRcfRuntimeVersion = gRcfRuntimeVersionInherent;

    int getRuntimeVersionInherent()
    {
        return gRcfRuntimeVersionInherent;
    }

    int getRuntimeVersion()
    {
        return gRcfRuntimeVersion;
    }

    void setRuntimeVersion(int version)
    {
        RCF_VERIFY(
            1 <= version && version <= gRcfRuntimeVersionInherent,
            Exception(_RcfError_UnsupportedRuntimeVersion(version, gRcfRuntimeVersionInherent)))
            (version)(gRcfRuntimeVersionInherent);

        gRcfRuntimeVersion = version;
    }

} // namespace RCF
