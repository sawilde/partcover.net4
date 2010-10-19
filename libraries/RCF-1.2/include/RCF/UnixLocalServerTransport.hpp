
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP
#define INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP

#if defined(BOOST_WINDOWS)
#error Unix domain sockets not supported on Windows.
#endif

#include <boost/version.hpp>
#if BOOST_VERSION < 103600
#error Need Boost 1.36.0 or later for Unix domain socket support.
#endif

#include <RCF/AsioServerTransport.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class RCF_EXPORT UnixLocalServerTransport : 
        public AsioServerTransport
    {
    public:

        UnixLocalServerTransport(const std::string & fileName);

        ServerTransportPtr clone();

        AsioSessionStatePtr implCreateSessionState();
        void implOpen();
        ClientTransportAutoPtr implCreateClientTransport(
            const I_Endpoint &endpoint);

        std::string getPipeName() const;

    private:

        const std::string                mFileName;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_UNIXLOCALSERVERTRANSPORT_HPP
