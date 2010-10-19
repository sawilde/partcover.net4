
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_UNIXLOCALENDPOINT_HPP
#define INCLUDE_RCF_UNIXLOCALENDPOINT_HPP

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/ServerTransport.hpp>

#include <SF/SerializeParent.hpp>

#if defined(BOOST_WINDOWS)
#error Unix domain sockets not supported on Windows.
#endif

#include <boost/version.hpp>
#if BOOST_VERSION < 103600
#error Need Boost 1.36.0 or later for Unix domain socket support.
#endif

namespace RCF {

    class RCF_EXPORT UnixLocalEndpoint : public I_Endpoint
    {
    public:

        UnixLocalEndpoint();

        UnixLocalEndpoint(const std::string & pipeName);

        ServerTransportAutoPtr createServerTransport() const;
        ClientTransportAutoPtr createClientTransport() const;
        EndpointPtr clone() const;

        std::string asString();

#ifdef RCF_USE_SF_SERIALIZATION

        void serialize(SF::Archive &ar);

#endif

        std::string getPipeName() const
        {
            return mPipeName;
        }

    private:

        std::string mPipeName;
    };

} // namespace RCF

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(RCF::UnixLocalEndpoint)

#endif // ! INCLUDE_RCF_UNIXLOCALENDPOINT_HPP
