
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TCPENDPOINT_HPP
#define INCLUDE_RCF_TCPENDPOINT_HPP

#include <string>
#include <memory>

#include <boost/shared_ptr.hpp>

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/TypeTraits.hpp>

namespace RCF {

    class I_ServerTransport;
    class I_ClientTransport;

    /// Represents a TCP communications endpoint.
    class RCF_EXPORT TcpEndpoint : public I_Endpoint
    {
    public:

        /// Default constructor.
        TcpEndpoint();

        /// Constructor.
        /// \param port TCP port number of the endpoint.
        TcpEndpoint(int port);

        /// Constructor.
        /// \param ip IP number of the endpoint.
        /// \param port TCP port number of the endpoint.
        TcpEndpoint(const std::string &ip, int port);

        /// Copy constructor.
        /// \param rhs TcpEndpoint object to construct this object from.
        TcpEndpoint(const TcpEndpoint &rhs);

        /// Creates a server transport.
        /// \return Auto pointer to server transport.
        std::auto_ptr<I_ServerTransport> createServerTransport() const;

        /// Creates a client transport.
        /// \return Auto pointer to client transport.
        std::auto_ptr<I_ClientTransport> createClientTransport() const;
       
        /// Clones this endpoint.
        /// \return Shared pointer to a new TcpEndpoint object.
        EndpointPtr clone() const;
       
        /// Returns IP number of this endpoint.
        /// \return IP number of this endpoint.
        std::string getIp() const;
       
        /// Returns port number of this endpoint.
        /// \return port number of this endpoint.
        int getPort() const ;

        std::string asString();

#ifdef RCF_USE_SF_SERIALIZATION

        /// Serializes the TcpEndpoint object.
        void serialize(SF::Archive &ar);

#endif

    private:
        std::string mIp;
        int mPort;
    };

    // serialization
    inline bool operator<(const TcpEndpoint &lhs, const TcpEndpoint &rhs)
    {
        return
            (lhs.getIp() < rhs.getIp()) ||
            (lhs.getIp() == rhs.getIp() && lhs.getPort() < rhs.getPort());
    }
   
} // namespace RCF

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(RCF::TcpEndpoint)

#endif // ! INCLUDE_RCF_TCPENDPOINT_HPP
