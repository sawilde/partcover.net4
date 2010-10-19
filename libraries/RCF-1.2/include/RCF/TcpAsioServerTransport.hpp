
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TCPASIOSERVERTRANSPORT_HPP
#define INCLUDE_RCF_TCPASIOSERVERTRANSPORT_HPP

#include <RCF/AsioServerTransport.hpp>

namespace RCF {

    class RCF_EXPORT TcpAsioServerTransport : 
        public AsioServerTransport,
        public I_IpServerTransport
    {
    public:
        TcpAsioServerTransport(int port = 0);

        TcpAsioServerTransport(
            const std::string &networkInterface, 
            int port = 0);

        ServerTransportPtr clone();

        // I_IpServerTransport implementation
        int                    getPort() const;

    private:

        AsioSessionStatePtr     implCreateSessionState();
        void                    implOpen();

        ClientTransportAutoPtr  implCreateClientTransport(
                                    const I_Endpoint &endpoint);

    private:
        int     mPort;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_TCPASIOSERVERTRANSPORT_HPP
