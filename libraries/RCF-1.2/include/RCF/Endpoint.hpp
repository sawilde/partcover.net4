
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_ENDPOINT_HPP
#define INCLUDE_RCF_ENDPOINT_HPP

#include <memory>
#include <string>

#include <boost/shared_ptr.hpp>

#include <RCF/Exception.hpp>
#include <RCF/SerializationProtocol.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/SfNew.hpp>
#endif

namespace RCF {

    class I_ServerTransport;
    class I_ClientTransport;
    class I_SessionManager;

    class I_Endpoint;
    typedef boost::shared_ptr<I_Endpoint> EndpointPtr;

    /// Represents an abstraction of a communications endpoint.
    /// In essence, I_Endpoint is a factory for creating compatible pairs of server and client transports.
    class I_Endpoint
    {
    public:
        /// Virtual destructor.
        virtual ~I_Endpoint()
        {}

        /// Creates a server transport corresponding to the endpoint.
        /// \return Auto pointer to a server transport.
        virtual std::auto_ptr<I_ServerTransport> createServerTransport() const = 0;

        /// Creates a client transport corresponding to the endpoint.
        /// \return Auto pointer to a client transport.
        virtual std::auto_ptr<I_ClientTransport> createClientTransport() const = 0;

        /// Clones the endpoint
        /// \return Shared pointer to a clone (deep copy) of this endpoint.
        virtual EndpointPtr clone() const = 0;

        void serialize(SF::Archive &)
        {}

        virtual std::string asString() = 0;
    };

} // namespace RCF

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(RCF::I_Endpoint)

#ifdef RCF_USE_SF_SERIALIZATION
namespace SF {
    SF_NO_CTOR(RCF::I_Endpoint)
}
#endif

#include <boost/version.hpp>

#if defined(RCF_USE_BOOST_SERIALIZATION) && BOOST_VERSION < 103600
#include <boost/serialization/is_abstract.hpp>
#include <boost/serialization/shared_ptr.hpp>
BOOST_IS_ABSTRACT(RCF::I_Endpoint)
BOOST_SERIALIZATION_SHARED_PTR(RCF::I_Endpoint)
#endif

#if defined(RCF_USE_BOOST_SERIALIZATION) && BOOST_VERSION >= 103600
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/shared_ptr.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(RCF::I_Endpoint)
BOOST_SERIALIZATION_SHARED_PTR(RCF::I_Endpoint)
#endif

// The following is a starting point for registering polymorphic serialization 
// for I_Endpoint-derived classes, with Boost.Serialization ...

/*
#ifdef RCF_USE_BOOST_SERIALIZATION
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/shared_ptr.hpp>
BOOST_CLASS_EXPORT_GUID(RCF::TcpEndpoint, "RCF::TcpEndpoint")
BOOST_SERIALIZATION_SHARED_PTR(RCF::TcpEndpoint)
#endif
*/

#endif // ! INCLUDE_RCF_ENDPOINT_HPP
